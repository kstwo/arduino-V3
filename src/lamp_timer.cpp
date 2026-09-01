#include "lamp_timer.h"

void LampTimer::start(uint32_t now, uint16_t minutes) {
    dur_    = static_cast<uint32_t>(minutes) * 60000UL;
    start_  = now;
    active_ = true;
}

bool LampTimer::cancel() {
    if (!active_) return false;
    active_ = false;
    return true;
}

bool LampTimer::expired(uint32_t now) const {
    return active_ && now - start_ >= dur_;
}
