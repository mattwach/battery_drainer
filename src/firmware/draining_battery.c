#include "draining_battery.h"

#include "draining_battery_ui.h"

static void abort_charge(struct SharedState* state) {
  state_change(state, FINISHED);
}

static inline void max8(uint8_t cur, uint8_t *final) {
  if (cur > *final) {
    *final = cur;
  }
}

static inline void max16(uint16_t cur, uint16_t *final) {
  if (cur > *final) {
    *final = cur;
  }
}

static void init_state(struct SharedState* state) {
  state->state_started_ms = state->uptime_ms;
}

static void update_final_stats(
  const struct DrainingBatteryUIFields* cur,
  struct DrainingBatteryUIFields* final) {
  final->time_seconds = cur->time_seconds;
  final->charge_mah = cur->charge_mah;
  max16(cur->current_mv, &final->current_mv);
  max16(cur->current_ma, &final->current_ma);
  max16(cur->power_watts, &final->power_watts);
  max8(cur->temp_c, &final->temp_c);
  max8(cur->fet_percent, &final->fet_percent);
  max8(cur->fan_percent, &final->fan_percent);
}

void draining_battery(
    const struct Settings* settings,
    struct SharedState* state) {

  if (state->state_started_ms == 0) {
    init_state(state);
  }

  struct DrainingBatteryUIFields dui;
  // fake fields for now
  dui.time_seconds = (state->uptime_ms - state->state_started_ms) / 1000;
  dui.charge_mah = 500;
  dui.current_mv = estimate_unloaded_mv(state);
  dui.current_ma = state->current_ma;
  dui.power_watts = (
    ((uint32_t)dui.current_mv * (uint32_t)dui.current_ma) /
    1000000);
  dui.temp_c = state->temperature_c;
  dui.fet_percent = 100;
  dui.fan_percent = 25;
  dui.limiter = VOLTAGE_SAG;

  update_final_stats(&dui, &state->final_stats);

  draining_battery_ui_render_active(&(state->text), &dui, state->cells, state->target_mv);

  if (state->button) {
    abort_charge(state);
  }
}

