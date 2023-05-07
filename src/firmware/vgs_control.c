#include "vgs_control.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define FAST_GPIO 16
#define SLOW_GPIO 17
// These map to FAST_GPIO and SLOW_GPIO via the hardware spec.
// there are also pwm_gpio_to_PWM_SLICE and pwm_gpio_to_channel
// functions available but that create uneeded runtime variables
// for values that are static
#define PWM_SLICE 0
#define FAST_CHAN 0
#define SLOW_CHAN 1

#define MAX_FREQ 125000000
// The tradeoff
// Higher frequency = more stable vgs but less change resolution
// 10000 sets the divider at 12500, which allow for that many discrete steps
#define FREQ 10000
#define WRAP (MAX_FREQ / FREQ)

// duty cycle ranges from 0-32767
static inline uint16_t calc_level(uint16_t duty_cycle) {
  if (duty_cycle >= 32767) {
    return WRAP;
  }
  return (WRAP * (uint32_t)duty_cycle) / 32767;
}

void vgs_control_init(void) {
  gpio_set_function(FAST_GPIO, GPIO_FUNC_PWM);
  gpio_set_function(SLOW_GPIO, GPIO_FUNC_PWM);
  pwm_set_enabled(PWM_SLICE, 0);
  pwm_set_wrap(PWM_SLICE, WRAP);
}

static void set_level(const struct SharedState* state) {
  pwm_set_enabled(PWM_SLICE, 0);
  if (state->vgs_level == 0) {
    return;
  }
  // note that values >= 32768 are capped at full-on within calc_level
  pwm_set_chan_level(PWM_SLICE, SLOW_CHAN, calc_level(state->vgs_level));
  if (state->vgs_level <= 32768) {
    pwm_set_chan_level(PWM_SLICE, FAST_CHAN, 0);
  } else {
    pwm_set_chan_level(
      PWM_SLICE, FAST_CHAN, calc_level(state->vgs_level - 32768));
  }
  pwm_set_enabled(PWM_SLICE, 1);
}

static enum Limiter find_limiter(
  const struct Settings* settings, const struct SharedState* state) {
    const struct ProfileSettings* ps =
        settings->profile + state->active_profile_index;

    if (state->current_ma > ps->max_ma) {
      return CURRENT_LIMIT;
    }

    if (state->temperature_c > ps->max_celsius) {
      return TEMPERATURE_LIMIT;
    }

    const uint32_t power = state->current_ma * state->loaded_mv / 1000000;
    if (power > ps->max_watts) {
      return POWER_LIMIT;
    }

    const uint16_t sag_mv =
      state->loaded_mv <= state->last_unloaded_sample_mv ?
      state->last_unloaded_sample_mv - state->loaded_mv :
      0;
    const uint16_t per_cell_sag_mv = sag_mv / state->cells;
    if (per_cell_sag_mv > ps->cell.max_vsag_mv) {
      return VOLTAGE_SAG_LIMIT;
    }

    return NO_LIMIT;
}

static uint16_t calc_new_level(
  const struct Settings* settings, struct SharedState* state) {
  if (state->state != DRAINING_BATTERY) {
    return 0;
  }

  if (state->is_sampling_voltage) {
    return 0;
  }

  const struct ResponseSettings* r = &(settings->global.response);

  if (state->last_response_update_ms == 0) {
    // no time span to use yet
    state->velocity = r->max_velocity;
    return 0;
  }

  state->limiter = find_limiter(settings, state);

  if (state->limiter == NO_LIMIT) {
    if (state->velocity > 0) {
      // ok, but nothing is maximized.  example 10 -> 11
      state->velocity += r->acceleration;
      if (state->velocity > r->max_velocity) {
        state->velocity = r->max_velocity;
      }
    } else {
      // going negative when we are ok, reverse.  example -10 -> 9
      state->velocity = -state->velocity - r->deceleration;
      if (state->velocity < r->min_velocity) {
        state->velocity = r->min_velocity;
      }
    }
  } else {
    if (state->velocity > 0) {
      // too much. example: 10 -> -9
      state->velocity = -state->velocity + r->deceleration;
      if (state->velocity > -r->min_velocity) {
        state->velocity = -r->min_velocity;
      }
    } else {
      // have not gotten in back to an ok state yet. example -10 -> -11
      state->velocity -= r->acceleration;
      if (state->velocity < -r->max_velocity) {
        state->velocity = -r->max_velocity;
      }
    }
  }
  
  // now to actually add a chunk of velocity to the level
  // first, velocity is for a second of time.  Need to cut it down
  // to cover the actual time span
  const float time_span_ms = (float)(
    state->uptime_ms - state->last_response_update_ms);
  float scaled_v = state->velocity * time_span_ms / 1000.0;
  // scaled v is on a 0-100 scale and needs to be on a 0-65535 scale
  scaled_v = scaled_v * 65535.0 / 100.0;
  const float float_vgs_level = (float)(state->vgs_level) + scaled_v;
  if (float_vgs_level <= 0.0) {
    return 0;
  }
  if (float_vgs_level >= 65535.0) {
    return 65535;
  }
  return (uint16_t)(float_vgs_level);
}

void vgs_control(
  const struct Settings* settings, struct SharedState* state) {
  const uint16_t old_level = state->vgs_level;
  state->vgs_level = calc_new_level(settings, state);
  if (old_level != state->vgs_level) {
    set_level(state);
  }
  state->last_response_update_ms = state->uptime_ms;
}