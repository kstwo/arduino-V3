#pragma once

// Модули логики собираются как для AVR, так и для хоста (тесты, статический
// анализ). На хосте нет avr/pgmspace.h, поэтому чтение из PROGMEM вырождается
// в обычное разыменование.

#ifdef __AVR__
  #include <avr/pgmspace.h>
#else
  #include <stdint.h>
  #ifndef PROGMEM
    #define PROGMEM
  #endif
  #ifndef pgm_read_word
    #define pgm_read_word(addr) (*reinterpret_cast<const uint16_t*>(addr))
  #endif
#endif
