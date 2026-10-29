# How the game works

This page follows one button press from the mini breadboard to pixels on the glass. Pins and the resistor dividers are on the [bench card](WIRING.md). Which file to edit is in [CONTRIBUTING.md](../CONTRIBUTING.md).

## The bench

```mermaid
flowchart LR
    subgraph LeftBB["Left mini breadboard, cyan"]
        L_UP(["Up button"])
        L_DN(["Down button"])
    end
    subgraph RightBB["Right mini breadboard, amber"]
        R_UP(["Up button"])
        R_DN(["Down button"])
    end

    subgraph Nano["Arduino Nano, ATmega328P"]
        GPIO["D2 D3 D4 D5<br/>INPUT_PULLUP"]
        SPI["Hardware SPI<br/>D13 SCK, D11 MOSI"]
        CTRL["D10 CS, D9 DC, D8 RESET"]
        BUZ["D6, optional"]
        USB["USB serial<br/>115200 baud"]
        P5["5V"]
        P33["3V3"]
    end

    DIV["470 ohm series +<br/>1k to GND, x5<br/>5 V to about 3.4 V"]

    subgraph TFT["3.5 in SPI TFT, 480 x 320"]
        TLOGIC["CS DC RESET<br/>SDI SCK"]
        TVCC["VCC"]
        TLED["LED backlight"]
        TOUCH["T_CS tied to 3V3,<br/>other touch pins open"]
        MISO["SDO / MISO open"]
    end

    L_UP -->|"D2, press = LOW"| GPIO
    L_DN -->|"D3"| GPIO
    R_UP -->|"D4"| GPIO
    R_DN -->|"D5"| GPIO
    SPI --> DIV
    CTRL --> DIV
    DIV --> TLOGIC
    P5 --> TVCC
    P33 --> TLED
    BUZ -.->|"tone()"| PIEZO(["Piezo to GND"])
    USB -.->|"pin list, button lines,<br/>score lines"| PC["Serial monitor"]
```

All grounds are shared: the Nano, the TFT, and both mini breadboards.

## Firmware files

`game.cpp` holds every rule and touches no hardware. `make test` compiles it on a laptop with `test/test_game.cpp`.

```mermaid
flowchart TB
    CFG["config.h<br/>pins, driver choice, field size,<br/>speeds, colors, timings"]

    subgraph Host["Builds on a laptop too"]
        GAME["game.h / game.cpp<br/>Game, Mode, GameEvent<br/>gameInit, gameUpdate"]
        TEST["test/test_game.cpp"]
    end

    subgraph Device["Needs the Nano"]
        INO["Pong.ino<br/>setup, loop"]
        INPUT["input.cpp<br/>debounce, press edges"]
        FEED["feedback.cpp<br/>serial score, beeps"]
        RENDER["render.cpp<br/>dirty-rectangle drawing"]
        DISPLAY["display.cpp<br/>ILI9488 / ILI9486 over SPI"]
        FONT["font.cpp<br/>5 x 7 glyphs in PROGMEM"]
    end

    TOOL["tools/make_font.py"] -->|"writes"| FONT
    CFG --> GAME
    CFG --> INPUT
    CFG --> FEED
    CFG --> RENDER
    CFG --> DISPLAY
    TEST --> GAME
    INO --> INPUT
    INO --> GAME
    INO --> FEED
    INO --> RENDER
    INO --> DISPLAY
    RENDER --> DISPLAY
    DISPLAY --> FONT
    INPUT -->|"Buttons"| GAME
    RENDER -->|"reads Game"| GAME
    FEED -->|"reads Game"| GAME
```

## One pass of loop()

`loop()` has no frame timer. Each pass measures the real time since the last one, and `gameUpdate()` caps it at `MAX_FRAME_MS` (40 ms), so a slow redraw can't throw the ball through a paddle.

```mermaid
sequenceDiagram
    autonumber
    participant Sketch as Pong.ino
    participant Input as input.cpp
    participant Game as game.cpp
    participant Feed as feedback.cpp
    participant Render as render.cpp
    participant Panel as display.cpp and TFT

    Note over Sketch: setup()
    Sketch->>Input: inputBegin(), pull-ups on D2-D5
    Sketch->>Panel: displayBegin(), reset, init, MADCTL, display on
    Sketch->>Game: gameInit(), Mode Title
    Sketch->>Render: renderBegin(), clear, borders, net, title text

    Note over Sketch: loop()
    Sketch->>Sketch: dt = millis() - lastMs
    Sketch->>Input: inputRead()
    Input-->>Sketch: Buttons, held state after 20 ms debounce
    Sketch->>Input: inputConsumePress()
    Input-->>Sketch: true once per new press
    Sketch->>Game: gameUpdate(game, buttons, edge, now, dt)
    Game-->>Sketch: GameEvent
    Sketch->>Feed: feedback(game, event)
    opt Score or Win
        Feed->>Feed: Serial "left N  right M"
    end
    opt BUZZER_ENABLED
        Feed->>Feed: tone() for the event
    end
    Sketch->>Render: render(game)
    alt mode changed
        Render->>Panel: erase old labels, repair net, draw new labels
    end
    Render->>Panel: changed score digits only
    Render->>Panel: erase old ball, repair net under it
    Render->>Panel: slide or repaint each paddle
    Render->>Panel: paint the ball at its new spot
```

## Game modes

```mermaid
stateDiagram-v2
    direction LR
    [*] --> Title: gameInit()
    Title --> Play: any press<br/>scores 0-0, serve left
    Play --> Play: Wall or Paddle event
    Play --> PointPause: ball leaves the field<br/>Score event
    PointPause --> Play: 700 ms later<br/>serve toward the player who missed
    Play --> GameOver: a side reaches 11<br/>Win event
    GameOver --> Play: any press after 1000 ms<br/>fresh match, serve left

    Title: Title, PONG and PRESS ANY BUTTON
    PointPause: PointPause, ball waits at center
    GameOver: GameOver, LEFT WINS or RIGHT WINS
```

