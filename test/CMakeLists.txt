cmake_minimum_required(VERSION 3.16)
project(smart_light_tests CXX)

set(CMAKE_CXX_STANDARD 11)          # как в ядре Arduino AVR (-std=gnu++11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

set(FIRMWARE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_executable(equivalence
    equivalence.cpp
    legacy.cpp
    stub/Arduino.cpp
    stub/IRremote.cpp
    ${FIRMWARE_DIR}/button.cpp
    ${FIRMWARE_DIR}/clap.cpp
    ${FIRMWARE_DIR}/lamp_timer.cpp
    ${FIRMWARE_DIR}/led_fx.cpp
    ${FIRMWARE_DIR}/melody.cpp
    ${FIRMWARE_DIR}/controller.cpp
)

target_include_directories(equivalence PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/stub
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${FIRMWARE_DIR}
)

add_test(NAME equivalence COMMAND equivalence)
