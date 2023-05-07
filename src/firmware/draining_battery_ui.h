#ifndef DRAINING_BATTERY_UI_H
#define DRAINING_BATTERY_UI_H

#include "state.h"

// Note: we cant use struct SharedState because it would create a circular
// dependency
void draining_battery_ui_render_active(struct SharedState* state);

void draining_battery_ui_render_finished(
  struct SharedState* state, uint16_t finish_seconds);

#endif

