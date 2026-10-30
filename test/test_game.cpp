#include "game.h"

#include <cstdio>

static int failures = 0;

#define CHECK(cond)                                                          \
  do {                                                                       \
    if (!(cond)) {                                                           \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);           \
      failures++;                                                            \
    }                                                                        \
  } while (0)

static void title(const char* name) { std::printf("  %s\n", name); }

static Buttons none() {
  Buttons buttons = {false, false, false, false};
  return buttons;
}

static int16_t iabs16(int16_t value) {
  return (value < 0) ? (int16_t)(-value) : value;
}

static void testTitleAndServeDirection() {
  title("title ignores buttons and a tap serves up or down");
  Game game;
  gameInit(game);
  CHECK(game.mode == Mode::Title);
  CHECK(game.vx == 0 && game.vy == 0);
  CHECK(game.scoreBot == 0 && game.scoreTop == 0);
  CHECK(game.ballX == (int32_t)BALL_REST_X * FP_ONE);
  CHECK(game.ballY == (int32_t)BALL_REST_Y * FP_ONE);

  int16_t x = game.botX;
  Buttons held = none();
  held.botLeft = true;
  held.topRight = true;
  GameEvent event = gameUpdate(game, held, false, 10000, 40);
  CHECK(event == GameEvent::None);
  CHECK(game.mode == Mode::Title);
  CHECK(game.botX == x);
  CHECK(game.topX == x);

  event = gameUpdate(game, none(), true, 0, 20);
  CHECK(event == GameEvent::Serve);
  CHECK(game.mode == Mode::Play);
  CHECK(game.vy == (int16_t)(-SERVE_SPEED));
  CHECK(game.vx == (int16_t)(-60));
  CHECK(game.scoreBot == 0 && game.scoreTop == 0);

  gameInit(game);
  event = gameUpdate(game, none(), true, 17, 20);
  CHECK(event == GameEvent::Serve);
  CHECK(game.vy == SERVE_SPEED);
  CHECK(game.vx == 30);
}

static void testWalls() {
  title("ball bounces off the left and right edges");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.ballX = (int32_t)3 * FP_ONE;
  game.ballY = (int32_t)BALL_REST_Y * FP_ONE;
  game.vx = (int16_t)(-MAX_VX);
  game.vy = SERVE_SPEED;

  GameEvent event = gameUpdate(game, none(), false, 2000, 40);
  CHECK(event == GameEvent::Wall);
  CHECK(game.ballX == 0);
  CHECK(game.vx == MAX_VX);
  CHECK(game.vy == SERVE_SPEED);

  const int32_t limit = (int32_t)(SCREEN_W - BALL_SIZE) * FP_ONE;
  game.ballX = limit - (int32_t)3 * FP_ONE;
  game.vx = MAX_VX;
  game.vy = (int16_t)(-SERVE_SPEED);
  event = gameUpdate(game, none(), false, 2040, 40);
  CHECK(event == GameEvent::Wall);
  CHECK(game.ballX == limit);
  CHECK(game.vx == (int16_t)(-MAX_VX));
  CHECK(game.vy == (int16_t)(-SERVE_SPEED));
  CHECK(game.mode == Mode::Play);
}

static void testTopPaddle() {
  title("top paddle returns the ball and aims from the hit");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.topX = 80;
  game.ballX = (int32_t)100 * FP_ONE;
  game.ballY = (int32_t)30 * FP_ONE;
  game.vx = 0;
  game.vy = (int16_t)(-SERVE_SPEED);

  GameEvent event = gameUpdate(game, none(), false, 3000, 40);
  CHECK(event == GameEvent::Paddle);
  CHECK(game.vy == SERVE_SPEED);
  CHECK(game.vx == (int16_t)(-52));
  CHECK(game.ballY == (int32_t)(TOP_PADDLE_Y + PADDLE_THICK) * FP_ONE);
  CHECK(game.mode == Mode::Play);
  CHECK(game.scoreBot == 0 && game.scoreTop == 0);
}

static void testBottomPaddleCap() {
  title("bottom paddle returns the ball, and the angle stays capped");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.botX = 0;
  game.ballX = (int32_t)80 * FP_ONE;
  game.ballY = (int32_t)436 * FP_ONE;
  game.vx = 0;
  game.vy = SERVE_SPEED;

  GameEvent event = gameUpdate(game, none(), false, 4000, 40);
  CHECK(event == GameEvent::Paddle);
  CHECK(game.vy == (int16_t)(-SERVE_SPEED));
  CHECK(game.vx == MAX_VX);
  CHECK(game.ballY == (int32_t)(BOTTOM_PADDLE_Y - BALL_SIZE) * FP_ONE);
}

