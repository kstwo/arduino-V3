// Проверка эквивалентности: исходный скетч и модульная версия получают один
// и тот же поток входов и должны давать одинаковые уровни на реле,
// светодиоде и зуммере на каждом шаге.

#include <cstdio>
#include <cstdlib>

#include "Arduino.h"
#include "IRremote.hpp"

#include "config.h"
#include "controller.h"

extern void legacy_setup();
extern void legacy_loop();

namespace {

// --- слой железа новой версии, повторяет write_relay/write_outputs скетча ---

int new_relay = LOW;
int new_led   = 0;
int new_tone  = 0;

void apply_new(const Outputs& out) {
#if RELAY_LOW
    new_relay = out.relay_on ? LOW : HIGH;
#else
    new_relay = out.relay_on ? HIGH : LOW;
#endif
    new_led = out.led_pwm;
    if (out.tone_changed) new_tone = out.tone_freq;
}

Inputs snapshot() {
    Inputs in;
    in.now       = sim_now;
    in.mic_high  = sim_in[MIC_PIN] == HIGH;
    in.btn_light = !sim_in[BTN_LIGHT];
    in.btn_pwr   = !sim_in[BTN_PWR];
    in.btn_mute  = !sim_in[BTN_MUTE];
    in.btn_night = !sim_in[BTN_NIGHT];
    return in;
}

// --- детерминированный генератор стимулов ---

uint32_t rng = 0x9E3779B9u;  // переопределяется первым аргументом

uint32_t rnd() {
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return rng;
}

const uint16_t IR_CODES[] = {IR_MUTE, IR_L_ON, IR_L_OFF, IR_1M,  IR_5M,  IR_10M,
                             IR_15M,  IR_20M,  IR_30M,   IR_1H,  IR_2H,  IR_3H,
                             IR_4H,   IR_5H,   IR_8H,    0x0000, 0x007F};
const int IR_CODES_N = static_cast<int>(sizeof(IR_CODES) / sizeof(IR_CODES[0]));

const uint8_t BTN_PINS[] = {BTN_LIGHT, BTN_PWR, BTN_MUTE, BTN_NIGHT};

// --- статистика покрытия, чтобы тест не проходил вхолостую ---

struct Coverage {
    long relay_changes = 0;
    long tone_changes  = 0;
    long led_nonzero   = 0;
    long ir_events     = 0;
    long clap_edges    = 0;
    long wraps         = 0;
};

Coverage cov;

}  // namespace

int main(int argc, char** argv) {
    if (argc > 1) rng = static_cast<uint32_t>(std::atoi(argv[1]));

    sim_reset();

    // --- setup обеих версий ---
    legacy_setup();

    Controller ctl;
    ctl.begin(snapshot());
    apply_new(ctl.outputs());

    if (sim_digital[RELAY_PIN] != new_relay || sim_level[LED_PIN] != new_led ||
        sim_tone_freq != new_tone) {
        std::printf("РАСХОЖДЕНИЕ после setup: реле %d/%d, свет %d/%d, зуммер %d/%d\n",
                    sim_digital[RELAY_PIN], new_relay, sim_level[LED_PIN], new_led,
                    sim_tone_freq, new_tone);
        return 1;
    }

    const long STEPS    = 400000;
    int        failures = 0;

    int prev_relay = new_relay;
    int prev_tone  = new_tone;

    for (long step = 0; step < STEPS && failures < 5; step++) {
        // --- время ---
        const uint32_t roll = rnd() % 100;
        uint32_t       dt;
        if (roll < 88)      dt = 1 + rnd() % 20;         // обычный шаг цикла
        else if (roll < 97) dt = 20 + rnd() % 400;       // задержка
        else                dt = 1000 + rnd() % 900000;  // прыжок к срабатыванию таймера

        const uint32_t before = sim_now;
        sim_now += dt;
        if (sim_now < before) cov.wraps++;  // переполнение millis()

        // --- кнопки ---
        for (int i = 0; i < 4; i++) {
            if (rnd() % 100 < 3) sim_in[BTN_PINS[i]] = sim_in[BTN_PINS[i]] ? LOW : HIGH;
        }

        // --- микрофон ---
        if (rnd() % 100 < 6) {
            const uint8_t next = sim_in[MIC_PIN] ? LOW : HIGH;
            if (sim_in[MIC_PIN] == HIGH && next == LOW) cov.clap_edges++;
            sim_in[MIC_PIN] = next;
        }

        // --- ИК ---
        bool     have_ir   = rnd() % 100 < 4;
        bool     is_repeat = have_ir && (rnd() % 100 < 25);
        uint16_t cmd       = IR_CODES[rnd() % IR_CODES_N];
        if (have_ir) cov.ir_events++;

        // --- старая версия ---
        IrReceiver.pending                = have_ir;
        IrReceiver.decodedIRData.command  = cmd;
        IrReceiver.decodedIRData.flags    = is_repeat ? IRDATA_FLAGS_IS_REPEAT : 0;
        legacy_loop();

        // --- новая версия ---
        Inputs in = snapshot();
        if (have_ir) {
            in.ir_valid   = !is_repeat;
            in.ir_command = cmd;
        }
        ctl.update(in);
        apply_new(ctl.outputs());

        // --- сравнение ---
        if (sim_digital[RELAY_PIN] != new_relay || sim_level[LED_PIN] != new_led ||
            sim_tone_freq != new_tone) {
            std::printf(
                "РАСХОЖДЕНИЕ на шаге %ld (t=%u): реле %d/%d, свет %d/%d, зуммер %d/%d\n",
                step, static_cast<unsigned>(sim_now), sim_digital[RELAY_PIN], new_relay,
                sim_level[LED_PIN], new_led, sim_tone_freq, new_tone);
            failures++;
        }

        if (new_relay != prev_relay) { cov.relay_changes++; prev_relay = new_relay; }
        if (new_tone != prev_tone)   { cov.tone_changes++;  prev_tone  = new_tone; }
        if (new_led != 0)            cov.led_nonzero++;
    }

    std::printf("шагов: %ld\n", STEPS);
    std::printf("покрытие: реле %ld, зуммер %ld, светодиод %ld, ИК %ld, хлопки %ld, "
                "переполнений millis() %ld\n",
                cov.relay_changes, cov.tone_changes, cov.led_nonzero, cov.ir_events,
                cov.clap_edges, cov.wraps);

    if (failures != 0) {
        std::printf("РЕЗУЛЬТАТ: расхождения найдены\n");
        return 1;
    }
    if (cov.relay_changes == 0 || cov.tone_changes == 0 || cov.wraps == 0) {
        std::printf("РЕЗУЛЬТАТ: стимул недостаточен, проверка не показательна\n");
        return 1;
    }
    std::printf("РЕЗУЛЬТАТ: поведение совпадает\n");
    return 0;
}
