#pragma once

#include <stdint.h>

void displayBegin();
void displayFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void displayFillScreen(uint16_t color);
void displayText(int16_t x, int16_t y, const char* text, uint16_t color,
                 uint16_t bg, uint8_t scale);
int16_t displayTextWidth(const char* text, uint8_t scale);
