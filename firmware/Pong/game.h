#pragma once

#include <stdint.h>

#include "config.h"

enum class Mode : uint8_t { Title, Play, PointPause, Paused };

enum class GameEvent : uint8_t {
  None,
  Serve,
  Wall,
  Paddle,
  Score,
  Win,
  Pause,
  Resume
};

struct Buttons {
  bool botLeft;
  bool botRight;
  bool topLeft;
  bool topRight;
};

struct Game {
  Mode mode;
  int32_t ballX;  // pixel position times FP_ONE, top-left of the ball
  int32_t ballY;
  int16_t vx;  // pixels per second
  int16_t vy;
  int16_t botX;
  int16_t topX;
  int16_t botSub;  // leftover paddle motion, in thousandths of a pixel
  int16_t topSub;
  uint8_t scoreBot;
  uint8_t scoreTop;
  uint8_t winner;  // 1 bottom, 2 top, 0 while the match is open
  int8_t serveDir;  // -1 travels up, +1 travels down
  uint32_t pauseStart;  // point pause, or the moment the winner screen appeared
  Mode pausedFrom;
  uint32_t holdStart;
};

void gameInit(Game& game);

// tapEdge is true for the frame a finger lands on the glass. A held finger
// does not set it again. dtMs is real elapsed time; values above MAX_FRAME_MS
// are clamped so a stall cannot tunnel the ball through a paddle.
GameEvent gameUpdate(Game& game, const Buttons& buttons, bool tapEdge,
                     uint32_t nowMs, uint16_t dtMs);
