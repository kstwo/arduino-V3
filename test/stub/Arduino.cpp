#include "Arduino.h"

uint32_t sim_now = 0;
uint8_t  sim_in[SIM_PINS];
uint8_t  sim_digital[SIM_PINS];
int      sim_level[SIM_PINS];
int      sim_tone_freq = 0;

SerialStub Serial;

void sim_reset() {
    sim_now       = 0;
    sim_tone_freq = 0;
    for (int i = 0; i < SIM_PINS; i++) {
        sim_in[i]      = HIGH;  // подтяжки: всё отпущено
        sim_digital[i] = LOW;
        sim_level[i]   = 0;
    }
}

uint32_t millis() { return sim_now; }

void pinMode(uint8_t, uint8_t) {}

int digitalRead(uint8_t pin) { return sim_in[pin]; }

void digitalWrite(uint8_t pin, uint8_t value) {
    sim_digital[pin] = value;
    sim_level[pin]   = value ? 255 : 0;
}

void analogWrite(uint8_t pin, int value) { sim_level[pin] = value; }

void tone(uint8_t, unsigned int freq) { sim_tone_freq = static_cast<int>(freq); }

void noTone(uint8_t) { sim_tone_freq = 0; }
