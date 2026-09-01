#pragma once

#include <stdint.h>

// Обратный отсчёт до переключения света.
class LampTimer {
public:
    void start(uint32_t now, uint16_t minutes);

    // true, если таймер действительно был активен.
    bool cancel();

    bool active() const { return active_; }
    bool expired(uint32_t now) const;

private:
    bool     active_ = false;
    uint32_t dur_    = 0;
    uint32_t start_  = 0;
};
