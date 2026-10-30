#include "game.h"

static_assert(ROTATION == 0 || ROTATION == 2, "use a portrait rotation");
static_assert(SCREEN_W == 320 && SCREEN_H == 480, "3.5 inch portrait panel");
static_assert(POINTS_TO_WIN >= 1 && POINTS_TO_WIN <= 99, "scores are two digits");
static_assert(PADDLE_LENGTH > BALL_SIZE && PADDLE_LENGTH < SCREEN_W, "paddle fits");
static_assert(TOP_PADDLE_Y + PADDLE_THICK + BALL_SIZE < BOTTOM_PADDLE_Y,
              "gap between the paddles");
static_assert(SERVE_SPEED > 0 && MAX_VX > 0, "speeds");
static_assert(BALL_REST_Y > TOP_PADDLE_Y + PADDLE_THICK, "resting ball clears the top");
static_assert(BALL_REST_Y + BALL_SIZE < BOTTOM_PADDLE_Y, "resting ball clears the bottom");

static const int16_t kMaxX = (int16_t)(SCREEN_W - PADDLE_LENGTH);
static const int32_t kTopFace = (int32_t)(TOP_PADDLE_Y + PADDLE_THICK) * FP_ONE;
static const int32_t kBotFace = (int32_t)BOTTOM_PADDLE_Y * FP_ONE;
static const int32_t kBotBack = (int32_t)(BOTTOM_PADDLE_Y + PADDLE_THICK) * FP_ONE;
static const int32_t kBall = (int32_t)BALL_SIZE * FP_ONE;
static const int32_t kLimitX = (int32_t)(SCREEN_W - BALL_SIZE) * FP_ONE;
static const int32_t kTopOut = (int32_t)TOP_PADDLE_Y * FP_ONE;
static const int32_t kBotOut =
    (int32_t)(BOTTOM_PADDLE_Y + PADDLE_THICK - BALL_SIZE) * FP_ONE;

static void movePaddle(int16_t& x, int16_t& sub, bool left, bool right, uint16_t dtMs) {
  int8_t dir = 0;
  if (left && !right) {
    dir = -1;
  } else if (right && !left) {
    dir = 1;
  }

  // Thousandths of a pixel, so a short frame still adds up to real motion.
  int32_t acc = (int32_t)sub + (int32_t)dir * PADDLE_SPEED * (int32_t)dtMs;
  int32_t pixels = acc / 1000;
  int32_t remain = acc % 1000;
  int32_t next = (int32_t)x + pixels;
  if (next < 0) {
    next = 0;
    remain = 0;
  } else if (next > kMaxX) {
    next = kMaxX;
    remain = 0;
  }
  x = (int16_t)next;
  sub = (int16_t)remain;
}

static void placeBall(Game& game) {
  game.ballX = (int32_t)BALL_REST_X * FP_ONE;
  game.ballY = (int32_t)BALL_REST_Y * FP_ONE;
  game.vx = 0;
  game.vy = 0;
}

static void launch(Game& game, uint32_t nowMs) {
  placeBall(game);
  // nowMs picks the direction so a match is not always served the same way,
  // and so a test can force either sign. 0 travels up, 17 travels down.
  game.serveDir = ((nowMs / 17u) & 1u) ? (int8_t)1 : (int8_t)-1;
  game.vy = (int16_t)(game.serveDir * SERVE_SPEED);
  int16_t pattern = (int16_t)((nowMs / 5u) % 5u) - 2;  // -2..2
  int16_t vx = (int16_t)(pattern * 30);
  if (vx == 0) {
    vx = ((nowMs / 17u) & 2u) ? (int16_t)30 : (int16_t)-30;
  }
  game.vx = vx;
}

static void beginMatch(Game& game, uint32_t nowMs) {
  game.scoreBot = 0;
  game.scoreTop = 0;
  game.winner = 0;
  launch(game, nowMs);
  game.mode = Mode::Play;
}

static bool xHits(const Game& game, int16_t paddleX) {
  int32_t ballLeft = game.ballX;
  int32_t ballRight = game.ballX + kBall;
  int32_t padLeft = (int32_t)paddleX * FP_ONE;
  int32_t padRight = (int32_t)(paddleX + PADDLE_LENGTH) * FP_ONE;
  return ballRight >= padLeft && ballLeft <= padRight;
}

static int16_t aim(const Game& game, int16_t paddleX) {
  int16_t ballCenter = (int16_t)(game.ballX / FP_ONE) + (int16_t)(BALL_SIZE / 2);
  int16_t padCenter = (int16_t)(paddleX + PADDLE_LENGTH / 2);
  int32_t vx = (int32_t)(ballCenter - padCenter) * ENGLISH;
  if (vx > MAX_VX) {
    vx = MAX_VX;
  }
  if (vx < -MAX_VX) {
    vx = -MAX_VX;
  }
  return (int16_t)vx;
}

