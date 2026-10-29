#pragma once

#include <stdint.h>

#include "config.h"

enum class Mode : uint8_t { Title, Play, PointPause, GameOver };

enum class GameEvent : uint8_t { None, Serve, Wall, Paddle, Score, Win };

struct Buttons {
  bool p1Up;
  bool p1Down;
  bool p2Up;
  bool p2Down;
};

struct Game {
  Mode mode;
  int32_t ballX;  // pixel position times FP_ONE, top-left of the ball
  int32_t ballY;
  int16_t vx;  // pixels per second
  int16_t vy;
  int16_t p1Y;
  int16_t p2Y;
  int16_t p1Sub;  // leftover paddle motion, in thousandths of a pixel
  int16_t p2Sub;
  uint8_t scoreL;
  uint8_t scoreR;
  uint8_t winner;  // 1 left, 2 right, 0 while the match is open
  int8_t serveDir;  // -1 travels toward the left paddle
  uint32_t pauseStart;
};

void gameInit(Game& game);

// pressEdge is true for the frame a button goes down. dtMs is real elapsed
// time; values above MAX_FRAME_MS are clamped so a stall cannot tunnel.
GameEvent gameUpdate(Game& game, const Buttons& buttons, bool pressEdge,
                     uint32_t nowMs, uint16_t dtMs);
