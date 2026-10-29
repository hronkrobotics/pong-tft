// Two-player Pong for an Arduino Nano clone and a 3.5 inch SPI TFT.
// Open this folder in the Arduino IDE. Wiring is in docs/WIRING.md.
// Pins, winning score, and ball speed live in config.h.

#include <Arduino.h>

#include "config.h"
#include "display.h"
#include "feedback.h"
#include "game.h"
#include "input.h"
#include "render.h"

static_assert(PIN_SCK == 13 && PIN_MOSI == 11 && PIN_MISO == 12,
              "Nano hardware SPI pins");
static_assert(PIN_BUZZER == 6, "buzzer pin");

static Game game;
static uint32_t lastMs = 0;

void setup() {
  Serial.begin(SERIAL_BAUD);
  inputBegin();
#if BUZZER_ENABLED
  pinMode(PIN_BUZZER, OUTPUT);
#endif

  Serial.println(F("Pong TFT"));
  Serial.println(F("Left paddle  D2 up   D3 down"));
  Serial.println(F("Right paddle D4 up   D5 down"));
  Serial.println(F("TFT CS D10  DC D9  RST D8  MOSI D11  SCK D13"));
  Serial.println(F("Init display..."));
  displayBegin();
  gameInit(game);
  Serial.println(F("Clearing screen..."));
  renderBegin(game);
  lastMs = millis();
  Serial.println(F("Ready. Press any button to serve."));
}

void loop() {
  uint32_t now = millis();
  uint32_t elapsed = now - lastMs;
  lastMs = now;
  uint16_t dt = (elapsed > 65535ul) ? (uint16_t)65535u : (uint16_t)elapsed;

  Buttons buttons = inputRead();
  bool edge = inputConsumePress();
  GameEvent event = gameUpdate(game, buttons, edge, now, dt);
  feedback(game, event);
  render(game);
}
