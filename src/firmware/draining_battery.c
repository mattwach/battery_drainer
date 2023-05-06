#include "draining_battery.h"

#include "draining_battery_ui.h"

// how often to accumulate into charge_mas
// at a rating of 100ms, there needs to me at least 5ma
// of draw to get over 1 mas added.  Thus anything less
// than 5 ma is not counted
#define CHARGE_SAMPLE_INTERVAL_MS 200

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
  state->last_charge_sample_ms = state->uptime_ms;
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

static void accumulate_charge(struct SharedState* state) {
  const uint32_t ms_elapsed = state->uptime_ms - state->last_charge_sample_ms;
  // we assume that whatever ma we are at now was the value over the
  // last ms_elapsed
  const uint32_t mas_accumulated = (state->current_ma * ms_elapsed) / 1000;
  state->charge_mas += mas_accumulated;
  state->last_charge_sample_ms = state->uptime_ms;
}

void draining_battery(
    const struct Settings* settings,
    struct SharedState* state) {

  if (state->state_started_ms == 0) {
    init_state(state);
  }

  if ((state->uptime_ms - state->last_charge_sample_ms) >=
      CHARGE_SAMPLE_INTERVAL_MS) {
    accumulate_charge(state);
  }

  struct DrainingBatteryUIFields dui;
  // fake fields for now
  dui.time_seconds = (state->uptime_ms - state->state_started_ms) / 1000;
  dui.charge_mah = state->charge_mas / 3600;
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

