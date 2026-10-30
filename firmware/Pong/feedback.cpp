#include <Arduino.h>

#include "config.h"
#include "feedback.h"

void feedback(const Game& game, GameEvent event) {
  if (event == GameEvent::Score || event == GameEvent::Win) {
    Serial.print(F("bottom "));
    Serial.print(game.scoreBot);
    Serial.print(F("  top "));
    Serial.println(game.scoreTop);
  }
  if (event == GameEvent::Win) {
    if (game.winner == 1) {
      Serial.println(F("bottom wins"));
    } else {
      Serial.println(F("top wins"));
    }
  }
  if (event == GameEvent::Pause) {
    Serial.println(F("paused"));
  }
  if (event == GameEvent::Resume) {
    Serial.println(F("resumed"));
  }

#if BUZZER_ENABLED
  switch (event) {
    case GameEvent::Paddle:
      tone(PIN_BUZZER, 880, 16);
      break;
    case GameEvent::Wall:
      tone(PIN_BUZZER, 1320, 10);
      break;
    case GameEvent::Score:
      tone(PIN_BUZZER, 220, 120);
      break;
    case GameEvent::Win:
      tone(PIN_BUZZER, 523, 200);
      break;
    case GameEvent::Serve:
      tone(PIN_BUZZER, 660, 24);
      break;
    case GameEvent::Pause:
      tone(PIN_BUZZER, 440, 30);
      break;
    case GameEvent::Resume:
      tone(PIN_BUZZER, 660, 30);
      break;
    case GameEvent::None:
      break;
  }
#endif
}
