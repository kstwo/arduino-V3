// Светильник с реле: ИК-пульт, двойной хлопок, таймеры и четыре кнопки.
//
// Скетч содержит только слой железа: читает выводы, отдаёт снимок входов
// контроллеру и переносит его выходы обратно на выводы. Вся логика лежит
// в отдельных модулях и собирается без Arduino.

#include <avr/wdt.h>

// IRremote на таймере 1, иначе конфликт с tone()
#define IR_USE_AVR_TIMER1
#include <IRremote.hpp>

#include "src/config.h"
#include "src/controller.h"

#if DEBUG
  #define DBG(x)   Serial.print(x)
  #define DBGLN(x) Serial.println(x)
#else
  #define DBG(x)
  #define DBGLN(x)
#endif

static Controller ctl;

static void write_relay(bool on) {
#if RELAY_LOW
    digitalWrite(RELAY_PIN, on ? LOW : HIGH);
#else
    digitalWrite(RELAY_PIN, on ? HIGH : LOW);
#endif
}

static Inputs read_inputs() {
    Inputs in;
    in.now       = millis();
    in.mic_high  = digitalRead(MIC_PIN) == HIGH;
    in.btn_light = !digitalRead(BTN_LIGHT);
    in.btn_pwr   = !digitalRead(BTN_PWR);
    in.btn_mute  = !digitalRead(BTN_MUTE);
    in.btn_night = !digitalRead(BTN_NIGHT);
    return in;
}

static void write_outputs(const Outputs& out) {
    write_relay(out.relay_on);
    analogWrite(LED_PIN, out.led_pwm);

    if (out.tone_changed) {
        if (out.tone_freq > 0) tone(BUZ_PIN, out.tone_freq);
        else noTone(BUZ_PIN);
    }
}

void setup() {
    pinMode(MIC_PIN, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZ_PIN, OUTPUT);

    pinMode(BTN_LIGHT, INPUT_PULLUP);
    pinMode(BTN_PWR, INPUT_PULLUP);
    pinMode(BTN_MUTE, INPUT_PULLUP);
    pinMode(BTN_NIGHT, INPUT_PULLUP);

    write_relay(false);
    digitalWrite(LED_PIN, LOW);
    noTone(BUZ_PIN);

    Serial.begin(BAUD);

    IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

    ctl.begin(read_inputs());
    write_outputs(ctl.outputs());

#if USE_WDT
    wdt_enable(WDTO_2S);
#endif
    DBGLN(F("init ok"));
}

void loop() {
#if USE_WDT
    wdt_reset();
#endif

    Inputs in = read_inputs();

    const bool decoded = IrReceiver.decode();
    if (decoded) {
        in.ir_valid   = !(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT);
        in.ir_command = IrReceiver.decodedIRData.command;
    }

    ctl.update(in);
    write_outputs(ctl.outputs());

    if (decoded) IrReceiver.resume();
}
