#include "melody.h"

namespace {

// частоты, Гц
constexpr uint16_t REST = 0;
constexpr uint16_t G4   = 392;
constexpr uint16_t C5   = 523;
constexpr uint16_t E5   = 659;
constexpr uint16_t F5   = 698;
constexpr uint16_t G5   = 784;
constexpr uint16_t A5   = 880;
constexpr uint16_t C6   = 1047;
constexpr uint16_t E6   = 1319;
constexpr uint16_t G6   = 1568;

const Note SND_MARIO[] PROGMEM = {
    {E5, 100}, {E5, 100}, {REST, 100}, {E5, 100},
    {REST, 100}, {C5, 100}, {E5, 200},
    {G5, 200}, {REST, 200}, {G4, 200}
};

const Note SND_L_ON[]  PROGMEM = { {C6, 60}, {E6, 60}, {REST, 40}, {G6, 120} };
const Note SND_L_OFF[] PROGMEM = { {A5, 60}, {C6, 60}, {REST, 40}, {E6, 120} };
const Note SND_M_ON[]  PROGMEM = { {G4, 200} };
const Note SND_M_OFF[] PROGMEM = { {C6, 200} };
const Note SND_TIMER[] PROGMEM = { {C5, 60}, {F5, 60}, {G5, 120} };
const Note SND_CLICK[] PROGMEM = { {C6, 40} };

template <uint16_t N>
constexpr Melody make(const Note (&notes)[N]) {
    return Melody{notes, N};
}

}  // namespace

Melody mel_mario()     { return make(SND_MARIO); }
Melody mel_light_on()  { return make(SND_L_ON); }
Melody mel_light_off() { return make(SND_L_OFF); }
Melody mel_mute_on()   { return make(SND_M_ON); }
Melody mel_mute_off()  { return make(SND_M_OFF); }
Melody mel_timer()     { return make(SND_TIMER); }
Melody mel_click()     { return make(SND_CLICK); }

bool MelodyPlayer::play(uint32_t now, const Melody& m) {
    if (m.len == 0 || m.notes == nullptr) return false;

    notes_      = m.notes;
    len_        = m.len;
    idx_        = 0;
    note_start_ = now;
    playing_    = true;
    freq_       = pgm_read_word(&m.notes[0].freq);
    return true;
}

bool MelodyPlayer::tick(uint32_t now) {
    if (!playing_) return false;

    const uint16_t dur = pgm_read_word(&notes_[idx_].dur);
    if (now - note_start_ < dur) return false;

    idx_++;
    if (idx_ >= len_) {
        playing_ = false;
        freq_    = 0;
        return true;
    }

    note_start_ = now;
    freq_       = pgm_read_word(&notes_[idx_].freq);
    return true;
}

void MelodyPlayer::stop() {
    playing_ = false;
    freq_    = 0;
}
