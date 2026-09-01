#pragma once

#include <stdint.h>

#include "button.h"
#include "clap.h"
#include "lamp_timer.h"
#include "led_fx.h"
#include "melody.h"

// Снимок входов за одну итерацию цикла. Все уровни уже приведены к логике
// «true — нажато / активно», время передаётся снаружи одним значением.
struct Inputs {
    uint32_t now         = 0;
    bool     mic_high    = false;
    bool     btn_light   = false;
    bool     btn_pwr     = false;
    bool     btn_mute    = false;
    bool     btn_night   = false;
    bool     ir_valid    = false;  // команда получена и не является повтором
    uint16_t ir_command  = 0;
};

// Требуемое состояние выходов. Скетч переносит его на выводы.
struct Outputs {
    bool     relay_on     = false;
    uint8_t  led_pwm      = 0;
    uint16_t tone_freq    = 0;      // 0 — тишина
    bool     tone_changed = false;  // частоту нужно применить
};

// Вся логика светильника. Не зависит от Arduino и собирается на хосте.
class Controller {
public:
    void begin(const Inputs& in);
    void update(const Inputs& in);

    const Outputs& outputs() const { return out_; }

    // Спящий режим. Сейчас ниоткуда не вызывается — как и в исходной версии;
    // сброс сторожевого таймера при этом остаётся на стороне скетча.
    void sleep();
    void wake(uint32_t now);

private:
    void handle_buttons(const Inputs& in);
    void handle_ir(uint32_t now, uint16_t cmd);
    void apply_light(uint32_t now);
    void cancel_timer();
    void start_melody(uint32_t now, const Melody& m);
    void stop_melody();
    void start_fx(Fx f, uint32_t now);

    Button       b_pwr_, b_light_, b_mute_, b_night_;
    ClapDetector clap_;
    LampTimer    timer_;
    LedFx        fx_;
    MelodyPlayer melody_;
    Outputs      out_;

    bool     pwr_   = false;
    bool     light_ = false;
    bool     muted_ = false;
    bool     night_ = false;

    uint32_t last_ir_     = 0;
    uint32_t relay_click_ = 0;
};
