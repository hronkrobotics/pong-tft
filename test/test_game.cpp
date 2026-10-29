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

static void testTitleAndOpeningServe() {
  title("title waits, then the opening serve travels left");
  Game game;
  gameInit(game);
  CHECK(game.mode == Mode::Title);
  CHECK(game.vx == 0);
  CHECK(game.scoreL == 0 && game.scoreR == 0);

  uint32_t now = 10000;
  GameEvent event = gameUpdate(game, none(), false, now, 20);
  CHECK(event == GameEvent::None);
  CHECK(game.mode == Mode::Title);

  event = gameUpdate(game, none(), true, now + 20, 20);
  CHECK(event == GameEvent::Serve);
  CHECK(game.mode == Mode::Play);
  CHECK(game.vx < 0);
  CHECK(game.scoreL == 0 && game.scoreR == 0);
}

static void testWalls() {
  title("ball bounces off the top and bottom");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.ballX = (int32_t)200 * FP_ONE;
  game.ballY = (int32_t)FIELD_TOP * FP_ONE + 50;
  game.vx = 80;
  game.vy = -400;

  GameEvent event = gameUpdate(game, none(), false, 2000, 20);
  CHECK(event == GameEvent::Wall);
  CHECK(game.ballY == (int32_t)FIELD_TOP * FP_ONE);
  CHECK(game.vy == 400);

  const int32_t maxY = (int32_t)(FIELD_BOTTOM - BALL_SIZE) * FP_ONE;
  game.ballY = maxY - 50;
  game.vy = 400;
  event = gameUpdate(game, none(), false, 2020, 20);
  CHECK(event == GameEvent::Wall);
  CHECK(game.ballY == maxY);
  CHECK(game.vy == -400);
}

static void testLeftPaddle() {
  title("left paddle returns the ball and kicks it upward");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.p1Y = 120;
  game.ballX = (int32_t)(PADDLE_INSET + PADDLE_W + 2) * FP_ONE;
  game.ballY = (int32_t)(game.p1Y + 4) * FP_ONE;
  game.vx = -200;
  game.vy = 0;

  GameEvent event = gameUpdate(game, none(), false, 3000, 20);
  CHECK(event == GameEvent::Paddle);
  CHECK(game.vx == (int16_t)(200 + SPEED_BUMP));
  CHECK(game.vy < 0);
  CHECK(game.ballX == (int32_t)(PADDLE_INSET + PADDLE_W) * FP_ONE);
  CHECK(game.mode == Mode::Play);
  CHECK(game.scoreL == 0 && game.scoreR == 0);
}

static void testRightPaddleAndCap() {
  title("right paddle returns the ball, and speed stays capped");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.p2Y = 120;
  const int32_t limit =
      (int32_t)(SCREEN_W - PADDLE_INSET - PADDLE_W - BALL_SIZE) * FP_ONE;
  game.ballX = limit - (int32_t)2 * FP_ONE;
  game.ballY = (int32_t)(game.p2Y + PADDLE_H / 2 - BALL_SIZE / 2) * FP_ONE;
  game.vx = MAX_VX;
  game.vy = 0;

  GameEvent event = gameUpdate(game, none(), false, 4000, 20);
  CHECK(event == GameEvent::Paddle);
  CHECK(game.vx == (int16_t)(-MAX_VX));
  CHECK(game.ballX == limit);
  CHECK(game.scoreL == 0 && game.scoreR == 0);
}

