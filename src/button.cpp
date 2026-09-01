#include "button.h"

#include "config.h"

void Button::poll(uint32_t now, bool raw_pressed) {
    if (raw_pressed != last_raw_) db_time_ = now;
    last_raw_ = raw_pressed;

    if (now - db_time_ > BTN_DB) {
        if (raw_pressed != pressed_) {
            pressed_ = raw_pressed;
            if (!pressed_) {
                if (clicks_ == 0 || now - rel_time_ > DBL_CLICK) clicks_ = 1;
                else clicks_++;
                rel_time_ = now;
            }
        }
    }
}

bool Button::take_click() {
    if (clicks_ > 0 && !pressed_) {
        clicks_ = 0;
        return true;
    }
    return false;
}