Paddles move in every mode, so players can line up during the title screen and the pauses.

## Inside gameUpdate()

All positions are fixed point: `ballX` and `ballY` are pixels times `FP_ONE` (256). Speeds are pixels per second.

```mermaid
flowchart TD
    START(["gameUpdate(game, buttons, edge, now, dt)"]) --> CLAMP["dt = min(dt, 40 ms)"]
    CLAMP --> PAD["Move both paddles<br/>210 px/s, remainder kept in thousandths,<br/>both buttons held = stand still,<br/>clamped to the field"]
    PAD --> HOLD{"GameOver and<br/>under 1000 ms?"}
    HOLD -->|"yes"| DROP["Ignore the press"]
    HOLD -->|"no"| MODE
    DROP --> MODE{"Mode?"}
    MODE -->|"Title or GameOver"| WAIT{"Press?"}
    WAIT -->|"yes"| BEGIN["beginMatch(): 0-0,<br/>serve left"] --> SERVE(["return Serve"])
    WAIT -->|"no"| NONE(["return None"])
    MODE -->|"PointPause"| PAUSE{"700 ms since<br/>the point?"}
    PAUSE -->|"yes"| LAUNCH["launch(): ball to center,<br/>vx = +/-170, vy from now / 3 mod 7"] --> SERVE
    PAUSE -->|"no"| NONE
    MODE -->|"Play"| MOVE["ball += v * dt"]
    MOVE --> WALL{"Past the top<br/>or bottom?"}
    WALL -->|"yes"| REFLECT["Clamp to the edge,<br/>flip vy, event = Wall"] --> LEFT
    WALL -->|"no"| LEFT{"Moving left, at or inside the<br/>left paddle, and rows overlap?"}
    LEFT -->|"yes"| BOUNCE_L["Snap to the paddle face,<br/>bounce(), event = Paddle"] --> RIGHT
    LEFT -->|"no"| RIGHT{"Moving right, at or inside the<br/>right paddle, and rows overlap?"}
    RIGHT -->|"yes"| BOUNCE_R["Snap to the paddle face,<br/>bounce(), event = Paddle"] --> OUT
    RIGHT -->|"no"| OUT{"ballX < 0 or<br/>> 464?"}
    OUT -->|"no"| EVENT(["return the event"])
    OUT -->|"yes"| SCORE["scorePoint()<br/>the other side +1,<br/>serve toward the side that missed"]
    SCORE --> WIN{"A side has 11?"}
    WIN -->|"yes"| OVER["Mode GameOver,<br/>hold starts"] --> WINEV(["return Win"])
    WIN -->|"no"| PP["Mode PointPause"] --> SCOREEV(["return Score"])
```

`bounce()` adds `SPEED_BUMP` (16) to the horizontal speed, keeps it between 160 and 320 px/s, and adds `ENGLISH` (5) px/s of vertical speed for every pixel the ball's center sits away from the paddle's center, capped at 280 px/s.

## Drawing without a frame buffer

A full 480 x 320 picture at 3 bytes a pixel is 460,800 bytes. The Nano has 2,048 bytes of RAM. `render.cpp` remembers where the ball and paddles were and repaints only what changed.

```mermaid
flowchart TD
    R(["render(game)"]) --> MC{"Mode changed?"}
    MC -->|"yes"| LBL["Erase the old labels, repair the net,<br/>draw the new mode's text"]
    MC -->|"no"| SC
    LBL --> SC{"Score changed?"}
    SC -->|"yes"| DIG["Redraw that side's two digits,<br/>scale 3, with background"]
    SC -->|"no"| BALL
    DIG --> BALL{"Ball moved or hidden?"}
    BALL -->|"yes"| ERASE["Fill old ball rect with the background,<br/>repair net dashes under it,<br/>note if it overlapped a paddle"]
    BALL -->|"no"| PADS
    ERASE --> PADS{"Paddle moved or<br/>punched by the eraser?"}
    PADS -->|"moved less than its height"| SLIDE["Slide: clear the strip it left,<br/>paint the strip it entered"]
    PADS -->|"punched or jumped"| FULL["Erase the old rect, repair the net,<br/>paint the whole paddle"]
    PADS -->|"no"| PAINT
    SLIDE --> PAINT["Paint the ball at its new spot"]
    FULL --> PAINT
```

Every fill goes through `displayFillRect()`: set the column and row window (`0x2A`, `0x2B`), send `0x2C`, then stream 3 bytes per pixel. Typical SPI traffic at 4 MHz (2 µs per byte, before loop overhead):

| Drawing | Bytes | Time on the bus |
| --- | ---: | ---: |
| Ball erase plus repaint, 16 x 16 twice | 1,536 | about 3 ms |
| Paddle slide by 4 px, two 14 x 4 strips | 336 | under 1 ms |
| One side's score, two digits at scale 3 | 2,079 | about 4 ms |
| Whole screen at boot | 460,800 | about 0.9 s |

## Buttons

```mermaid
stateDiagram-v2
    direction LR
    [*] --> Stable: inputBegin() reads the pins
    Stable --> Settling: raw reading changes
    Settling --> Settling: changes again,<br/>timer restarts
    Settling --> Stable: same for 20 ms<br/>update held mask
    Stable --> Stable: no change
    note right of Stable
        When the held mask gains a button,
        pendingEdge is set. inputConsumePress()
        returns it once and clears it.
    end note
```

With `DEBUG_BUTTONS` on, each new press also prints `button: left up` and the like, which is how the README's wiring check works.
