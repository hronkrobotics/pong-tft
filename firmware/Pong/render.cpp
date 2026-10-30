#include "config.h"
#include "display.h"
#include "render.h"

constexpr int16_t TITLE_Y = 96;
constexpr uint8_t TITLE_SCALE = 4;
constexpr int16_t WINNER_Y = 150;
constexpr uint8_t WINNER_SCALE = 3;
constexpr int16_t SUB_Y = 300;
constexpr uint8_t SUB_SCALE = 2;
constexpr int16_t PAUSE_Y = 150;
constexpr uint8_t PAUSE_SCALE = 4;

constexpr int16_t SCORE_SCALE = 3;
constexpr int16_t TOP_SCORE_Y = (int16_t)(TOP_PADDLE_Y + PADDLE_THICK + 8);
constexpr int16_t BOT_SCORE_Y =
    (int16_t)(BOTTOM_PADDLE_Y - 8 - 7 * SCORE_SCALE);

constexpr int16_t NET_Y =
    (int16_t)((TOP_PADDLE_Y + PADDLE_THICK + BOTTOM_PADDLE_Y) / 2 - 1);
constexpr int16_t NET_H = 2;

static_assert(TITLE_Y + 7 * TITLE_SCALE < BALL_REST_Y, "title overlaps the ball");
static_assert(WINNER_Y + 7 * WINNER_SCALE < BALL_REST_Y, "winner text overlaps the ball");
static_assert(SUB_Y > BALL_REST_Y + BALL_SIZE, "subtitle overlaps the ball");
static_assert(TOP_SCORE_Y >= TOP_PADDLE_Y + PADDLE_THICK, "top score clears the paddle");
static_assert(BOT_SCORE_Y + 7 * SCORE_SCALE <= BOTTOM_PADDLE_Y,
              "bottom score clears the paddle");
static_assert(TITLE_Y >= TOP_SCORE_Y + 7 * SCORE_SCALE, "title clears the top score");
static_assert(SUB_Y + 7 * SUB_SCALE <= BOT_SCORE_Y, "subtitle clears the bottom score");

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

struct ScoreBox {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
};

static Sprite sprBall = {0, 0, 0, 0, false};
static Sprite sprBot = {0, 0, 0, 0, false};
static Sprite sprTop = {0, 0, 0, 0, false};
static Label labels[3];
static uint8_t labelCount = 0;
static uint8_t shownBot = 255;
static uint8_t shownTop = 255;
static bool botHot = false;
static bool topHot = false;
static Mode shownMode = Mode::Title;
static ScoreBox botBox = {0, 0, 0, 0};
static ScoreBox topBox = {0, 0, 0, 0};

