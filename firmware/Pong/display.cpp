#include <Arduino.h>
#include <SPI.h>
#include <avr/io.h>

#include "config.h"
#include "display.h"
#include "font.h"

static_assert(ROTATION == 0 || ROTATION == 2, "use a portrait rotation");
static_assert(SCREEN_W == 320 && SCREEN_H == 480, "3.5 inch portrait panel");

static volatile uint8_t* csPort = 0;
static volatile uint8_t* dcPort = 0;
static uint8_t csBit = 0;
static uint8_t dcBit = 0;

struct Rgb {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

static inline void spiSend(uint8_t value) __attribute__((always_inline));
static inline void spiSend(uint8_t value) {
  SPDR = value;
  while (!(SPSR & _BV(SPIF))) {
  }
}

static Rgb expand565(uint16_t color) {
  // ILI9488 SPI wants 18-bit pixels: one byte each for red, green, and blue.
  Rgb out;
  out.r = (uint8_t)((color >> 8) & 0xF8);
  out.g = (uint8_t)((color >> 3) & 0xFC);
  out.b = (uint8_t)((color << 3) & 0xF8);
  return out;
}

static void cmdN(uint8_t command, const uint8_t* bytes, uint8_t count) {
  *csPort &= ~csBit;
  *dcPort &= ~dcBit;
  spiSend(command);
  if (count) {
    *dcPort |= dcBit;
    while (count--) {
      spiSend(*bytes++);
    }
  }
  *csPort |= csBit;
}

static void cmdP(uint8_t command, const uint8_t* progmemBytes, uint8_t count) {
  *csPort &= ~csBit;
  *dcPort &= ~dcBit;
  spiSend(command);
  *dcPort |= dcBit;
  while (count--) {
    spiSend(pgm_read_byte(progmemBytes++));
  }
  *csPort |= csBit;
}

static void sendCmd(uint8_t command) {
  *dcPort &= ~dcBit;
  spiSend(command);
}

static void sendByte(uint8_t value) {
  *dcPort |= dcBit;
  spiSend(value);
}

static void sendWord(uint16_t value) {
  sendByte((uint8_t)(value >> 8));
  sendByte((uint8_t)value);
}

static void openPixels(int16_t x, int16_t y, int16_t w, int16_t h) {
  int16_t x1 = (int16_t)(x + w - 1);
  int16_t y1 = (int16_t)(y + h - 1);
  *csPort &= ~csBit;
  sendCmd(0x2A);
  sendWord((uint16_t)x);
  sendWord((uint16_t)x1);
  sendCmd(0x2B);
  sendWord((uint16_t)y);
  sendWord((uint16_t)y1);
  sendCmd(0x2C);
  *dcPort |= dcBit;
}

static void closePixels() { *csPort |= csBit; }

static void pushRgb(const Rgb& color) {
  spiSend(color.r);
  spiSend(color.g);
  spiSend(color.b);
}

// Register values follow the panel bring-up used by 3.5 inch SPI boards:
// gamma, power, 18-bit pixels, then leave sleep. displayBegin() turns the
// panel on after this returns.
#if TFT_DRIVER == 9486

static const uint8_t GAMMA_P[] PROGMEM = {
    0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98,
    0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00};
static const uint8_t GAMMA_N[] PROGMEM = {
    0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
    0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00};

static void initPanel() {
  cmdN(0x01, 0, 0);
  delay(120);
  cmdN(0x11, 0, 0);
  delay(120);
  const uint8_t pix[] = {0x66};
  cmdN(0x3A, pix, 1);
  const uint8_t power1[] = {0x0E, 0x0E};
  cmdN(0xC0, power1, 2);
  const uint8_t power2[] = {0x41, 0x00};
  cmdN(0xC1, power2, 2);
  const uint8_t power3[] = {0x55};
  cmdN(0xC2, power3, 1);
  const uint8_t vcom[] = {0x00, 0x00, 0x00, 0x00};
  cmdN(0xC5, vcom, 4);
  cmdP(0xE0, GAMMA_P, 15);
  cmdP(0xE1, GAMMA_N, 15);
}

#else

static const uint8_t GAMMA_P[] PROGMEM = {
    0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78,
    0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F};
static const uint8_t GAMMA_N[] PROGMEM = {
    0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45,
    0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F};

static void initPanel() {
  cmdP(0xE0, GAMMA_P, 15);
  cmdP(0xE1, GAMMA_N, 15);
  const uint8_t power1[] = {0x17, 0x15};
  cmdN(0xC0, power1, 2);
  const uint8_t power2[] = {0x41};
  cmdN(0xC1, power2, 1);
  const uint8_t vcom[] = {0x00, 0x12, 0x80};
  cmdN(0xC5, vcom, 3);
  const uint8_t pix[] = {0x66};
  cmdN(0x3A, pix, 1);
  const uint8_t interfaceMode[] = {0x00};
  cmdN(0xB0, interfaceMode, 1);
  const uint8_t frame[] = {0xA0};
  cmdN(0xB1, frame, 1);
  const uint8_t inversion[] = {0x02};
  cmdN(0xB4, inversion, 1);
  const uint8_t function[] = {0x02, 0x02, 0x3B};
  cmdN(0xB6, function, 3);
  const uint8_t entry[] = {0xC6};
  cmdN(0xB7, entry, 1);
  const uint8_t adjust[] = {0xA9, 0x51, 0x2C, 0x82};
  cmdN(0xF7, adjust, 4);
  cmdN(0x11, 0, 0);
  delay(120);
}

#endif

void displayBegin() {
  pinMode(PIN_CS, OUTPUT);
  pinMode(PIN_DC, OUTPUT);
  pinMode(PIN_RST, OUTPUT);
  pinMode(PIN_T_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_RST, HIGH);
  digitalWrite(PIN_T_CS, HIGH);

  csPort = portOutputRegister(digitalPinToPort(PIN_CS));
  csBit = digitalPinToBitMask(PIN_CS);
  dcPort = portOutputRegister(digitalPinToPort(PIN_DC));
  dcBit = digitalPinToBitMask(PIN_DC);

  SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE0));

  digitalWrite(PIN_RST, LOW);
  delay(20);
  digitalWrite(PIN_RST, HIGH);
  delay(150);

  initPanel();

  // Portrait. Rotation 0 sets MX, rotation 2 sets MY. BGR is applied below.
  uint8_t madctl = (ROTATION == 2) ? (uint8_t)0x80 : (uint8_t)0x40;
  if (TFT_MIRROR) {
    madctl ^= 0x40;
  }
  if (TFT_USE_BGR) {
    madctl |= 0x08;
  }

  *csPort &= ~csBit;
  sendCmd(0x36);
  sendByte(madctl);
  sendCmd(TFT_INVERT ? 0x21 : 0x20);
  sendCmd(0x29);
  *csPort |= csBit;
  delay(20);
}

void displayFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (w <= 0 || h <= 0) {
    return;
  }
  if (x < 0) {
    w = (int16_t)(w + x);
    x = 0;
  }
  if (y < 0) {
    h = (int16_t)(h + y);
    y = 0;
  }
  if (w <= 0 || h <= 0 || x >= SCREEN_W || y >= SCREEN_H) {
    return;
  }
  if (x + w > SCREEN_W) {
    w = (int16_t)(SCREEN_W - x);
  }
  if (y + h > SCREEN_H) {
    h = (int16_t)(SCREEN_H - y);
  }
  if (w <= 0 || h <= 0) {
    return;
  }

  openPixels(x, y, w, h);
  Rgb rgb = expand565(color);
  uint32_t count = (uint32_t)w * (uint32_t)h;
  while (count--) {
    pushRgb(rgb);
  }
  closePixels();
}

void displayFillScreen(uint16_t color) {
  displayFillRect(0, 0, SCREEN_W, SCREEN_H, color);
}

int16_t displayTextWidth(const char* text, uint8_t scale) {
  int16_t count = 0;
  while (text[count] != 0) {
    count++;
  }
  if (count == 0 || scale == 0) {
    return 0;
  }
  return (int16_t)(count * (6 * scale) - scale);
}

void displayText(int16_t x, int16_t y, const char* text, uint16_t color,
                 uint16_t bg, uint8_t scale) {
  if (scale == 0 || text == 0 || text[0] == 0) {
    return;
  }

  uint8_t columns[20][5];
  int16_t count = 0;
  while (text[count] != 0 && count < 20) {
    const uint8_t* glyph = fontGlyph(text[count]);
    for (uint8_t col = 0; col < 5; col++) {
      columns[count][col] = pgm_read_byte(glyph + col);
    }
    count++;
  }
  if (count == 0) {
    return;
  }

  int16_t width = (int16_t)(count * (6 * scale) - scale);
  int16_t height = (int16_t)(7 * scale);
  if (x < 0 || y < 0 || x + width > SCREEN_W || y + height > SCREEN_H) {
    return;
  }

  Rgb fg = expand565(color);
  Rgb background = expand565(bg);
  openPixels(x, y, width, height);
  for (uint8_t row = 0; row < 7; row++) {
    for (uint8_t sy = 0; sy < scale; sy++) {
      for (int16_t index = 0; index < count; index++) {
        for (uint8_t col = 0; col < 5; col++) {
          bool on = columns[index][col] & (uint8_t)(1u << row);
          for (uint8_t sx = 0; sx < scale; sx++) {
            pushRgb(on ? fg : background);
          }
        }
        if (index + 1 < count) {
          for (uint8_t sx = 0; sx < scale; sx++) {
            pushRgb(background);
          }
        }
      }
    }
  }
  closePixels();
}
