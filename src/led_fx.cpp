#include "led_fx.h"

uint8_t LedFx::update(uint32_t now, bool timer_active, bool muted) {
    const uint32_t dt = now - start_;

    switch (fx_) {
        case Fx::Flash:
            if (dt < 150) return 255;
            fx_ = Fx::None;
            return 0;

        case Fx::Double:
            if (dt < 100) return 255;
            if (dt < 180) return 0;
            if (dt < 280) return 255;
            fx_ = Fx::None;
            return 0;

        case Fx::Fade:
            if (dt < 800) return static_cast<uint8_t>(255 - (dt * 255 / 800));
            fx_ = Fx::None;
            return 0;

        default:
            if (timer_active) return ((now / 1000) % 2) ? 0 : 255;
            if (muted) {
                const uint16_t ph = now % 2000;
                return static_cast<uint8_t>(
                    ph < 1000 ? (ph * 255 / 1000) : (255 - ((ph - 1000) * 255 / 1000)));
            }
            return 0;
    }
}