static void testNoSpeedRamp() {
  title("paddle hits keep the same speed");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.topX = 80;

  for (int i = 0; i < 30; i++) {
    game.ballX = (int32_t)100 * FP_ONE;
    game.ballY = (int32_t)20 * FP_ONE;
    game.vx = 100;
    game.vy = (int16_t)(-SERVE_SPEED);
    GameEvent event = gameUpdate(game, none(), false, (uint32_t)(5000 + i), 0);
    CHECK(event == GameEvent::Paddle);
    CHECK(game.vy == SERVE_SPEED);
    CHECK(game.vx == (int16_t)(-52));
  }

  game.topX = 120;
  game.ballX = (int32_t)153 * FP_ONE;
  game.ballY = (int32_t)20 * FP_ONE;
  game.vx = 90;
  game.vy = (int16_t)(-SERVE_SPEED);
  GameEvent event = gameUpdate(game, none(), false, 6000, 0);
  CHECK(event == GameEvent::Paddle);
  CHECK(game.vx == 0);
  CHECK(game.vy == SERVE_SPEED);
}

static void testMisses() {
  title("a miss scores for the other end, then a fresh serve");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.ballX = 0;
  game.ballY = (int32_t)20 * FP_ONE;
  game.vx = 0;
  game.vy = (int16_t)(-SERVE_SPEED);

  uint32_t now = 10000;
  GameEvent event = gameUpdate(game, none(), false, now, 40);
  CHECK(event == GameEvent::Score);
  CHECK(game.scoreBot == 1);
  CHECK(game.scoreTop == 0);
  CHECK(game.mode == Mode::PointPause);
  CHECK(game.vx == 0 && game.vy == 0);

  event = gameUpdate(game, none(), false, now + POINT_PAUSE_MS, 16);
  CHECK(event == GameEvent::Serve);
  CHECK(game.mode == Mode::Play);
  CHECK(game.vy == SERVE_SPEED);
  CHECK(game.vx == (int16_t)(-60));

  game.mode = Mode::Play;
  game.ballX = 0;
  game.ballY = (int32_t)450 * FP_ONE;
  game.vx = 0;
  game.vy = SERVE_SPEED;
  now = 20000;
  event = gameUpdate(game, none(), false, now, 40);
  CHECK(event == GameEvent::Score);
  CHECK(game.scoreTop == 1);
  CHECK(game.scoreBot == 1);
  CHECK(game.mode == Mode::PointPause);
}

static void testPause() {
  title("a tap freezes play, and the next tap resumes");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.ballX = (int32_t)100 * FP_ONE;
  game.ballY = (int32_t)BALL_REST_Y * FP_ONE;
  game.vx = 40;
  game.vy = SERVE_SPEED;
  int16_t paddle = game.botX;

  uint32_t now = 8000;
  GameEvent event = gameUpdate(game, none(), true, now, 40);
  CHECK(event == GameEvent::Pause);
  CHECK(game.mode == Mode::Paused);
  CHECK(game.ballX == (int32_t)100 * FP_ONE);
  CHECK(game.ballY == (int32_t)BALL_REST_Y * FP_ONE);
  CHECK(game.vy == SERVE_SPEED);

  Buttons push = none();
  push.botRight = true;
  event = gameUpdate(game, push, false, now + 40, 40);
  CHECK(event == GameEvent::None);
  CHECK(game.mode == Mode::Paused);
  CHECK(game.ballX == (int32_t)100 * FP_ONE);
  CHECK(game.botX == paddle);

  event = gameUpdate(game, none(), true, now + 80, 40);
  CHECK(event == GameEvent::Resume);
  CHECK(game.mode == Mode::Play);
  CHECK(game.ballY == (int32_t)BALL_REST_Y * FP_ONE);

  int32_t y = game.ballY;
  event = gameUpdate(game, none(), false, now + 120, 40);
  int32_t step = ((int32_t)SERVE_SPEED * 40 * FP_ONE) / 1000;
  CHECK(game.ballY == y + step);
  CHECK(game.mode == Mode::Play);
}