static bool overlap(const Sprite& a, const Sprite& b) {
  if (!a.on || !b.on) {
    return false;
  }
  return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

static bool hitsBox(const ScoreBox& box, int16_t x, int16_t y, int16_t w, int16_t h) {
  if (box.w <= 0 || w <= 0 || h <= 0) {
    return false;
  }
  return x < box.x + box.w && x + w > box.x && y < box.y + box.h && y + h > box.y;
}

static void repairNet(int16_t x, int16_t y, int16_t w, int16_t h) {
  if (w <= 0 || h <= 0) {
    return;
  }
  if (y >= NET_Y + NET_H || y + h <= NET_Y) {
    return;
  }
  if (x < 0) {
    w = (int16_t)(w + x);
    x = 0;
  }
  if (x >= SCREEN_W || w <= 0) {
    return;
  }
  if (x + w > SCREEN_W) {
    w = (int16_t)(SCREEN_W - x);
  }

  const int16_t xEnd = (int16_t)(x + w);
  int16_t dash = (int16_t)(x - (int16_t)(x % 12));
  for (; dash < xEnd; dash = (int16_t)(dash + 12)) {
    int16_t d0 = dash;
    int16_t d1 = (int16_t)(dash + 6);
    if (d1 <= x || d0 >= xEnd) {
      continue;
    }
    if (d0 < x) {
      d0 = x;
    }
    if (d1 > xEnd) {
      d1 = xEnd;
    }
    displayFillRect(d0, NET_Y, (int16_t)(d1 - d0), NET_H, COL_NET);
  }
}

static void drawNet() {
  for (int16_t x = 0; x < SCREEN_W; x = (int16_t)(x + 12)) {
    int16_t w = 6;
    if (x + w > SCREEN_W) {
      w = (int16_t)(SCREEN_W - x);
    }
    displayFillRect(x, NET_Y, w, NET_H, COL_NET);
  }
}

static bool labelsCoverBall() {
  if (!sprBall.on) {
    return false;
  }
  for (uint8_t i = 0; i < labelCount; i++) {
    Sprite box;
    box.x = labels[i].x;
    box.y = labels[i].y;
    box.w = labels[i].w;
    box.h = labels[i].h;
    box.on = true;
    if (overlap(sprBall, box)) {
      return true;
    }
  }
  return false;
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
  if (labelCount < 3) {
    labels[labelCount].x = x;
    labels[labelCount].y = y;
    labels[labelCount].w = w;
    labels[labelCount].h = (int16_t)(7 * scale);
    labelCount++;
  }
}

static void enterMode(const Game& game) {
  if (game.mode == Mode::Title) {
    showCentered("PONG", TITLE_Y, TITLE_SCALE, COL_TITLE);
    if (game.winner == 1) {
      showCentered("BOTTOM WINS", WINNER_Y, WINNER_SCALE, COL_P1);
    } else if (game.winner == 2) {
      showCentered("TOP WINS", WINNER_Y, WINNER_SCALE, COL_P2);
    }
    showCentered("TAP TO START", SUB_Y, SUB_SCALE, COL_SUB);
  } else if (game.mode == Mode::Paused) {
    showCentered("PAUSED", PAUSE_Y, PAUSE_SCALE, COL_TITLE);
    showCentered("TAP TO RESUME", SUB_Y, SUB_SCALE, COL_SUB);
  }
}

static void prepareScores() {
  int16_t w = displayTextWidth("00", SCORE_SCALE);
  int16_t x = (int16_t)((SCREEN_W - w) / 2);
  int16_t h = (int16_t)(7 * SCORE_SCALE);
  topBox.x = x;
  topBox.y = TOP_SCORE_Y;
  topBox.w = w;
  topBox.h = h;
  botBox.x = x;
  botBox.y = BOT_SCORE_Y;
  botBox.w = w;
  botBox.h = h;
}

static void drawOneScore(uint8_t value, const ScoreBox& box, uint16_t color) {
  char text[3];
  text[0] = (char)('0' + value / 10);
  text[1] = (char)('0' + value % 10);
  text[2] = 0;
  displayText(box.x, box.y, text, color, COL_BG, SCORE_SCALE);
}

static void drawScores(const Game& game) {
  bool hitTop = sprBall.on && hitsBox(topBox, sprBall.x, sprBall.y, sprBall.w, sprBall.h);
  bool hitBot = sprBall.on && hitsBox(botBox, sprBall.x, sprBall.y, sprBall.w, sprBall.h);
  if (game.scoreTop != shownTop || hitTop || topHot) {
    drawOneScore(game.scoreTop, topBox, COL_P2);
    shownTop = game.scoreTop;
  }
  if (game.scoreBot != shownBot || hitBot || botHot) {
    drawOneScore(game.scoreBot, botBox, COL_P1);
    shownBot = game.scoreBot;
  }
  topHot = hitTop;
  botHot = hitBot;
}

static void paintPaddle(Sprite& sprite, int16_t x, int16_t y, uint16_t color, bool full) {
  const int16_t w = PADDLE_LENGTH;
  const int16_t h = PADDLE_THICK;
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

  int16_t delta = (int16_t)(x - sprite.x);
  if (delta < 0) {
    delta = (int16_t)(-delta);
  }
  bool slide = !full && sprite.y == y && sprite.w == w && sprite.h == h && delta < w &&
               delta != 0;
  if (!slide) {
    displayFillRect(sprite.x, sprite.y, sprite.w, sprite.h, COL_BG);
    repairNet(sprite.x, sprite.y, sprite.w, sprite.h);
    displayFillRect(x, y, w, h, color);
  } else if (x > sprite.x) {
    int16_t dx = (int16_t)(x - sprite.x);
    displayFillRect(sprite.x, sprite.y, dx, h, COL_BG);
    repairNet(sprite.x, sprite.y, dx, h);
    displayFillRect((int16_t)(sprite.x + w), y, dx, h, color);
  } else {
    int16_t dx = (int16_t)(sprite.x - x);
    displayFillRect((int16_t)(x + w), y, dx, h, COL_BG);
    repairNet((int16_t)(x + w), y, dx, h);
    displayFillRect(x, y, dx, h, color);
  }
  sprite.x = x;
  sprite.y = y;
  sprite.w = w;
  sprite.h = h;
  sprite.on = true;
}

static void drawPlay(const Game& game, bool forceBall) {
  int16_t bx = (int16_t)(game.ballX / FP_ONE);
  int16_t by = (int16_t)(game.ballY / FP_ONE);
  bool showBall = true;

  bool punchedBot = false;
  bool punchedTop = false;
  bool moved = sprBall.on && (sprBall.x != bx || sprBall.y != by);
  bool paintBall = showBall && (!sprBall.on || moved || forceBall);
  if (sprBall.on && (!showBall || moved || forceBall)) {
    displayFillRect(sprBall.x, sprBall.y, sprBall.w, sprBall.h, COL_BG);
    repairNet(sprBall.x, sprBall.y, sprBall.w, sprBall.h);
    punchedBot = overlap(sprBall, sprBot);
    punchedTop = overlap(sprBall, sprTop);
    sprBall.on = false;
  }

  // A moving paddle slides, so the body stays lit. A ball that crossed a
  // paddle punches a hole, and that paddle is painted in full on that frame.
  if (!sprBot.on || sprBot.x != game.botX || punchedBot) {
    paintPaddle(sprBot, game.botX, BOTTOM_PADDLE_Y, COL_P1, punchedBot);
  }
  if (!sprTop.on || sprTop.x != game.topX || punchedTop) {
    paintPaddle(sprTop, game.topX, TOP_PADDLE_Y, COL_P2, punchedTop);
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
  drawNet();
  shownBot = 255;
  shownTop = 255;
  botHot = false;
  topHot = false;
  labelCount = 0;
  sprBall.on = false;
  sprBot.on = false;
  sprTop.on = false;
  prepareScores();
  shownMode = game.mode;
  drawPlay(game, false);
  enterMode(game);
  drawScores(game);
}

void render(const Game& game) {
  if (game.mode != shownMode) {
    bool cover = labelsCoverBall();
    eraseLabels();
    shownMode = game.mode;
    drawPlay(game, cover);
    enterMode(game);
  } else {
    drawPlay(game, false);
  }
  drawScores(game);
}
