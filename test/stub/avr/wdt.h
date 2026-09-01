#pragma once

#define WDTO_15MS 0
#define WDTO_2S   7

inline void wdt_enable(int) {}
inline void wdt_reset() {}
inline void wdt_disable() {}