static void testPauseDuringPoint() {
  title("pausing a point keeps the rest of the wait");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.ballX = 0;
  game.ballY = (int32_t)20 * FP_ONE;
  game.vx = 0;
  game.vy = (int16_t)(-SERVE_SPEED);

  uint32_t now = 10000;
  GameEvent event = gameUpdate(game, none(), false, now, 40);
  CHECK(event == GameEvent::Score);
  CHECK(game.mode == Mode::PointPause);

  event = gameUpdate(game, none(), true, now + 100, 16);
  CHECK(event == GameEvent::Pause);
  CHECK(game.mode == Mode::Paused);

  event = gameUpdate(game, none(), false, now + POINT_PAUSE_MS, 16);
  CHECK(game.mode == Mode::Paused);

  event = gameUpdate(game, none(), true, now + 400, 16);
  CHECK(event == GameEvent::Resume);
  CHECK(game.mode == Mode::PointPause);

  event = gameUpdate(game, none(), false, now + 999, 16);
  CHECK(game.mode == Mode::PointPause);
  CHECK(game.vy == 0);

  event = gameUpdate(game, none(), false, now + 1000, 16);
  CHECK(event == GameEvent::Serve);
  CHECK(game.mode == Mode::Play);
  CHECK(iabs16(game.vy) == SERVE_SPEED);
}

static void testWinReturnsToTitle() {
  title("first to 11 returns to the start screen");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.scoreBot = (uint8_t)(POINTS_TO_WIN - 1);
  game.ballX = 0;
  game.ballY = (int32_t)20 * FP_ONE;
  game.vx = 0;
  game.vy = (int16_t)(-SERVE_SPEED);

  GameEvent event = gameUpdate(game, none(), false, 30000, 40);
  CHECK(event == GameEvent::Win);
  CHECK(game.mode == Mode::Title);
  CHECK(game.winner == 1);
  CHECK(game.scoreBot == POINTS_TO_WIN);
  CHECK(game.vx == 0 && game.vy == 0);
  CHECK(game.ballY == (int32_t)BALL_REST_Y * FP_ONE);

  event = gameUpdate(game, none(), false, 30100, 16);
  CHECK(game.mode == Mode::Title);
  CHECK(game.scoreBot == POINTS_TO_WIN);

  event = gameUpdate(game, none(), true, 30010, 16);
  CHECK(event == GameEvent::None);
  CHECK(game.mode == Mode::Title);
  CHECK(game.winner == 1);
  event = gameUpdate(game, none(), true, 30000 + GAME_OVER_HOLD_MS - 1, 16);
  CHECK(game.mode == Mode::Title);
  CHECK(game.winner == 1);

  event = gameUpdate(game, none(), true, 30000 + GAME_OVER_HOLD_MS, 16);
  CHECK(event == GameEvent::Serve);
  CHECK(game.mode == Mode::Play);
  CHECK(game.scoreBot == 0 && game.scoreTop == 0);
  CHECK(game.winner == 0);
  CHECK(game.vy == SERVE_SPEED);
  CHECK(game.vx == (int16_t)(-60));
}

static void testPaddleControl() {
  title("paddles follow one button, ignore both, and unstick at the edge");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  int16_t x = game.botX;
  Buttons left = none();
  left.botLeft = true;
  gameUpdate(game, left, false, 1000, 40);
  CHECK(game.botX == (int16_t)(x - PADDLE_SPEED * 40 / 1000));

  int16_t held = game.botX;
  Buttons both = none();
  both.botLeft = true;
  both.botRight = true;
  gameUpdate(game, both, false, 1100, 40);
  CHECK(game.botX == held);

  game.botX = 0;
  game.botSub = 0;
  for (int i = 0; i < 8; i++) {
    gameUpdate(game, left, false, (uint32_t)(1200 + i * 40), 40);
  }
  CHECK(game.botX == 0);

  Buttons right = none();
  right.botRight = true;
  gameUpdate(game, right, false, 2000, 40);
  CHECK(game.botX == (int16_t)(PADDLE_SPEED * 40 / 1000));

  int16_t edge = (int16_t)(SCREEN_W - PADDLE_LENGTH);
  game.topX = edge;
  game.topSub = 0;
  Buttons topRight = none();
  topRight.topRight = true;
  gameUpdate(game, topRight, false, 2100, 40);
  CHECK(game.topX == edge);

  Buttons topLeft = none();
  topLeft.topLeft = true;
  gameUpdate(game, topLeft, false, 2200, 40);
  CHECK(game.topX == (int16_t)(edge - PADDLE_SPEED * 40 / 1000));

  game.mode = Mode::PointPause;
  game.pauseStart = 3000;
  game.botSub = 0;
  int16_t during = game.botX;
  gameUpdate(game, right, false, 3040, 40);
  CHECK(game.botX == (int16_t)(during + PADDLE_SPEED * 40 / 1000));
  CHECK(game.mode == Mode::PointPause);
}

