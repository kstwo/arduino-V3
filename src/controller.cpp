#include "controller.h"

#include "config.h"

void Controller::begin(const Inputs& in) {
    pwr_ = true;

    clap_.begin(in.mic_high);
    b_pwr_.begin(in.btn_pwr);
    b_light_.begin(in.btn_light);
    b_mute_.begin(in.btn_mute);
    b_night_.begin(in.btn_night);

    start_melody(in.now, mel_mario());
}

// Порядок шагов повторяет исходный цикл: кнопки, зуммер, индикация,
// ИК, хлопки, таймер, снятие ожидания второго хлопка.
void Controller::update(const Inputs& in) {
    out_.tone_changed = false;

    handle_buttons(in);
    if (!pwr_) return;

    if (!night_ && melody_.tick(in.now)) {
        out_.tone_freq    = melody_.freq();
        out_.tone_changed = true;
    }

    out_.led_pwm = night_ ? 0 : fx_.update(in.now, timer_.active(), muted_);

    if (in.ir_valid && in.now - last_ir_ > IR_DB) {
        handle_ir(in.now, in.ir_command);
        last_ir_ = in.now;
    }

    const bool suppressed = muted_ || (in.now - relay_click_ < RELAY_PROT);
    switch (clap_.update(in.now, in.mic_high, suppressed)) {
        case ClapEvent::Flash:
            start_fx(Fx::Flash, in.now);
            break;
        case ClapEvent::Toggle:
            cancel_timer();
            light_ = !light_;
            apply_light(in.now);
            break;
        default:
            break;
    }

    if (timer_.expired(in.now)) {
        cancel_timer();
        light_ = !light_;
        apply_light(in.now);
    }

    clap_.expire(in.now);

    out_.relay_on = light_;
}

void Controller::handle_buttons(const Inputs& in) {
    b_pwr_.poll(in.now, in.btn_pwr);
    if (b_pwr_.take_click() && timer_.cancel()) {
        fx_.clear();
        start_melody(in.now, mel_click());
        start_fx(Fx::Flash, in.now);
    }

    if (!pwr_) return;

    b_light_.poll(in.now, in.btn_light);
    if (b_light_.take_click()) {
        light_ = !light_;
        apply_light(in.now);
    }

    b_mute_.poll(in.now, in.btn_mute);
    if (b_mute_.take_click()) {
        muted_ = !muted_;
        start_melody(in.now, muted_ ? mel_mute_on() : mel_mute_off());
    }

    b_night_.poll(in.now, in.btn_night);
    if (b_night_.take_click()) {
        night_ = !night_;
        if (night_) {
            stop_melody();
            out_.led_pwm = 0;
        } else {
            start_melody(in.now, mel_mute_off());
        }
    }
}

void Controller::handle_ir(uint32_t now, uint16_t cmd) {
    bool     is_timer = true;
    uint16_t minutes  = 0;

    switch (cmd) {
        case IR_MUTE:
            muted_ = !muted_;
            start_melody(now, muted_ ? mel_mute_on() : mel_mute_off());
            return;
        case IR_L_ON:
            cancel_timer();
            light_ = true;
            apply_light(now);
            return;
        case IR_L_OFF:
            cancel_timer();
            light_ = false;
            apply_light(now);
            return;

        case IR_1M:  minutes = 1;   break;
        case IR_5M:  minutes = 5;   break;
        case IR_10M: minutes = 10;  break;
        case IR_15M: minutes = 15;  break;
        case IR_20M: minutes = 20;  break;
        case IR_30M: minutes = 30;  break;
        case IR_1H:  minutes = 60;  break;
        case IR_2H:  minutes = 120; break;
        case IR_3H:  minutes = 180; break;
        case IR_4H:  minutes = 240; break;
        case IR_5H:  minutes = 300; break;
        case IR_8H:  minutes = 480; break;
        default:     is_timer = false; break;
    }

    if (is_timer) {
        timer_.start(now, minutes);
        fx_.clear();
        start_melody(now, mel_timer());
    }
}

void Controller::apply_light(uint32_t now) {
    out_.relay_on = light_;
    relay_click_  = now;

    if (light_) {
        start_melody(now, mel_light_on());
        start_fx(Fx::Double, now);
    } else {
        start_melody(now, mel_light_off());
        start_fx(Fx::Fade, now);
    }
}

void Controller::cancel_timer() {
    if (timer_.cancel()) fx_.clear();
}

void Controller::start_melody(uint32_t now, const Melody& m) {
    if (!pwr_ || night_) return;
    if (melody_.play(now, m)) {
        out_.tone_freq    = melody_.freq();
        out_.tone_changed = true;
    }
}

void Controller::stop_melody() {
    melody_.stop();
    out_.tone_freq    = 0;
    out_.tone_changed = true;
}

void Controller::start_fx(Fx f, uint32_t now) {
    if (night_) return;
    fx_.start(f, now);
}

void Controller::sleep() {
    cancel_timer();
    light_        = false;
    out_.relay_on = false;
    pwr_          = false;
    stop_melody();
    out_.led_pwm = 0;
}

void Controller::wake(uint32_t now) {
    pwr_ = true;
    start_melody(now, mel_light_on());
    start_fx(Fx::Double, now);
}
