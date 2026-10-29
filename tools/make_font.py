#!/usr/bin/env python3
"""Build firmware/Pong/font.cpp from the 5x7 glyphs below.

Bit 0 of each column is the top pixel. Edit a picture here, then run this
script from the repo root. The Arduino sketch compiles the generated file
and does not need Python.
"""

import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "firmware", "Pong", "font.cpp")

# Each glyph is 7 rows of 5 columns. '#' is ink.
GLYPHS = {
    " ": [
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
        ".....",
    ],
    "0": [
        ".###.",
        "#...#",
        "#..##",
        "#.#.#",
        "##..#",
        "#...#",
        ".###.",
    ],
    "1": [
        "..#..",
        ".##..",
        "..#..",
        "..#..",
        "..#..",
        "..#..",
        ".###.",
    ],
    "2": [
        ".###.",
        "#...#",
        "....#",
        "..##.",
        ".#...",
        "#....",
        "#####",
    ],
    "3": [
        ".###.",
        "#...#",
        "....#",
        "..##.",
        "....#",
        "#...#",
        ".###.",
    ],
    "4": [
        "...#.",
        "..##.",
        ".#.#.",
        "#..#.",
        "#####",
        "...#.",
        "...#.",
    ],
    "5": [
        "#####",
        "#....",
        "####.",
        "....#",
        "....#",
        "#...#",
        ".###.",
    ],
    "6": [
        ".###.",
        "#....",
        "#....",
        "####.",
        "#...#",
        "#...#",
        ".###.",
    ],
    "7": [
        "#####",
        "....#",
        "...#.",
        "..#..",
        ".#...",
        ".#...",
        ".#...",
    ],
    "8": [
        ".###.",
        "#...#",
        "#...#",
        ".###.",
        "#...#",
        "#...#",
        ".###.",
    ],
    "9": [
        ".###.",
        "#...#",
        "#...#",
        ".####",
        "....#",
        "....#",
        ".###.",
    ],
    "A": [
        ".###.",
        "#...#",
        "#...#",
        "#####",
        "#...#",
        "#...#",
        "#...#",
    ],
    "B": [
        "####.",
        "#...#",
        "#...#",
        "####.",
        "#...#",
        "#...#",
        "####.",
    ],
    "C": [
        ".###.",
        "#...#",
        "#....",
        "#....",
        "#....",
        "#...#",
        ".###.",
    ],
    "D": [
        "####.",
        "#...#",
        "#...#",
        "#...#",
        "#...#",
        "#...#",
        "####.",
    ],
    "E": [
        "#####",
        "#....",
        "#....",
        "####.",
        "#....",
        "#....",
        "#####",
    ],
    "F": [
        "#####",
        "#....",
        "#....",
        "####.",
        "#....",
        "#....",
        "#....",
    ],
    "G": [
        ".###.",
        "#...#",
        "#....",
        "#.###",
        "#...#",
        "#...#",
        ".###.",
    ],
    "H": [
        "#...#",
        "#...#",
        "#...#",
        "#####",
        "#...#",
        "#...#",
        "#...#",
    ],
    "I": [
        ".###.",
        "..#..",
        "..#..",
        "..#..",
        "..#..",
        "..#..",
        ".###.",
    ],
    "J": [
        "..###",
        "...#.",
        "...#.",
        "...#.",
        "#..#.",
        "#..#.",
        ".##..",
    ],
    "K": [
        "#...#",
        "#..#.",
        "#.#..",
        "##...",
        "#.#..",
        "#..#.",
        "#...#",
    ],
    "L": [
        "#....",
        "#....",
        "#....",
        "#....",
        "#....",
        "#....",
        "#####",
    ],
    "M": [
        "#...#",
        "##.##",
        "#.#.#",
        "#...#",
        "#...#",
        "#...#",
        "#...#",
    ],
    "N": [
        "#...#",
        "##..#",
        "#.#.#",
        "#..##",
        "#...#",
        "#...#",
        "#...#",
    ],
    "O": [
        ".###.",
        "#...#",
        "#...#",
        "#...#",
        "#...#",
        "#...#",
        ".###.",
    ],
    "P": [
        "####.",
        "#...#",
        "#...#",
        "####.",
        "#....",
        "#....",
        "#....",
    ],
    "Q": [
        ".###.",
        "#...#",
        "#...#",
        "#...#",
        "#.#.#",
        "#..#.",
        ".##.#",
    ],
    "R": [
        "####.",
        "#...#",
        "#...#",
        "####.",
        "#.#..",
        "#..#.",
        "#...#",
    ],
    "S": [
        ".###.",
        "#...#",
        "#....",
        ".###.",
        "....#",
        "#...#",
        ".###.",
    ],
    "T": [
        "#####",
        "..#..",
        "..#..",
        "..#..",
        "..#..",
        "..#..",
        "..#..",
    ],
    "U": [
        "#...#",
        "#...#",
        "#...#",
        "#...#",
        "#...#",
        "#...#",
        ".###.",
    ],
    "V": [
        "#...#",
        "#...#",
        "#...#",
        "#...#",
        ".#.#.",
        ".#.#.",
        "..#..",
    ],
    "W": [
        "#...#",
        "#...#",
        "#...#",
        "#.#.#",
        "#.#.#",
        "##.##",
        "#...#",
    ],
    "X": [
        "#...#",
        "#...#",
        ".#.#.",
        "..#..",
        ".#.#.",
        "#...#",
        "#...#",
    ],
    "Y": [
        "#...#",
        "#...#",
        ".#.#.",
        "..#..",
        "..#..",
        "..#..",
        "..#..",
    ],
    "Z": [
        "#####",
        "....#",
        "...#.",
        "..#..",
        ".#...",
        "#....",
        "#####",
    ],
}


