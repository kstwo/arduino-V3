#pragma once

#include <stdint.h>

// --- сборка ---
#define DEBUG 1
#define USE_WDT 1
#define RELAY_LOW 1

constexpr uint32_t BAUD = 115200;

// --- пины ---
constexpr uint8_t IR_PIN    = 2;
constexpr uint8_t MIC_PIN   = 3;
constexpr uint8_t RELAY_PIN = 4;
constexpr uint8_t LED_PIN   = 5;
constexpr uint8_t BUZ_PIN   = 6;
constexpr uint8_t BTN_LIGHT = 7;
constexpr uint8_t BTN_PWR   = 8;
constexpr uint8_t BTN_MUTE  = 9;
constexpr uint8_t BTN_NIGHT = 10;

// --- тайминги, мс ---
constexpr uint32_t IR_DB      = 250;  // антидребезг ИК-команд
constexpr uint32_t CLAP_DB    = 250;  // минимальный интервал между хлопками
constexpr uint32_t CLAP_WIN   = 700;  // окно ожидания второго хлопка
constexpr uint32_t RELAY_PROT = 500;  // глухота микрофона после щелчка реле
constexpr uint32_t BTN_DB     = 50;   // антидребезг кнопок
constexpr uint32_t DBL_CLICK  = 350;  // окно двойного нажатия

// --- коды пульта ---
constexpr uint16_t IR_MUTE  = 0x09;
constexpr uint16_t IR_L_ON  = 0x15;
constexpr uint16_t IR_L_OFF = 0x07;

constexpr uint16_t IR_1M  = 0x16;
constexpr uint16_t IR_5M  = 0x19;
constexpr uint16_t IR_10M = 0x0D;
constexpr uint16_t IR_15M = 0x0C;
constexpr uint16_t IR_20M = 0x18;
constexpr uint16_t IR_30M = 0x5E;
constexpr uint16_t IR_1H  = 0x08;
constexpr uint16_t IR_2H  = 0x5A;
constexpr uint16_t IR_3H  = 0x42;
constexpr uint16_t IR_4H  = 0x52;
constexpr uint16_t IR_5H  = 0x1C;
constexpr uint16_t IR_8H  = 0x4A;
