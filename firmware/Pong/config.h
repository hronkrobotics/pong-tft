#pragma once

#include <stdint.h>

// Driver on the 3.5 inch module. 9488 is the common MSP3520 / MSP3521 board.
// Change this to 9486 only when the silkscreen says ILI9486.
#ifndef TFT_DRIVER
#define TFT_DRIVER 9488
#endif

// Landscape orientations. Use 3 when the picture is upside down.
constexpr uint8_t ROTATION = 1;

// These panels are BGR. Flip this when red and blue are swapped.
constexpr bool TFT_USE_BGR = true;

// Flip this when the picture is mirrored left to right.
constexpr bool TFT_MIRROR = false;

// ILI9486 SPI panels need inversion on. ILI9488 panels need it off.
// If the picture looks like a photo negative, flip the value for your driver.
#if TFT_DRIVER == 9486
constexpr bool TFT_INVERT = true;
#else
constexpr bool TFT_INVERT = false;
#endif

// 4 MHz is the reliable speed on a breadboard. 8000000 is fine once the
// picture is solid and the SCK wire is short.
constexpr uint32_t SPI_HZ = 4000000;

// D10 is the AVR SPI slave-select pin. It has to be an output or the Nano
// drops out of SPI master mode. Leave chip-select on D10.
constexpr uint8_t PIN_CS = 10;
constexpr uint8_t PIN_DC = 9;
constexpr uint8_t PIN_RST = 8;
constexpr uint8_t PIN_SCK = 13;
constexpr uint8_t PIN_MOSI = 11;
constexpr uint8_t PIN_MISO = 12;

// Left mini breadboard is the cyan paddle. Right mini breadboard is amber.
constexpr uint8_t PIN_P1_UP = 2;
constexpr uint8_t PIN_P1_DOWN = 3;
constexpr uint8_t PIN_P2_UP = 4;
constexpr uint8_t PIN_P2_DOWN = 5;

// Optional piezo between D6 and GND. Set this to 1 when the buzzer is fitted.
#define BUZZER_ENABLED 0
constexpr uint8_t PIN_BUZZER = 6;

// Prints one line each time a button goes down, so a wiring check can use
// the serial monitor before the picture is trusted.
#define DEBUG_BUTTONS 1

constexpr uint8_t DEBOUNCE_MS = 20;
constexpr uint32_t SERIAL_BAUD = 115200;

constexpr int16_t SCREEN_W = 480;
constexpr int16_t SCREEN_H = 320;

// Playfield inside the 480x320 landscape panel. Scores live above FIELD_TOP.
constexpr int16_t FIELD_TOP = 38;
constexpr int16_t FIELD_BOTTOM = 318;

constexpr int16_t PADDLE_W = 14;
constexpr int16_t PADDLE_H = 64;
constexpr int16_t PADDLE_INSET = 16;
constexpr int16_t BALL_SIZE = 16;

constexpr int16_t PADDLE_SPEED = 210;  // pixels per second
constexpr int16_t SERVE_SPEED = 170;
constexpr int16_t MIN_VX = 160;
constexpr int16_t MAX_VX = 320;
constexpr int16_t MAX_VY = 280;
constexpr int16_t SPEED_BUMP = 16;  // added to horizontal speed on each paddle hit
constexpr int16_t ENGLISH = 5;      // vertical kick from an off-center hit

constexpr uint8_t POINTS_TO_WIN = 11;
constexpr uint16_t POINT_PAUSE_MS = 700;
// Buttons are ignored this long after the winning point, so a player still
// mashing from the last rally does not skip the winner screen.
constexpr uint16_t GAME_OVER_HOLD_MS = 1000;
constexpr uint16_t MAX_FRAME_MS = 40;
constexpr int32_t FP_ONE = 256;

constexpr uint16_t COL_BG = 0x0000;
constexpr uint16_t COL_NET = 0x3186;
constexpr uint16_t COL_P1 = 0x07FF;
constexpr uint16_t COL_P2 = 0xFBE0;
constexpr uint16_t COL_BALL = 0xFFFF;
constexpr uint16_t COL_TITLE = 0xFFFF;
constexpr uint16_t COL_SUB = 0xBDF7;
