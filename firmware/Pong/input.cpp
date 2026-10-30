#include <Arduino.h>

#include "config.h"
#include "input.h"

static const uint8_t kPins[4] = {PIN_BOT_LEFT, PIN_BOT_RIGHT, PIN_TOP_LEFT,
                                 PIN_TOP_RIGHT};

static uint8_t stableMask = 0;
static uint8_t lastRaw = 0;
static uint32_t changeAt = 0;

static uint8_t readRaw() {
  uint8_t mask = 0;
  for (uint8_t i = 0; i < 4; i++) {
    if (digitalRead(kPins[i]) == LOW) {
      mask |= (uint8_t)(1u << i);
    }
  }
  return mask;
}

void inputBegin() {
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(kPins[i], INPUT_PULLUP);
  }
  lastRaw = readRaw();
  stableMask = lastRaw;
  changeAt = millis();
}

Buttons inputRead() {
  uint8_t raw = readRaw();
  uint32_t now = millis();
  if (raw != lastRaw) {
    lastRaw = raw;
    changeAt = now;
  } else if (raw != stableMask && (uint32_t)(now - changeAt) >= DEBOUNCE_MS) {
#if DEBUG_BUTTONS
    uint8_t pressed = (uint8_t)(raw & (uint8_t)~stableMask);
    if (pressed & 0x01) {
      Serial.println(F("button: bottom left"));
    }
    if (pressed & 0x02) {
      Serial.println(F("button: bottom right"));
    }
    if (pressed & 0x04) {
      Serial.println(F("button: top left"));
    }
    if (pressed & 0x08) {
      Serial.println(F("button: top right"));
    }
#endif
    stableMask = raw;
  }

  Buttons buttons;
  buttons.botLeft = stableMask & 0x01;
  buttons.botRight = stableMask & 0x02;
  buttons.topLeft = stableMask & 0x04;
  buttons.topRight = stableMask & 0x08;
  return buttons;
}
