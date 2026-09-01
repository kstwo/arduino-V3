#pragma once

// Заглушка ядра Arduino для хостовой сборки. Достаточна для обеих версий
// прошивки: время, уровни выводов, зуммер и Serial.

#include <stdint.h>

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

#ifndef PROGMEM
  #define PROGMEM
#endif
#ifndef pgm_read_word
  #define pgm_read_word(addr) (*reinterpret_cast<const uint16_t*>(addr))
#endif
#define F(x) (x)

constexpr int SIM_PINS = 24;

extern uint32_t sim_now;
extern uint8_t  sim_in[SIM_PINS];       // уровни на входных выводах
extern uint8_t  sim_digital[SIM_PINS];  // последний digitalWrite
extern int      sim_level[SIM_PINS];    // последний digitalWrite/analogWrite
extern int      sim_tone_freq;          // текущая частота зуммера, 0 — тишина

void sim_reset();

uint32_t millis();
void     pinMode(uint8_t pin, uint8_t mode);
int      digitalRead(uint8_t pin);
void     digitalWrite(uint8_t pin, uint8_t value);
void     analogWrite(uint8_t pin, int value);
void     tone(uint8_t pin, unsigned int freq);
void     noTone(uint8_t pin);

struct SerialStub {
    void begin(unsigned long) {}
    void flush() {}
    void println() {}
    template <class T> void print(const T&) {}
    template <class T> void println(const T&) {}
};

extern SerialStub Serial;
