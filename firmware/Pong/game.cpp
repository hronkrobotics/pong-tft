#include "game.h"

static_assert(ROTATION == 1 || ROTATION == 3, "use a landscape rotation");
static_assert(POINTS_TO_WIN >= 1 && POINTS_TO_WIN <= 99, "scores are two digits");
static_assert(FIELD_TOP + PADDLE_H < FIELD_BOTTOM, "paddle fits the field");
static_assert(BALL_SIZE < PADDLE_H, "ball is smaller than the paddle");
static_assert(FIELD_BOTTOM < SCREEN_H, "bottom border sits on the panel");
static_assert(PADDLE_INSET + PADDLE_W < SCREEN_W / 2 - 4, "paddles stay off the net");
static_assert(SERVE_SPEED <= MAX_VX && MIN_VX <= MAX_VX, "speed limits");
static_assert(SCREEN_W == 480 && SCREEN_H == 320, "3.5 inch landscape panel");

static int16_t iabs16(int16_t value) {
  return (value < 0) ? (int16_t)(-value) : value;
}

static void movePaddle(int16_t& y, int16_t& sub, bool up, bool down, uint16_t dtMs) {
  int8_t dir = 0;
  if (up && !down) {
    dir = -1;
  } else if (down && !up) {
    dir = 1;
  }

  // Thousandths of a pixel, so a short frame still adds up to real motion.
  int32_t acc = (int32_t)sub + (int32_t)dir * PADDLE_SPEED * (int32_t)dtMs;
  int32_t pixels = acc / 1000;
  int32_t remain = acc % 1000;
  int32_t next = (int32_t)y + pixels;
  const int16_t maxY = (int16_t)(FIELD_BOTTOM - PADDLE_H);
  if (next < FIELD_TOP) {
    next = FIELD_TOP;
    remain = 0;
  } else if (next > maxY) {
    next = maxY;
    remain = 0;
  }
  y = (int16_t)next;
  sub = (int16_t)remain;
}

static void placeBall(Game& game) {
  game.ballX = (int32_t)((SCREEN_W - BALL_SIZE) / 2) * FP_ONE;
  game.ballY = (int32_t)((FIELD_TOP + FIELD_BOTTOM - BALL_SIZE) / 2) * FP_ONE;
  game.vx = 0;
  game.vy = 0;
}

static void launch(Game& game, uint32_t nowMs) {
  placeBall(game);
  game.vx = (game.serveDir < 0) ? (int16_t)(-SERVE_SPEED) : (int16_t)SERVE_SPEED;
  int16_t pattern = (int16_t)((nowMs / 3u) % 7u) - 3;  // -3..3
  int16_t vy = (int16_t)(pattern * 22);
  if (vy == 0) {
    vy = 40;
  }
  game.vy = vy;
}

static void beginMatch(Game& game, uint32_t nowMs) {
  game.scoreL = 0;
  game.scoreR = 0;
  game.winner = 0;
  game.serveDir = -1;
  launch(game, nowMs);
  game.mode = Mode::Play;
}

static bool yHits(const Game& game, int16_t paddleY) {
  int32_t ballTop = game.ballY;
  int32_t ballBottom = game.ballY + (int32_t)BALL_SIZE * FP_ONE;
  int32_t padTop = (int32_t)paddleY * FP_ONE;
  int32_t padBottom = (int32_t)(paddleY + PADDLE_H) * FP_ONE;
  return ballBottom >= padTop && ballTop <= padBottom;
}

static void bounce(Game& game, int16_t paddleY, int8_t dir) {
  int16_t speed = (int16_t)(iabs16(game.vx) + SPEED_BUMP);
  if (speed < MIN_VX) {
    speed = MIN_VX;
  }
  if (speed > MAX_VX) {
    speed = MAX_VX;
  }
  game.vx = (dir > 0) ? speed : (int16_t)(-speed);

  int16_t ballCenter = (int16_t)(game.ballY / FP_ONE) + (int16_t)(BALL_SIZE / 2);
  int16_t padCenter = (int16_t)(paddleY + PADDLE_H / 2);
  int32_t vy = (int32_t)game.vy + (int32_t)(ballCenter - padCenter) * ENGLISH;
  if (vy > MAX_VY) {
    vy = MAX_VY;
  }
  if (vy < -MAX_VY) {
    vy = -MAX_VY;
  }
  game.vy = (int16_t)vy;
}

static void scorePoint(Game& game, uint8_t scorer, uint32_t nowMs) {
  if (scorer == 1) {
    if (game.scoreL < 99) {
      game.scoreL++;
    }
    game.serveDir = 1;
  } else {
    if (game.scoreR < 99) {
      game.scoreR++;
    }
    game.serveDir = -1;
  }
  placeBall(game);
  if (game.scoreL >= POINTS_TO_WIN || game.scoreR >= POINTS_TO_WIN) {
    game.mode = Mode::GameOver;
    game.winner = (game.scoreL >= POINTS_TO_WIN) ? 1 : 2;
  } else {
    game.mode = Mode::PointPause;
    game.pauseStart = nowMs;
  }
}

