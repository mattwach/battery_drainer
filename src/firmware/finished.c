#include "finished.h"
#include "draining_battery_ui.h"
#include "power.h"

#define MIN_FINISH_SECONDS 10

static void init_state(struct SharedState* state) {
  state->state_started_ms = state->uptime_ms;
}

void finished(struct SharedState* state, float finish_display) {
  if (state->state_started_ms == 0) {
    init_state(state);
  }

  uint16_t finish_seconds = (uint16_t)(
    (float)state->max_values.charge_mah * finish_display);
  if (finish_seconds < MIN_FINISH_SECONDS) {
    finish_seconds = MIN_FINISH_SECONDS;
  }

  const uint16_t elapsed_seconds = (
    state->uptime_ms - state->state_started_ms) / 1000;

  const uint16_t finish_seconds_left = 
    elapsed_seconds >= finish_seconds ?
    0 :
    finish_seconds - elapsed_seconds;

  draining_battery_ui_render_finished(
    state,
    finish_seconds_left);

  if (state->button || (finish_seconds_left == 0)) {
    power_off();
  }
}