#include "clap.h"

#include "config.h"

ClapEvent ClapDetector::update(uint32_t now, bool mic_high, bool suppressed) {
    if (suppressed) {
        mic_last_ = mic_high;
        return ClapEvent::None;
    }

    ClapEvent ev = ClapEvent::None;

    if (mic_last_ && !mic_high && now - last_clap_ >= CLAP_DB) {
        last_clap_ = now;

        if (!waiting_) {
            first_clap_ = now;
            waiting_    = true;
            ev          = ClapEvent::Flash;
        } else if (now - first_clap_ <= CLAP_WIN) {
            waiting_ = false;
            ev       = ClapEvent::Toggle;
        } else {
            first_clap_ = now;
            ev          = ClapEvent::Flash;
        }
    }

    mic_last_ = mic_high;
    return ev;
}

void ClapDetector::expire(uint32_t now) {
    if (waiting_ && now - first_clap_ > CLAP_WIN) waiting_ = false;
}