static void testLeftMissAndServe() {
  title("a left miss scores for the right, then serves back left");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.p1Y = (int16_t)((FIELD_TOP + FIELD_BOTTOM - PADDLE_H) / 2);
  game.ballY = (int32_t)FIELD_TOP * FP_ONE;
  game.ballX = (int32_t)(PADDLE_INSET + PADDLE_W + 4) * FP_ONE;
  game.vx = -1000;
  game.vy = 0;

  uint32_t now = 10000;
  GameEvent event = gameUpdate(game, none(), false, now, 40);
  CHECK(event == GameEvent::Score);
  CHECK(game.scoreR == 1);
  CHECK(game.scoreL == 0);
  CHECK(game.mode == Mode::PointPause);
  CHECK(game.serveDir < 0);
  CHECK(game.vx == 0);

  event = gameUpdate(game, none(), true, now + 10, 16);
  CHECK(game.mode == Mode::PointPause);

  event = gameUpdate(game, none(), false, now + POINT_PAUSE_MS - 1, 16);
  CHECK(game.mode == Mode::PointPause);

  event = gameUpdate(game, none(), false, now + POINT_PAUSE_MS, 16);
  CHECK(event == GameEvent::Serve);
  CHECK(game.mode == Mode::Play);
  CHECK(game.vx < 0);
}

static void testRightMiss() {
  title("a right miss scores for the left and the next serve travels right");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.p2Y = FIELD_TOP;
  game.ballY = (int32_t)(FIELD_BOTTOM - BALL_SIZE - 4) * FP_ONE;
  const int32_t limit =
      (int32_t)(SCREEN_W - PADDLE_INSET - PADDLE_W - BALL_SIZE) * FP_ONE;
  game.ballX = limit - (int32_t)4 * FP_ONE;
  game.vx = 1000;
  game.vy = 0;

  uint32_t now = 20000;
  GameEvent event = gameUpdate(game, none(), false, now, 40);
  CHECK(event == GameEvent::Score);
  CHECK(game.scoreL == 1);
  CHECK(game.scoreR == 0);
  CHECK(game.serveDir > 0);

  event = gameUpdate(game, none(), false, now + POINT_PAUSE_MS, 16);
  CHECK(event == GameEvent::Serve);
  CHECK(game.vx > 0);
}

static void testWinAndRematch() {
  title("first to 11 wins, and a press starts a fresh match");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.scoreL = (uint8_t)(POINTS_TO_WIN - 1);
  game.p2Y = FIELD_TOP;
  game.ballY = (int32_t)(FIELD_BOTTOM - BALL_SIZE - 4) * FP_ONE;
  const int32_t limit =
      (int32_t)(SCREEN_W - PADDLE_INSET - PADDLE_W - BALL_SIZE) * FP_ONE;
  game.ballX = limit - (int32_t)4 * FP_ONE;
  game.vx = 1000;
  game.vy = 0;

  GameEvent event = gameUpdate(game, none(), false, 30000, 40);
  CHECK(event == GameEvent::Win);
  CHECK(game.mode == Mode::GameOver);
  CHECK(game.winner == 1);
  CHECK(game.scoreL == POINTS_TO_WIN);

  event = gameUpdate(game, none(), true, 31000, 16);
  CHECK(event == GameEvent::Serve);
  CHECK(game.mode == Mode::Play);
  CHECK(game.scoreL == 0 && game.scoreR == 0);
  CHECK(game.winner == 0);
  CHECK(game.vx < 0);
}

static void testPaddleControl() {
  title("paddles follow one button, ignore both, and unstick at the edge");
  Game game;
  gameInit(game);
  int16_t y = game.p1Y;
  Buttons up = none();
  up.p1Up = true;
  gameUpdate(game, up, false, 1000, 40);
  CHECK(game.p1Y == (int16_t)(y - PADDLE_SPEED * 40 / 1000));
  CHECK(game.mode == Mode::Title);

  int16_t held = game.p1Y;
  Buttons both = none();
  both.p1Up = true;
  both.p1Down = true;
  gameUpdate(game, both, false, 1100, 40);
  CHECK(game.p1Y == held);

  game.p1Y = FIELD_TOP;
  game.p1Sub = 0;
  for (int i = 0; i < 8; i++) {
    gameUpdate(game, up, false, (uint32_t)(1200 + i * 40), 40);
  }
  CHECK(game.p1Y == FIELD_TOP);

  Buttons down = none();
  down.p1Down = true;
  gameUpdate(game, down, false, 2000, 40);
  CHECK(game.p1Y == (int16_t)(FIELD_TOP + PADDLE_SPEED * 40 / 1000));

  int16_t bottom = (int16_t)(FIELD_BOTTOM - PADDLE_H);
  game.p2Y = bottom;
  game.p2Sub = 0;
  Buttons rightDown = none();
  rightDown.p2Down = true;
  gameUpdate(game, rightDown, false, 2100, 40);
  CHECK(game.p2Y == bottom);
}

