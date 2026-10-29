# Bench card

Pins live in `firmware/Pong/config.h`. If you move a wire, change the number there too.

The picture is 320 by 480, portrait. The cyan paddle is the bottom player. The amber paddle is the top player. A tap on the glass starts the match, pauses it, and resumes it. How the program moves between those screens is in [docs/ARCHITECTURE.md](ARCHITECTURE.md).

## Reading the picture

The drawing in the README is a map of the three breadboards. It is not a photo. Read it in number order.

1. **Middle, green.** This is the big breadboard. The white card is the screen. The blue card is the Nano. Each line is one wire. The word on the left is the pin on that part. The word on the right is the Nano pin it plugs into.
2. **Left, cyan.** This is the bottom player's small breadboard. Two buttons. The left button goes to D2. The right button goes to D3. The other side of each button goes to ground.
3. **Right, amber.** This is the top player's small breadboard. Left button to D4, right button to D5, other side to ground.
4. **Bottom, white.** This is the resistor pair. A Nano pin puts out about 5 V. The screen and the touch chip want about 3.3 V. The 470 ohm resistor sits in the wire. The 1 k ohm resistor goes from that joint down to ground. Build this once for each of `CS`, `RESET`, `DC`, `MOSI`, `SCK`, and `T_CS`.

`T_DO` does not get a resistor pair. The touch chip already drives that wire at 3.3 V, straight into D12. Leave `T_IRQ` empty, and leave the screen's own `SDO` pin empty.

## Power and the display

| Nano | TFT pin | Notes |
| --- | --- | --- |
| 5V | VCC | The module accepts 3.3 V or 5 V here. Use 5 V. |
| GND | GND | Share this ground with both mini breadboards. |
| 3V3 | LED | Backlight control. High keeps the backlight on. |
| D10 | CS | Keep chip-select on D10. |
| D9 | DC / RS | High means pixel data, low means a command. |
| D8 | RESET | |
| D11 | SDI / MOSI | Hardware SPI. This pin is fixed. Also goes to `T_DIN`. |
| D13 | SCK | Hardware SPI. This pin is fixed. Also goes to `T_CLK`. |
| D12 | T_DO | Touch data out. Leave the panel's own SDO / MISO empty. |
| D7 | T_CS | Touch chip-select. The sketch holds this high while it draws. |

Leave `T_IRQ` empty. Do not tie `T_CS` to 3V3. The sketch has to drive that pin, or a tap never reaches the game.

The glass hangs off the big breadboard. Seat the header all the way, and keep the SCK and MOSI jumpers short.

## 5 V pins into a 3.3 V panel

The TFT logic pins and the touch inputs are 3.3 V. A Nano pin drives about 5 V. Do this for `CS`, `RESET`, `DC`, `MOSI`, `SCK`, and `T_CS`:

```
Nano pin ----[ 470 ohm ]----+---- TFT pin
                            |
                         [ 1k ohm ]
                            |
                           GND
```

Six 470 ohm resistors and six 1k ohm resistors cover every signal. `MOSI`'s divider feeds both TFT `SDI` and `T_DIN`. `SCK`'s divider feeds both TFT `SCK` and `T_CLK`. `T_DO` is an output from the touch chip at 3.3 V, so it goes straight to D12 with no divider. `LED` and `VCC` stay on the power pins above. The panel's `SDO` stays empty.

Some of these red boards have a solder jumper marked `J1` on the back. Bridging `J1` and powering `VCC` from 5 V lets the signal wires go straight across. The module maker says that path runs warm and ages the panel. Use it when the resistors are still in a drawer, and go back to the dividers for a board you want to keep.

## Buttons

Each mini breadboard holds two tactile buttons for one player. Seat each button so it straddles the center gap. The two legs on one side of the gap are already connected inside the switch, and the same is true of the other side. One side goes to the Nano pin. The other side goes to GND.

The sketch enables the Nano's internal pull-ups. A press reads low. The buttons themselves need no resistors.

| Player | Action | Nano |
| --- | --- | --- |
| Bottom (cyan) | Left | D2 |
| Bottom (cyan) | Right | D3 |
| Top (amber) | Left | D4 |
| Top (amber) | Right | D5 |

Left and right are as that player looks at their own paddle. If a paddle slides the wrong way, swap that player's two pin numbers in `config.h`.

Run one ground wire from each mini breadboard back to the GND rail on the big breadboard.

## Optional click

A piezo from D6 to GND clicks on paddle hits, walls, points, pause, and resume. In `config.h`, set `BUZZER_ENABLED` to `1`.

## Upload

1. Board: Arduino Nano.
2. Processor: ATmega328P (Old Bootloader). This is the one CH340 clones use.
3. If `avrdude` reports `stk500_getsync`, switch Processor to ATmega328P and upload again.
4. Use the normal Upload button. A data USB cable has to be in the Nano. Charge-only cables never show a port.
5. Open the serial monitor at 115200. It should print `Pong TFT`, then `Ready. Tap the screen to start.`
6. Press each button. The monitor prints `button: bottom left` and the matching line for the other three.
7. Tap the glass. The monitor prints `touch: tap`, and the ball leaves the middle of the table.
