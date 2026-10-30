#pragma once

void touchBegin();

// True for one frame when a finger lands. A finger that stays down does not
// repeat, and a second tap inside TAP_GAP_MS is ignored.
bool touchConsumeTap();