void gameInit(Game& game) {
  game.mode = Mode::Title;
  game.scoreL = 0;
  game.scoreR = 0;
  game.winner = 0;
  game.serveDir = -1;
  game.pauseStart = 0;
  game.p1Y = (int16_t)((FIELD_TOP + FIELD_BOTTOM - PADDLE_H) / 2);
  game.p2Y = game.p1Y;
  game.p1Sub = 0;
  game.p2Sub = 0;
  placeBall(game);
}

GameEvent gameUpdate(Game& game, const Buttons& buttons, bool pressEdge,
                     uint32_t nowMs, uint16_t dtMs) {
  if (dtMs > MAX_FRAME_MS) {
    dtMs = MAX_FRAME_MS;
  }

  movePaddle(game.p1Y, game.p1Sub, buttons.p1Up, buttons.p1Down, dtMs);
  movePaddle(game.p2Y, game.p2Sub, buttons.p2Up, buttons.p2Down, dtMs);

  if (game.mode == Mode::Title || game.mode == Mode::GameOver) {
    if (pressEdge) {
      beginMatch(game, nowMs);
      return GameEvent::Serve;
    }
    return GameEvent::None;
  }

  if (game.mode == Mode::PointPause) {
    if ((uint32_t)(nowMs - game.pauseStart) >= POINT_PAUSE_MS) {
      launch(game, nowMs);
      game.mode = Mode::Play;
      return GameEvent::Serve;
    }
    return GameEvent::None;
  }

  int32_t deltaX = ((int32_t)game.vx * (int32_t)dtMs * FP_ONE) / 1000;
  int32_t deltaY = ((int32_t)game.vy * (int32_t)dtMs * FP_ONE) / 1000;
  int32_t oldX = game.ballX;
  game.ballX += deltaX;
  game.ballY += deltaY;

  GameEvent event = GameEvent::None;
  const int32_t minY = (int32_t)FIELD_TOP * FP_ONE;
  const int32_t maxY = (int32_t)(FIELD_BOTTOM - BALL_SIZE) * FP_ONE;
  if (game.ballY < minY) {
    game.ballY = minY;
    if (game.vy < 0) {
      game.vy = (int16_t)(-game.vy);
    }
    event = GameEvent::Wall;
  } else if (game.ballY > maxY) {
    game.ballY = maxY;
    if (game.vy > 0) {
      game.vy = (int16_t)(-game.vy);
    }
    event = GameEvent::Wall;
  }

  const int32_t leftFace = (int32_t)(PADDLE_INSET + PADDLE_W) * FP_ONE;
  const int32_t leftPad = (int32_t)PADDLE_INSET * FP_ONE;
  int32_t ballRight = game.ballX + (int32_t)BALL_SIZE * FP_ONE;
  bool crossedLeft = deltaX < 0 && oldX >= leftFace && game.ballX <= leftFace;
  bool insideLeft = ballRight > leftPad && game.ballX < leftFace;
  if ((crossedLeft || insideLeft) && yHits(game, game.p1Y) && game.vx < 0) {
    game.ballX = leftFace;
    bounce(game, game.p1Y, 1);
    event = GameEvent::Paddle;
  }

  const int32_t rightPad = (int32_t)(SCREEN_W - PADDLE_INSET - PADDLE_W) * FP_ONE;
  const int32_t rightLimit = rightPad - (int32_t)BALL_SIZE * FP_ONE;
  const int32_t rightOuter = (int32_t)(SCREEN_W - PADDLE_INSET) * FP_ONE;
  ballRight = game.ballX + (int32_t)BALL_SIZE * FP_ONE;
  bool crossedRight = deltaX > 0 && oldX <= rightLimit && game.ballX >= rightLimit;
  bool insideRight = ballRight > rightPad && game.ballX < rightOuter;
  if ((crossedRight || insideRight) && yHits(game, game.p2Y) && game.vx > 0) {
    game.ballX = rightLimit;
    bounce(game, game.p2Y, -1);
    event = GameEvent::Paddle;
  }

  if (game.ballX < 0) {
    scorePoint(game, 2, nowMs);
    return (game.mode == Mode::GameOver) ? GameEvent::Win : GameEvent::Score;
  }
  if (game.ballX > (int32_t)(SCREEN_W - BALL_SIZE) * FP_ONE) {
    scorePoint(game, 1, nowMs);
    return (game.mode == Mode::GameOver) ? GameEvent::Win : GameEvent::Score;
  }
  return event;
}