static void testFrameClamp() {
  title("a long frame cannot fling the ball across the table");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.vx = 100;
  game.vy = 0;
  game.ballX = (int32_t)80 * FP_ONE;
  game.ballY = (int32_t)BALL_REST_Y * FP_ONE;
  int32_t before = game.ballX;
  gameUpdate(game, none(), false, 4000, 500);
  int32_t expected = (int32_t)100 * MAX_FRAME_MS * FP_ONE / 1000;
  CHECK(game.ballX - before == expected);
  CHECK(game.ballY == (int32_t)BALL_REST_Y * FP_ONE);
  CHECK(game.mode == Mode::Play);
}

static void testRally() {
  title("a long rally keeps the ball and paddles on the glass");
  Game game;
  gameInit(game);
  uint32_t now = 5000;
  gameUpdate(game, none(), true, now, 16);
  CHECK(game.mode == Mode::Play);

  int before = failures;
  for (int i = 0; i < 800; i++) {
    now += 16;
    Buttons buttons = none();
    bool tap = false;
    bool wasTitle = game.mode == Mode::Title;
    uint8_t scoreBot = game.scoreBot;
    uint8_t scoreTop = game.scoreTop;
    if (wasTitle) {
      now += GAME_OVER_HOLD_MS;
      tap = true;
    } else if (game.mode == Mode::Play || game.mode == Mode::PointPause) {
      int16_t center = (int16_t)(game.ballX / FP_ONE + BALL_SIZE / 2);
      int16_t bottom = (int16_t)(game.botX + PADDLE_LENGTH / 2);
      int16_t top = (int16_t)(game.topX + PADDLE_LENGTH / 2);
      if (center < bottom - 2) {
        buttons.botLeft = true;
      } else if (center > bottom + 2) {
        buttons.botRight = true;
      }
      if (center < top - 2) {
        buttons.topLeft = true;
      } else if (center > top + 2) {
        buttons.topRight = true;
      }
    }

    gameUpdate(game, buttons, tap, now, 16);

    if (wasTitle) {
      CHECK(scoreBot >= POINTS_TO_WIN || scoreTop >= POINTS_TO_WIN);
      CHECK(game.mode == Mode::Play);
      CHECK(game.scoreBot == 0 && game.scoreTop == 0);
      CHECK(game.winner == 0);
    }
    int16_t maxX = (int16_t)(SCREEN_W - PADDLE_LENGTH);
    CHECK(game.botX >= 0 && game.botX <= maxX);
    CHECK(game.topX >= 0 && game.topX <= maxX);
    if (game.mode == Mode::Play) {
      CHECK(iabs16(game.vy) == SERVE_SPEED);
      CHECK(iabs16(game.vx) <= MAX_VX);
      CHECK(game.ballX >= 0);
      CHECK(game.ballX <= (int32_t)(SCREEN_W - BALL_SIZE) * FP_ONE);
      CHECK(game.ballY >= (int32_t)TOP_PADDLE_Y * FP_ONE);
      CHECK(game.ballY <=
            (int32_t)(BOTTOM_PADDLE_Y + PADDLE_THICK - BALL_SIZE) * FP_ONE);
      CHECK(game.scoreBot < POINTS_TO_WIN);
      CHECK(game.scoreTop < POINTS_TO_WIN);
    }
    if (failures != before) {
      std::printf("    stopped on rally frame %d\n", i);
      break;
    }
  }
}

int main() {
  std::printf("pong physics\n");
  testTitleAndServeDirection();
  testWalls();
  testTopPaddle();
  testBottomPaddleCap();
  testNoSpeedRamp();
  testMisses();
  testPause();
  testPauseDuringPoint();
  testWinReturnsToTitle();
  testPaddleControl();
  testFrameClamp();
  testRally();
  if (failures) {
    std::printf("%d failed\n", failures);
    return 1;
  }
  std::printf("all passed\n");
  return 0;
}
