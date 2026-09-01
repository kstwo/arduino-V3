#pragma once

#include <stdint.h>

#include "compat.h"

struct Note {
    uint16_t freq;  // 0 — пауза
    uint16_t dur;   // мс
};

struct Melody {
    const Note* notes;
    uint16_t    len;
};

// Партитуры лежат во флеш-памяти; доступ только через эти функции.
Melody mel_mario();
Melody mel_light_on();
Melody mel_light_off();
Melody mel_mute_on();
Melody mel_mute_off();
Melody mel_timer();
Melody mel_click();

// Неблокирующий проигрыватель: сам не трогает зуммер, а сообщает,
// какая частота должна звучать сейчас.
class MelodyPlayer {
public:
    // true, если воспроизведение началось и частота обновилась.
    bool play(uint32_t now, const Melody& m);

    // true, если пора сменить частоту.
    bool tick(uint32_t now);

    void stop();

    bool     playing() const { return playing_; }
    uint16_t freq() const { return freq_; }

private:
    const Note* notes_     = nullptr;
    uint16_t    len_       = 0;
    uint16_t    idx_       = 0;
    uint16_t    freq_      = 0;
    uint32_t    note_start_ = 0;
    bool        playing_   = false;
};