def pack(rows):
    if len(rows) != 7 or any(len(row) != 5 for row in rows):
        raise SystemExit("every glyph needs 7 rows of 5 columns")
    cols = []
    for col in range(5):
        value = 0
        for row in range(7):
            pixel = rows[row][col]
            if pixel not in ".#":
                raise SystemExit("use only '.' and '#'")
            if pixel == "#":
                value |= 1 << row
        cols.append(value)
    return cols


def preview(ch, cols):
    print(" %s" % ch)
    for row in range(7):
        print("  " + "".join("#" if cols[col] & (1 << row) else "." for col in range(5)))


def main():
    required = [" "] + [chr(c) for c in range(ord("0"), ord("9") + 1)]
    required += [chr(c) for c in range(ord("A"), ord("Z") + 1)]
    missing = [ch for ch in required if ch not in GLYPHS]
    if missing:
        raise SystemExit("missing glyphs: " + " ".join(missing))

    packed = {ch: pack(rows) for ch, rows in GLYPHS.items()}
    for ch in ("P", "O", "N", "G", "0", "1", "8"):
        preview(ch, packed[ch])

    lines = []
    lines.append('#include <Arduino.h>')
    lines.append('#include "font.h"')
    lines.append("")
    lines.append("// Generated by tools/make_font.py. Edit the pictures there, then rerun it.")
    lines.append("// ASCII 32 through 90. Bit 0 of a column is the top pixel.")
    lines.append("static const uint8_t GLYPHS[] PROGMEM = {")
    for code in range(32, 91):
        ch = chr(code)
        cols = packed.get(ch, packed[" "])
        shown = ch if ch != " " else "space"
        hexes = ", ".join("0x%02x" % value for value in cols)
        lines.append("    %s, // %s" % (hexes, shown))
    lines.append("};")
    lines.append("")
    lines.append("const uint8_t* fontGlyph(char c) {")
    lines.append("  if (c >= 'a' && c <= 'z') {")
    lines.append("    c = (char)(c - ('a' - 'A'));")
    lines.append("  }")
    lines.append("  uint8_t index = 0;")
    lines.append("  if ((uint8_t)c >= 32 && (uint8_t)c <= 90) {")
    lines.append("    index = (uint8_t)((uint8_t)c - 32);")
    lines.append("  }")
    lines.append("  return GLYPHS + (uint16_t)index * 5;")
    lines.append("}")
    lines.append("")
    text = "\n".join(lines)
    with open(OUT, "w", encoding="utf-8") as handle:
        handle.write(text)
    print("wrote %s" % OUT)


if __name__ == "__main__":
    sys.exit(main() or 0)
