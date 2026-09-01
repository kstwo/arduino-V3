// Оригинальный скетч, собранный на хосте поверх заглушек ядра.
// Нужен только для проверки эквивалентности поведения после разделения кода
// на модули. Удаляется вместе с исходным файлом, когда проверка больше
// не требуется.

#include "Arduino.h"

#define setup legacy_setup
#define loop  legacy_loop

#include "legacy/light_v3_reference.ino"

#undef setup
#undef loop
