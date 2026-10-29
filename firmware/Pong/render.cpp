#include "config.h"
#include "display.h"
#include "render.h"

static_assert(96 + 28 < (FIELD_TOP + FIELD_BOTTOM - BALL_SIZE) / 2,
              "title overlaps the ball");
static_assert(220 > (FIELD_TOP + FIELD_BOTTOM - BALL_SIZE) / 2 + BALL_SIZE,
              "subtitle overlaps the ball");
static_assert(112 + 21 < (FIELD_TOP + FIELD_BOTTOM - BALL_SIZE) / 2,
              "winner text overlaps the ball");

struct Sprite {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
  bool on;
};

struct Label {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
};

static Sprite sprBall = {0, 0, 0, 0, false};
static Sprite sprP1 = {0, 0, 0, 0, false};
static Sprite sprP2 = {0, 0, 0, 0, false};
static Label labels[2];
static uint8_t labelCount = 0;
static uint8_t shownL = 255;
static uint8_t shownR = 255;
static Mode shownMode = Mode::Title;

static bool overlap(const Sprite& a, const Sprite& b) {
  if (!a.on || !b.on) {
    return false;
  }
  return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

static void repairNet(int16_t x, int16_t y, int16_t w, int16_t h) {
  const int16_t netX = (int16_t)(SCREEN_W / 2 - 1);
  const int16_t netW = 2;
  if (x >= netX + netW || x + w <= netX || h <= 0 || w <= 0) {
    return;
  }
  if (y < FIELD_TOP) {
    h = (int16_t)(h - (FIELD_TOP - y));
    y = FIELD_TOP;
  }
  if (y + h > FIELD_BOTTOM) {
    h = (int16_t)(FIELD_BOTTOM - y);
  }
  if (h <= 0) {
    return;
  }

  const int16_t yEnd = (int16_t)(y + h);
  for (int16_t dash = FIELD_TOP; dash < yEnd; dash = (int16_t)(dash + 12)) {
    int16_t d0 = dash;
    int16_t d1 = (int16_t)(dash + 6);
    if (d1 <= y || d0 >= yEnd) {
      continue;
    }
    if (d0 < y) {
      d0 = y;
    }
    if (d1 > yEnd) {
      d1 = yEnd;
    }
    displayFillRect(netX, d0, netW, (int16_t)(d1 - d0), COL_NET);
  }
}

static void drawNet() {
  const int16_t netX = (int16_t)(SCREEN_W / 2 - 1);
  for (int16_t y = FIELD_TOP; y < FIELD_BOTTOM; y = (int16_t)(y + 12)) {
    int16_t h = 6;
    if (y + h > FIELD_BOTTOM) {
      h = (int16_t)(FIELD_BOTTOM - y);
    }
    displayFillRect(netX, y, 2, h, COL_NET);
  }
}

static void eraseLabels() {
  for (uint8_t i = 0; i < labelCount; i++) {
    displayFillRect(labels[i].x, labels[i].y, labels[i].w, labels[i].h, COL_BG);
    repairNet(labels[i].x, labels[i].y, labels[i].w, labels[i].h);
  }
  labelCount = 0;
}

static void showCentered(const char* text, int16_t y, uint8_t scale, uint16_t color) {
  int16_t w = displayTextWidth(text, scale);
  int16_t x = (int16_t)((SCREEN_W - w) / 2);
  displayText(x, y, text, color, COL_BG, scale);
  if (labelCount < 2) {
    labels[labelCount].x = x;
    labels[labelCount].y = y;
    labels[labelCount].w = w;
    labels[labelCount].h = (int16_t)(7 * scale);
    labelCount++;
  }
}

static void enterMode(const Game& game) {
  if (game.mode == Mode::Title) {
    showCentered("PONG", 96, 4, COL_TITLE);
    showCentered("PRESS ANY BUTTON", 220, 2, COL_SUB);
  } else if (game.mode == Mode::GameOver) {
    if (game.winner == 1) {
      showCentered("LEFT WINS", 112, 3, COL_P1);
    } else {
      showCentered("RIGHT WINS", 112, 3, COL_P2);
    }
    showCentered("PRESS ANY BUTTON", 220, 2, COL_SUB);
  }
}

static int16_t scoreAnchor(uint8_t side) {
  int16_t w = displayTextWidth("00", 3);
  int16_t half = (int16_t)(SCREEN_W / 2);
  int16_t x = (int16_t)((half - w) / 2);
  if (side == 1) {
    x = (int16_t)(half + x);
  }
  return x;
}

static void drawScores(const Game& game) {
  if (game.scoreL != shownL) {
    char text[3];
    text[0] = (char)('0' + game.scoreL / 10);
    text[1] = (char)('0' + game.scoreL % 10);
    text[2] = 0;
    displayText(scoreAnchor(0), 6, text, COL_P1, COL_BG, 3);
    shownL = game.scoreL;
  }
  if (game.scoreR != shownR) {
    char text[3];
    text[0] = (char)('0' + game.scoreR / 10);
    text[1] = (char)('0' + game.scoreR % 10);
    text[2] = 0;
    displayText(scoreAnchor(1), 6, text, COL_P2, COL_BG, 3);
    shownR = game.scoreR;
  }
}

static void paintPaddle(Sprite& sprite, int16_t x, int16_t y, uint16_t color, bool full) {
  const int16_t w = PADDLE_W;
  const int16_t h = PADDLE_H;
  if (!sprite.on) {
    displayFillRect(x, y, w, h, color);
    sprite.x = x;
    sprite.y = y;
    sprite.w = w;
    sprite.h = h;
    sprite.on = true;
    return;
  }
  if (!full && sprite.x == x && sprite.y == y) {
    return;
  }

  int16_t delta = (int16_t)(y - sprite.y);
  if (delta < 0) {
    delta = (int16_t)(-delta);
  }
  bool slide = !full && sprite.x == x && sprite.w == w && sprite.h == h && delta < h && delta != 0;
  if (!slide) {
    displayFillRect(sprite.x, sprite.y, sprite.w, sprite.h, COL_BG);
    repairNet(sprite.x, sprite.y, sprite.w, sprite.h);
    displayFillRect(x, y, w, h, color);
  } else if (y > sprite.y) {
    int16_t dy = (int16_t)(y - sprite.y);
    displayFillRect(sprite.x, sprite.y, w, dy, COL_BG);
    displayFillRect(x, (int16_t)(sprite.y + h), w, dy, color);
  } else {
    int16_t dy = (int16_t)(sprite.y - y);
    displayFillRect(sprite.x, (int16_t)(y + h), w, dy, COL_BG);
    displayFillRect(x, y, w, dy, color);
  }
  sprite.x = x;
  sprite.y = y;
  sprite.w = w;
  sprite.h = h;
  sprite.on = true;
}

static void drawPlay(const Game& game) {
  int16_t p1x = PADDLE_INSET;
  int16_t p2x = (int16_t)(SCREEN_W - PADDLE_INSET - PADDLE_W);
  int16_t bx = (int16_t)(game.ballX / FP_ONE);
  int16_t by = (int16_t)(game.ballY / FP_ONE);
  bool showBall = game.mode == Mode::Title || game.mode == Mode::Play ||
                  game.mode == Mode::PointPause;

  bool punchedP1 = false;
  bool punchedP2 = false;
  bool paintBall = showBall && !sprBall.on;
  if (sprBall.on && (!showBall || sprBall.x != bx || sprBall.y != by)) {
    displayFillRect(sprBall.x, sprBall.y, sprBall.w, sprBall.h, COL_BG);
    repairNet(sprBall.x, sprBall.y, sprBall.w, sprBall.h);
    punchedP1 = overlap(sprBall, sprP1);
    punchedP2 = overlap(sprBall, sprP2);
    sprBall.on = false;
    paintBall = showBall;
  }

  // A moving paddle slides, so the body stays lit. A ball that crossed a
  // paddle punches a hole, and that paddle is painted in full on that frame.
  if (!sprP1.on || sprP1.y != game.p1Y || punchedP1) {
    paintPaddle(sprP1, p1x, game.p1Y, COL_P1, punchedP1);
  }
  if (!sprP2.on || sprP2.y != game.p2Y || punchedP2) {
    paintPaddle(sprP2, p2x, game.p2Y, COL_P2, punchedP2);
  }

  if (paintBall) {
    displayFillRect(bx, by, BALL_SIZE, BALL_SIZE, COL_BALL);
    sprBall.x = bx;
    sprBall.y = by;
    sprBall.w = BALL_SIZE;
    sprBall.h = BALL_SIZE;
    sprBall.on = true;
  }
}

void renderBegin(const Game& game) {
  displayFillScreen(COL_BG);
  displayFillRect(0, (int16_t)(FIELD_TOP - 2), SCREEN_W, 1, COL_NET);
  displayFillRect(0, FIELD_BOTTOM, SCREEN_W, 1, COL_NET);
  drawNet();
  shownL = 255;
  shownR = 255;
  labelCount = 0;
  sprBall.on = false;
  sprP1.on = false;
  sprP2.on = false;
  enterMode(game);
  shownMode = game.mode;
  drawScores(game);
  drawPlay(game);
}

void render(const Game& game) {
  if (game.mode != shownMode) {
    eraseLabels();
    shownMode = game.mode;
    enterMode(game);
  }
  drawScores(game);
  drawPlay(game);
}
