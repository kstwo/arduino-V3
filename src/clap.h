#pragma once

#include <stdint.h>

enum class ClapEvent : uint8_t {
    None,
    Flash,   // первый хлопок принят, ждём второй
    Toggle   // второй хлопок в пределах окна — переключить свет
};

// Детектор двойного хлопка по спадающему фронту сигнала микрофона.
class ClapDetector {
public:
    void begin(bool mic_high) { mic_last_ = mic_high; }

    // suppressed — микрофон заглушён (mute либо защита после щелчка реле).
    ClapEvent update(uint32_t now, bool mic_high, bool suppressed);

    // Снимает ожидание второго хлопка по истечении окна.
    void expire(uint32_t now);

private:
    bool     mic_last_   = false;
    bool     waiting_    = false;
    uint32_t last_clap_  = 0;
    uint32_t first_clap_ = 0;
};
