# Working on this game

Aayan Rahman is the collaborator this project is set up for. The GitHub account name is `aayanrahman` (two a's). `AyanRahman` is a different person's account. Once the repository is on GitHub, add that account under Settings → Collaborators.

The sketch has no libraries to install. Physics can be changed and checked on a computer, then flashed to the Nano when the table is on the desk.

## Where to edit

| Change | File |
| --- | --- |
| Pins, colors, points to win, paddle size, ball speed | `firmware/Pong/config.h` |
| Bounce, scoring, serves | `firmware/Pong/game.cpp` |
| What gets drawn, and when | `firmware/Pong/render.cpp` |
| ILI9488 / ILI9486 init, SPI pixels | `firmware/Pong/display.cpp` |
| Button debounce | `firmware/Pong/input.cpp` |
| Beeps and the serial score line | `firmware/Pong/feedback.cpp` |
| Letter shapes | `tools/make_font.py`, then rerun it |

`config.h` is the file most games of this size want first. `POINTS_TO_WIN`, `PADDLE_H`, `SERVE_SPEED`, `ENGLISH`, and `SPEED_BUMP` are the knobs for how the match feels.

[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) shows how the files fit together and what happens in one frame.

## Checks

```bash
make test
```

That compiles `firmware/Pong/game.cpp` with the tests in `test/` and plays rallies, misses, and a win without the Nano attached.

```bash
make firmware
```

That needs [arduino-cli](https://arduino.github.io/arduino-cli/) and the `arduino:avr` core. It builds the sketch for a Nano with the old bootloader, which is the usual CH340 clone. `make firmware-9486` builds the same sketch against the ILI9486 init.

The Nano has 30,720 bytes of flash with the old bootloader and 2,048 bytes of RAM. The current sketch uses about a third of the flash and a sixth of the RAM. Draw by sending pixels straight to the panel. A stored picture of the whole 480×320 screen does not fit.

## Notes from the bench

- `PIN_CS` stays on D10. On this chip that pin is the SPI slave-select, and it has to be an output.
- D11 and D13 are the hardware SPI pins.
- The font covers space, digits, and `A`–`Z`. Lowercase is drawn as uppercase.
- Serial output is 115200 baud.
