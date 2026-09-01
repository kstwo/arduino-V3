#pragma once

#include <stdint.h>

// Антидребезг и подсчёт нажатий одной кнопки.
// Логика не обращается к железу: уровень вывода передаётся снаружи.
class Button {
public:
    // Начальное состояние вывода, чтобы первый poll() не выдал ложное нажатие.
    void begin(bool raw_pressed) { last_raw_ = raw_pressed; }

    void poll(uint32_t now, bool raw_pressed);

    // Забирает накопленное нажатие: true, если кнопка отпущена и клик готов.
    bool take_click();

    bool pressed() const { return pressed_; }

private:
    bool     last_raw_ = false;
    bool     pressed_  = false;
    uint32_t db_time_  = 0;
    uint8_t  clicks_   = 0;
    uint32_t rel_time_ = 0;
};
