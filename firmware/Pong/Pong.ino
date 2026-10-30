// Two-player Pong for an Arduino Nano clone and a 3.5 inch SPI TFT.
// The glass is portrait: blue paddle on the bottom, green paddle on the top.
// Open this folder in the Arduino IDE. Wiring is in docs/WIRING.md.
// Pins, winning score, and ball speed live in config.h.

#include <Arduino.h>

#include "config.h"
#include "display.h"
#include "feedback.h"
#include "game.h"
#include "input.h"
#include "render.h"
#include "touch.h"

static_assert(PIN_SCK == 13 && PIN_MOSI == 11 && PIN_MISO == 12,
              "Nano hardware SPI pins");
static_assert(PIN_BUZZER == 6, "buzzer pin");
static_assert(PIN_T_CS == 7, "touch chip select");
static_assert(PIN_T_CS != PIN_CS && PIN_T_CS != PIN_DC && PIN_T_CS != PIN_RST &&
                  PIN_T_CS != PIN_BUZZER,
              "touch chip select is its own pin");

static Game game;
static uint32_t lastMs = 0;

void setup() {
  Serial.begin(SERIAL_BAUD);
  inputBegin();
  touchBegin();
#if BUZZER_ENABLED
  pinMode(PIN_BUZZER, OUTPUT);
#endif

  Serial.println(F("Pong TFT"));
  Serial.println(F("Bottom paddle D2 left  D3 right"));
  Serial.println(F("Top paddle    D4 left  D5 right"));
  Serial.println(F("TFT CS D10  DC D9  RST D8  MOSI D11  SCK D13"));
  Serial.println(F("Touch T_CS D7  T_DO D12  T_CLK D13  T_DIN D11"));
  Serial.println(F("Init display..."));
  displayBegin();
  gameInit(game);
  Serial.println(F("Clearing screen..."));
  renderBegin(game);
  lastMs = millis();
  Serial.println(F("Ready. Tap the screen to start."));
}

void loop() {
  uint32_t now = millis();
  uint32_t elapsed = now - lastMs;
  lastMs = now;
  uint16_t dt = (elapsed > 65535ul) ? (uint16_t)65535u : (uint16_t)elapsed;

  Buttons buttons = inputRead();
  bool tap = touchConsumeTap();
  GameEvent event = gameUpdate(game, buttons, tap, now, dt);
  feedback(game, event);
  render(game);
}
