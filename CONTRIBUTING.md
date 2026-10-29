# Working on this game

The rules can be changed and checked on a computer, then flashed to the Nano when the breadboards are on the desk. The README and [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) follow the same outline as [solareyes](https://github.com/hronkrobotics/solareyes): a short system picture in the README, then one architecture page for the state machine, the files, and a single frame.

AyanRahman is the collaborator this project is set up for. The GitHub account name is `AyanRahman` (no space). Add that account under Settings → Collaborators.

## Where to edit

| Change | File |
| --- | --- |
| Pins, colors, points to win, paddle length, ball speed | `firmware/Pong/config.h` |
| The four steps of a match, bounce, scoring, serves | `firmware/Pong/game.cpp` |
| What gets drawn, and when | `firmware/Pong/render.cpp` |
| ILI9488 / ILI9486 init, SPI pixels | `firmware/Pong/display.cpp` |
| Button debounce | `firmware/Pong/input.cpp` |
| Tap detection | `firmware/Pong/touch.cpp` |
| Boot and the frame loop | `firmware/Pong/Pong.ino` |
| Beeps and the serial score line | `firmware/Pong/feedback.cpp` |
| Letter shapes | `tools/make_font.py`, then rerun it |
| The wiring picture | `docs/wiring.svg` and `docs/WIRING.md` |
| How the program is explained | `docs/ARCHITECTURE.md` |

`game.cpp` has no Arduino code in it, so it builds and runs on a laptop. Keep it that way. Pins, SPI, and serial belong in the other files.

`POINTS_TO_WIN`, `PADDLE_LENGTH`, `SERVE_SPEED`, `ENGLISH`, and `MAX_VX` are the knobs for how the match feels. The ball does not speed up. A new rule needs a check in `test/test_game.cpp`.

## Checks

```bash
make test
```

That compiles `firmware/Pong/game.cpp` with the tests in `test/` and plays serves, wall bounces, paddle hits, a miss, a pause, and a win without the Nano attached.

```bash
make firmware
```

That needs [arduino-cli](https://arduino.github.io/arduino-cli/) and the `arduino:avr` core. It builds the sketch for a Nano with the old bootloader, which is the usual CH340 clone. `make firmware-9486` builds the same sketch against the ILI9486 init.

```bash
pio run -e nano_old -t upload
pio device monitor -b 115200
```

The current ILI9488 build uses 10,734 bytes of flash (34% of 30,720) and 396 bytes of RAM (19% of 2,048). Draw by sending pixels straight to the panel. A stored picture of the whole 320×480 screen does not fit.

## Notes from the bench

- `PIN_CS` stays on D10. On this chip that pin is the SPI slave-select, and it has to be an output.
- `PIN_T_CS` stays high whenever the screen is being written. The touch chip shares MOSI and SCK.
- D11 and D13 are the hardware SPI pins. D12 is the touch chip's data out.
- Portrait is `ROTATION` 0. Use 2 when the picture is upside down.
- The font covers space, digits, and `A`–`Z`. Lowercase is drawn as uppercase.
- Serial output is 115200 baud.
