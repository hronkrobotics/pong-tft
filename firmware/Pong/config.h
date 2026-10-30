#pragma once

#include <stdint.h>

// Driver on the 3.5 inch module. 9488 is the common MSP3520 / MSP3521 board.
// Change this to 9486 only when the silkscreen says ILI9486.
#ifndef TFT_DRIVER
#define TFT_DRIVER 9488
#endif

// Portrait, so one paddle sits at the top and one at the bottom.
// Use 2 when the picture is upside down.
constexpr uint8_t ROTATION = 0;

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
// picture is solid and the SCK wire is short. The touch chip is read at
// TOUCH_SPI_HZ; its bus tops out near 2.5 MHz.
constexpr uint32_t SPI_HZ = 4000000;
constexpr uint32_t TOUCH_SPI_HZ = 2000000;

// D10 is the AVR SPI slave-select pin. It has to be an output or the Nano
// drops out of SPI master mode. Leave chip-select on D10.
constexpr uint8_t PIN_CS = 10;
constexpr uint8_t PIN_DC = 9;
constexpr uint8_t PIN_RST = 8;
constexpr uint8_t PIN_SCK = 13;
constexpr uint8_t PIN_MOSI = 11;
constexpr uint8_t PIN_MISO = 12;

// XPT2046 chip-select on the touch header. Keep it high while the panel is
// written. T_CLK shares SCK, T_DIN shares MOSI, T_DO goes to MISO.
constexpr uint8_t PIN_T_CS = 7;

// Bottom mini breadboard is the blue paddle. Top mini breadboard is green.
// Left and right are along the glass, the way each player sees their paddle.
constexpr uint8_t PIN_BOT_LEFT = 2;
constexpr uint8_t PIN_BOT_RIGHT = 3;
constexpr uint8_t PIN_TOP_LEFT = 4;
constexpr uint8_t PIN_TOP_RIGHT = 5;

// Optional piezo between D6 and GND. Set this to 1 when the buzzer is fitted.
#define BUZZER_ENABLED 0
constexpr uint8_t PIN_BUZZER = 6;

// Prints one line each time a button goes down, or a finger lands, so a
// wiring check can use the serial monitor before the picture is trusted.
#define DEBUG_BUTTONS 1

constexpr uint8_t DEBOUNCE_MS = 20;
constexpr uint32_t SERIAL_BAUD = 115200;

// A finger landing on the glass. Z1 outside this window is noise or no touch.
constexpr uint16_t TOUCH_Z_MIN = 200;
constexpr uint16_t TOUCH_Z_MAX = 4000;
constexpr uint16_t TAP_GAP_MS = 300;

constexpr int16_t SCREEN_W = 320;
constexpr int16_t SCREEN_H = 480;

constexpr int16_t PADDLE_LENGTH = 80;
constexpr int16_t PADDLE_THICK = 12;
constexpr int16_t PADDLE_INSET = 14;
constexpr int16_t BALL_SIZE = 14;

constexpr int16_t TOP_PADDLE_Y = PADDLE_INSET;
constexpr int16_t BOTTOM_PADDLE_Y = SCREEN_H - PADDLE_INSET - PADDLE_THICK;

constexpr int16_t BALL_REST_X = (SCREEN_W - BALL_SIZE) / 2;
constexpr int16_t BALL_REST_Y =
    (TOP_PADDLE_Y + PADDLE_THICK + BOTTOM_PADDLE_Y - BALL_SIZE) / 2;

constexpr int16_t PADDLE_SPEED = 220;  // pixels per second
constexpr int16_t SERVE_SPEED = 180;   // vertical speed, constant for the whole match
constexpr int16_t MAX_VX = 140;        // sideways cap from an off-center hit
constexpr int16_t ENGLISH = 4;         // sideways speed per pixel off the paddle center

constexpr uint8_t POINTS_TO_WIN = 11;
constexpr uint16_t POINT_PAUSE_MS = 700;
// A tap in this window after the winning point is ignored, so a finger still
// on the glass does not skip the winner's name.
constexpr uint16_t GAME_OVER_HOLD_MS = 1000;
constexpr uint16_t MAX_FRAME_MS = 40;
constexpr int32_t FP_ONE = 256;

constexpr uint16_t COL_BG = 0x0000;
constexpr uint16_t COL_NET = 0x3186;
constexpr uint16_t COL_P1 = 0x001F;
constexpr uint16_t COL_P2 = 0x07E0;
constexpr uint16_t COL_BALL = 0xFFFF;
constexpr uint16_t COL_TITLE = 0xFFFF;
constexpr uint16_t COL_SUB = 0xBDF7;
