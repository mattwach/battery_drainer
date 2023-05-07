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
  state->vgs_level = 0;
}

static void update_final_stats(struct SharedState* ss) {
  struct MaxValues* mv = &(ss->max_values);
  max16((ss->uptime_ms - ss->state_started_ms) / 1000, &(mv->time_seconds));
  max16(ss->charge_mas / 3600, &(mv->charge_mah));
  max16(ss->current_ma, &(mv->current_ma));
  max16(
    ss->loaded_mv * ss->current_ma / 1000000,
    &(mv->power_watts));
  max8(ss->temperature_c, &(mv->temperature_c));
  max8(
    (uint8_t)(((uint32_t)ss->vgs_level * 100) / 65535),
    &(mv->fet_percent));
  max8(
    (uint8_t)(((uint32_t)ss->fan_level * 100) / 65535),
    &(mv->fan_percent));
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

  update_final_stats(state);

  draining_battery_ui_render_active(state);

  if (state->button) {
    abort_charge(state);
  }
}

