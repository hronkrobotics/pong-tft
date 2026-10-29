# Bench card

Pins live in `firmware/Pong/config.h`. If you move a wire, change the number there too.

The picture is 480 by 320, landscape. The cyan paddle is the left player. The amber paddle is the right player.

## Power and the display

| Nano | TFT pin | Notes |
| --- | --- | --- |
| 5V | VCC | The module accepts 3.3 V or 5 V here. Use 5 V. |
| GND | GND | Share this ground with both mini breadboards. |
| 3V3 | LED | Backlight control. High keeps the backlight on. |
| D10 | CS | Keep chip-select on D10. |
| D9 | DC / RS | High means pixel data, low means a command. |
| D8 | RESET | |
| D11 | SDI / MOSI | Hardware SPI. This pin is fixed. |
| D13 | SCK | Hardware SPI. This pin is fixed. |
| (open) | SDO / MISO | The game only writes. Leave this pin empty. |

Leave `T_CLK`, `T_DIN`, `T_DO`, and `T_IRQ` empty. If the header has `T_CS`, tie that pin to 3V3 so the touch chip stays idle.

The glass hangs off the big breadboard. Seat the header all the way, and keep the SCK and MOSI jumpers short.

## 5 V pins into a 3.3 V panel

The TFT's logic pins are 3.3 V. A Nano pin drives about 5 V. Do this for `CS`, `RESET`, `DC`, `MOSI`, and `SCK`:

```
Nano pin ----[ 470 ohm ]----+---- TFT pin
                            |
                         [ 1k ohm ]
                            |
                           GND
```

Five 470 ohm resistors and five 1k ohm resistors cover every signal. `LED` and `VCC` stay on the power pins above. `MISO` stays empty.

Some of these red boards have a solder jumper marked `J1` on the back. Bridging `J1` and powering `VCC` from 5 V lets the signal wires go straight across. The module maker says that path runs warm and ages the panel. Use it when the resistors are still in a drawer, and go back to the dividers for a board you want to keep.

## Buttons

Each mini breadboard holds two tactile buttons for one player. Seat each button so it straddles the center gap. The two legs on one side of the gap are already connected inside the switch, and the same is true of the other side. One side goes to the Nano pin. The other side goes to GND.

The sketch enables the Nano's internal pull-ups. A press reads low. The buttons themselves need no resistors.

| Player | Action | Nano | Where it sits |
| --- | --- | --- | --- |
| Left (cyan) | Up | D2 | Left mini breadboard, farther from you |
| Left (cyan) | Down | D3 | Left mini breadboard, closer to you |
| Right (amber) | Up | D4 | Right mini breadboard, farther from you |
| Right (amber) | Down | D5 | Right mini breadboard, closer to you |

Run one ground wire from each mini breadboard back to the GND rail on the big breadboard.

## Optional click

A piezo from D6 to GND clicks on paddle hits, walls, and points. In `config.h`, set `BUZZER_ENABLED` to `1`.

## Upload

1. Board: Arduino Nano.
2. Processor: ATmega328P (Old Bootloader). This is the one CH340 clones use.
3. If `avrdude` reports `stk500_getsync`, switch Processor to ATmega328P and upload again.
4. Use the normal Upload button. A data USB cable has to be in the Nano. Charge-only cables never show a port.
5. Open the serial monitor at 115200. It should print `Pong TFT`, then `Ready`.
6. Press each button. The monitor prints `button: left up` and the matching line for the other three.