static void testFrameClamp() {
  title("a long frame cannot fling the ball across the table");
  Game game;
  gameInit(game);
  game.mode = Mode::Play;
  game.vx = 200;
  game.vy = 0;
  game.ballX = (int32_t)200 * FP_ONE;
  game.ballY = (int32_t)((FIELD_TOP + FIELD_BOTTOM - BALL_SIZE) / 2) * FP_ONE;
  int32_t before = game.ballX;
  gameUpdate(game, none(), false, 4000, 500);
  int32_t expected = (int32_t)200 * MAX_FRAME_MS * FP_ONE / 1000;
  CHECK(game.ballX - before == expected);
  CHECK(game.mode == Mode::Play);
}

static void testRally() {
  title("a long rally keeps the ball and paddles inside the field");
  Game game;
  gameInit(game);
  uint32_t now = 5000;
  gameUpdate(game, none(), true, now, 16);
  CHECK(game.mode == Mode::Play);

  int before = failures;
  for (int i = 0; i < 800; i++) {
    now += 16;
    Buttons buttons = none();
    bool edge = false;
    bool wasOver = game.mode == Mode::GameOver;
    uint8_t scoreL = game.scoreL;
    uint8_t scoreR = game.scoreR;
    if (wasOver) {
      edge = true;
    } else {
      int16_t center = (int16_t)(game.ballY / FP_ONE + BALL_SIZE / 2);
      int16_t left = (int16_t)(game.p1Y + PADDLE_H / 2);
      int16_t right = (int16_t)(game.p2Y + PADDLE_H / 2);
      if (center < left - 2) {
        buttons.p1Up = true;
      } else if (center > left + 2) {
        buttons.p1Down = true;
      }
      if (center < right - 2) {
        buttons.p2Up = true;
      } else if (center > right + 2) {
        buttons.p2Down = true;
      }
    }

    gameUpdate(game, buttons, edge, now, 16);

    if (wasOver) {
      CHECK(scoreL >= POINTS_TO_WIN || scoreR >= POINTS_TO_WIN);
      CHECK(game.mode == Mode::Play);
      CHECK(game.scoreL == 0 && game.scoreR == 0);
    }
    CHECK(game.p1Y >= FIELD_TOP);
    CHECK(game.p1Y <= FIELD_BOTTOM - PADDLE_H);
    CHECK(game.p2Y >= FIELD_TOP);
    CHECK(game.p2Y <= FIELD_BOTTOM - PADDLE_H);
    if (game.mode == Mode::Play) {
      CHECK(game.vx != 0);
      CHECK(game.ballY >= (int32_t)FIELD_TOP * FP_ONE);
      CHECK(game.ballY <= (int32_t)(FIELD_BOTTOM - BALL_SIZE) * FP_ONE);
      CHECK(game.scoreL < POINTS_TO_WIN);
      CHECK(game.scoreR < POINTS_TO_WIN);
    }
    if (failures != before) {
      std::printf("    stopped on rally frame %d\n", i);
      break;
    }
  }
}

int main() {
  std::printf("pong physics\n");
  testTitleAndOpeningServe();
  testWalls();
  testLeftPaddle();
  testRightPaddleAndCap();
  testLeftMissAndServe();
  testRightMiss();
  testWinAndRematch();
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
