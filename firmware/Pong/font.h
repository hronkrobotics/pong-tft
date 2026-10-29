#pragma once

#include <stdint.h>

// Five column bytes in program memory. Bit 0 is the top pixel.
// Lowercase letters map to uppercase. Any other byte maps to a space.
const uint8_t* fontGlyph(char c);
