# Bench card

Pins live in `firmware/Pong/config.h`. If you move a wire, change the number there too.

Why the dividers exist, and why D11 and D13 are shared, is in [ELECTRICAL.md](ELECTRICAL.md). How the breadboards sit is in [HARDWARE.md](HARDWARE.md). The build order is in [TUTORIAL.md](TUTORIAL.md).

## Power

No resistors on these three.

| Nano | Screen |
| --- | --- |
| 5V | VCC |
| GND | GND |
| 3V3 | LED |

GND is also the ground of both button boards, and of the divider board if the pairs are not on the big breadboard.

## Signals

| Nano | Screen | How |
| --- | --- | --- |
| D10 | CS | one divider |
| D8 | RESET | one divider |
| D9 | DC / RS | one divider |
| D11 | SDI / MOSI, and T_DIN | one divider, two wires from the joint |
| D13 | SCK, and T_CLK | one divider, two wires from the joint |
| D7 | T_CS | one divider |
| D12 | T_DO | straight wire |

Leave `T_IRQ` empty. Leave `SDO` / `MISO` empty.

## One divider

The 470 ohm resistor is in the wire. The 1k resistor goes from the joint to ground. That joint is about 3.4 V.

```
Nano wire ----[ 470 ohm ]----+---- screen wire
                             |
                          [ 1k ohm ]
                             |
                        ground rail
```

A 220 ohm resistor in the wire and a 470 ohm resistor to ground is the same joint voltage. The smaller resistor stays on the Nano side. The larger one stays on the ground side.

D11 and D13 each get a second screen wire from that same joint:

```
D11 ----[ series ]----+---- SDI
                      |
                      +---- T_DIN
                      |
                   [ to ground ]
                      |
                    ground

D13 ----[ series ]----+---- SCK
                      |
                      +---- T_CLK
                      |
                   [ to ground ]
                      |
                    ground
```

## Buttons

One side of the button goes to the Nano pin. The other side goes to ground. No resistors. The sketch turns on the Nano's internal pull-ups, so a press reads low.

| Player | Action | Nano |
| --- | --- | --- |
| Bottom (blue) | Left | D2 |
| Bottom (blue) | Right | D3 |
| Top (green) | Left | D4 |
| Top (green) | Right | D5 |

Left and right are as that player looks at their own paddle. If a paddle slides the wrong way, swap that player's two pin numbers in `config.h`.

## Optional click

A piezo from D6 to GND. In `config.h`, set `BUZZER_ENABLED` to `1`.
