#include "draining_battery.h"

#include "draining_battery_ui.h"

static void abort_charge(struct SharedState* state) {
  state->drain_stated_ms = 0;
  state->state = PROFILE_SELECTION;
}

void draining_battery(
    const struct Settings* settings,
    struct SharedState* state) {

  if (state->drain_stated_ms == 0) {
    state->drain_stated_ms = state->uptime_ms;
  }

  struct DrainingBatteryUIFields dui;
  // fake fields for now
  dui.time_seconds = (state->uptime_ms - state->drain_stated_ms) / 1000;
  dui.charge_mah = 500;
  dui.cells = 6;
  dui.current_mv = estimate_unloaded_mv(state);
  dui.target_mv = 22800;
  dui.finish_seconds = 0;
  dui.current_ma = 15700;
  dui.power_watts = 394;
  dui.temp_c = 55;
  dui.fet_percent = 100;
  dui.fan_percent = 25;
  dui.limiter = VOLTAGE_SAG;

  draining_battery_ui_render(&(state->text), &dui);

  if (state->button) {
    abort_charge(state);
  }
}