static GameEvent miss(Game& game, uint8_t scorer, uint32_t nowMs) {
  if (scorer == 1) {
    if (game.scoreBot < 99) {
      game.scoreBot++;
    }
  } else if (game.scoreTop < 99) {
    game.scoreTop++;
  }
  bool won = game.scoreBot >= POINTS_TO_WIN || game.scoreTop >= POINTS_TO_WIN;
  placeBall(game);
  if (won) {
    game.mode = Mode::Title;
    game.winner = (game.scoreBot >= POINTS_TO_WIN) ? (uint8_t)1 : (uint8_t)2;
    game.pauseStart = nowMs;
    return GameEvent::Win;
  }
  game.mode = Mode::PointPause;
  game.pauseStart = nowMs;
  return GameEvent::Score;
}

void gameInit(Game& game) {
  game.mode = Mode::Title;
  game.scoreBot = 0;
  game.scoreTop = 0;
  game.winner = 0;
  game.serveDir = -1;
  game.pauseStart = 0;
  game.pausedFrom = Mode::Title;
  game.holdStart = 0;
  game.botX = (int16_t)((SCREEN_W - PADDLE_LENGTH) / 2);
  game.topX = game.botX;
  game.botSub = 0;
  game.topSub = 0;
  placeBall(game);
}

GameEvent gameUpdate(Game& game, const Buttons& buttons, bool tapEdge,
                     uint32_t nowMs, uint16_t dtMs) {
  if (dtMs > MAX_FRAME_MS) {
    dtMs = MAX_FRAME_MS;
  }

  // A tap starts a match, pauses play, or resumes. The finger's frame does
  // not also move the ball, so a pause lands on the picture the player saw.
  if (tapEdge && game.mode == Mode::Title) {
    bool winnerHolding = game.winner != 0 &&
                         (uint32_t)(nowMs - game.pauseStart) < GAME_OVER_HOLD_MS;
    if (!winnerHolding) {
      beginMatch(game, nowMs);
      return GameEvent::Serve;
    }
  }
  if (tapEdge && (game.mode == Mode::Play || game.mode == Mode::PointPause)) {
    game.pausedFrom = game.mode;
    game.holdStart = nowMs;
    game.mode = Mode::Paused;
    return GameEvent::Pause;
  }
  if (tapEdge && game.mode == Mode::Paused) {
    if (game.pausedFrom == Mode::PointPause) {
      game.pauseStart += (uint32_t)(nowMs - game.holdStart);
    }
    game.mode = game.pausedFrom;
    return GameEvent::Resume;
  }

  if (game.mode == Mode::Play || game.mode == Mode::PointPause) {
    movePaddle(game.botX, game.botSub, buttons.botLeft, buttons.botRight, dtMs);
    movePaddle(game.topX, game.topSub, buttons.topLeft, buttons.topRight, dtMs);
  }

  if (game.mode == Mode::PointPause) {
    if ((uint32_t)(nowMs - game.pauseStart) >= POINT_PAUSE_MS) {
      launch(game, nowMs);
      game.mode = Mode::Play;
      return GameEvent::Serve;
    }
    return GameEvent::None;
  }
  if (game.mode != Mode::Play) {
    return GameEvent::None;
  }

  int32_t deltaX = ((int32_t)game.vx * (int32_t)dtMs * FP_ONE) / 1000;
  int32_t deltaY = ((int32_t)game.vy * (int32_t)dtMs * FP_ONE) / 1000;
  int32_t oldY = game.ballY;
  game.ballX += deltaX;
  game.ballY += deltaY;

  GameEvent event = GameEvent::None;
  if (game.ballX < 0) {
    game.ballX = 0;
    if (game.vx < 0) {
      game.vx = (int16_t)(-game.vx);
    }
    event = GameEvent::Wall;
  } else if (game.ballX > kLimitX) {
    game.ballX = kLimitX;
    if (game.vx > 0) {
      game.vx = (int16_t)(-game.vx);
    }
    event = GameEvent::Wall;
  }

  bool crossedTop = deltaY < 0 && oldY >= kTopFace && game.ballY <= kTopFace;
  bool insideTop = game.ballY < kTopFace && game.ballY + kBall > kTopOut;
  if ((crossedTop || insideTop) && xHits(game, game.topX) && game.vy < 0) {
    game.ballY = kTopFace;
    game.vy = SERVE_SPEED;
    game.vx = aim(game, game.topX);
    event = GameEvent::Paddle;
  }

  int32_t ballBottom = game.ballY + kBall;
  int32_t oldBottom = oldY + kBall;
  bool crossedBot = deltaY > 0 && oldBottom <= kBotFace && ballBottom >= kBotFace;
  bool insideBot = ballBottom > kBotFace && game.ballY < kBotBack;
  if ((crossedBot || insideBot) && xHits(game, game.botX) && game.vy > 0) {
    game.ballY = kBotFace - kBall;
    game.vy = (int16_t)(-SERVE_SPEED);
    game.vx = aim(game, game.botX);
    event = GameEvent::Paddle;
  }

  if (game.ballY < kTopOut) {
    return miss(game, 1, nowMs);
  }
  if (game.ballY > kBotOut) {
    return miss(game, 2, nowMs);
  }
  return event;
}
