#pragma once

#include <stdint.h>

enum class Fx : uint8_t {
    None,
    Flash,   // короткая вспышка
    Double,  // две вспышки
    Fade     // плавное угасание
};

// Конечный автомат индикации: возвращает уровень ШИМ для светодиода.
class LedFx {
public:
    void start(Fx f, uint32_t now) {
        fx_    = f;
        start_ = now;
    }

    void clear() { fx_ = Fx::None; }

    // В покое индикация зависит от режимов: мигание при активном таймере,
    // плавная пульсация при mute.
    uint8_t update(uint32_t now, bool timer_active, bool muted);

private:
    Fx       fx_    = Fx::None;
    uint32_t start_ = 0;
};
