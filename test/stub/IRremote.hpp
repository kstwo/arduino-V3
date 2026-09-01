#pragma once

#include <stdint.h>

#define IRDATA_FLAGS_IS_REPEAT 0x01
#define ENABLE_LED_FEEDBACK    true

struct IRData {
    uint16_t command = 0;
    uint16_t flags   = 0;
};

struct IrReceiverStub {
    IRData decodedIRData;
    bool   pending = false;

    void begin(uint8_t, bool) {}
    bool decode() { return pending; }
    void resume() { pending = false; }
};

extern IrReceiverStub IrReceiver;
