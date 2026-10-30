#include <Arduino.h>
#include <SPI.h>

#include "config.h"
#include "touch.h"

static bool stableDown = false;
static uint8_t seenDown = 0;
static uint32_t lastTapMs = 0;
static bool haveTap = false;

// 12-bit sample. The first clock after the command is a null bit, then 12
// data bits, so the 16 clocks shifted down by 3 are the conversion.
static uint16_t sample(uint8_t command) {
  SPI.transfer(command);
  uint16_t hi = SPI.transfer(0);
  uint16_t lo = SPI.transfer(0);
  return (uint16_t)(((hi << 8) | lo) >> 3);
}

static uint16_t readZ() {
  digitalWrite(PIN_CS, HIGH);
  digitalWrite(PIN_T_CS, LOW);
  delayMicroseconds(2);
  (void)sample(0xB1);
  uint16_t z = sample(0xB1);
  SPI.transfer(0x80);  // leave the touch ADC powered down
  digitalWrite(PIN_T_CS, HIGH);
  return z;
}

void touchBegin() {
  pinMode(PIN_T_CS, OUTPUT);
  digitalWrite(PIN_T_CS, HIGH);
  stableDown = false;
  seenDown = 0;
  lastTapMs = 0;
  haveTap = false;
}

bool touchConsumeTap() {
  // The panel transaction stays open across frames. Drop it, read the touch
  // chip at its own clock, then open the panel clock again.
  SPI.endTransaction();
  SPI.beginTransaction(SPISettings(TOUCH_SPI_HZ, MSBFIRST, SPI_MODE0));
  uint16_t z = readZ();
  SPI.endTransaction();
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE0));

  bool raw = z >= TOUCH_Z_MIN && z <= TOUCH_Z_MAX;
  if (raw) {
    if (seenDown < 2) {
      seenDown++;
    }
  } else {
    seenDown = 0;
  }
  bool down = seenDown >= 2;
  bool edge = down && !stableDown;
  stableDown = down;
  if (!edge) {
    return false;
  }
  uint32_t now = millis();
  if (haveTap && (uint32_t)(now - lastTapMs) < TAP_GAP_MS) {
    return false;
  }
  haveTap = true;
  lastTapMs = now;
#if DEBUG_BUTTONS
  Serial.println(F("touch: tap"));
#endif
  return true;
}
