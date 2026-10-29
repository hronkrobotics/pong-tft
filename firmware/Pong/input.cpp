#include <Arduino.h>

#include "config.h"
#include "input.h"

static const uint8_t kPins[4] = {PIN_P1_UP, PIN_P1_DOWN, PIN_P2_UP, PIN_P2_DOWN};

static uint8_t stableMask = 0;
static uint8_t lastRaw = 0;
static uint32_t changeAt = 0;
static bool pendingEdge = false;

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
  pendingEdge = false;
}

Buttons inputRead() {
  uint8_t raw = readRaw();
  uint32_t now = millis();
  if (raw != lastRaw) {
    lastRaw = raw;
    changeAt = now;
  } else if (raw != stableMask && (uint32_t)(now - changeAt) >= DEBOUNCE_MS) {
    uint8_t pressed = (uint8_t)(raw & (uint8_t)~stableMask);
    if (pressed) {
      pendingEdge = true;
    }
#if DEBUG_BUTTONS
    if (pressed & 0x01) {
      Serial.println(F("button: left up"));
    }
    if (pressed & 0x02) {
      Serial.println(F("button: left down"));
    }
    if (pressed & 0x04) {
      Serial.println(F("button: right up"));
    }
    if (pressed & 0x08) {
      Serial.println(F("button: right down"));
    }
#endif
    stableMask = raw;
  }

  Buttons buttons;
  buttons.p1Up = stableMask & 0x01;
  buttons.p1Down = stableMask & 0x02;
  buttons.p2Up = stableMask & 0x04;
  buttons.p2Down = stableMask & 0x08;
  return buttons;
}

bool inputConsumePress() {
  bool edge = pendingEdge;
  pendingEdge = false;
  return edge;
}
