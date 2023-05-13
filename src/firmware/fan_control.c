#include "fan_control.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define FAN_GPIO 10
// These map to FAN_GPIO via the hardware spec.
// there are also pwm_gpio_to_pwm_slice and pwm_gpio_to_channel
// functions available but that create uneeded runtime variables
// for values that are static
#define FAN_PWM_SLICE 5
#define FAN_CHAN 0

// how often the fan speed is allowed to change
#define MIN_FAN_CHANGE_MS 1000
// how much the fan is allowed to change (0-65535)
#define MAX_FAN_CHANGE 6554

#define MAX_FREQ 125000000
#define FREQ 25000
#define WRAP (MAX_FREQ / FREQ)

// duty cycle ranges from 0-65535
static inline uint16_t calc_level(uint16_t duty_cycle) {
  return (WRAP * (uint32_t)duty_cycle) / 65535;
}

void fan_control_init(void) {
  gpio_set_function(FAN_GPIO, GPIO_FUNC_PWM);
  pwm_set_enabled(FAN_PWM_SLICE, 0);
  pwm_set_wrap(FAN_PWM_SLICE, WRAP);
}

static void set_pwm(const struct SharedState* state) {
  pwm_set_enabled(FAN_PWM_SLICE, 0);
  if (state->fan_level == 0) {
    return;
  }
  pwm_set_chan_level(
    FAN_PWM_SLICE, FAN_CHAN, calc_level(state->fan_level));
  pwm_set_enabled(FAN_PWM_SLICE, 1);
}

static uint16_t calc_new_level_by_temp(
  const struct Settings* settings, const struct SharedState* state) {
  const struct FanSettings* f = &(settings->global.fan);
  if (state->temperature_c < f->min_celsius) {
    return 0;
  }
  if (f->min_celsius >= f->max_celsius) {
    // this would create match errors below
    return 65535;
  }
  const uint32_t percent_base = (uint32_t)f->min_percent * 65535 / 100;
  const uint32_t percent_range = 65535 - percent_base;
  const uint32_t percent_add = (
    percent_range *
    (uint32_t)(state->temperature_c - f->min_celsius) /
    (f->max_celsius - f->min_celsius));
  return (uint16_t)(percent_base + percent_add);
}

static uint16_t calc_new_level_by_power(
  const struct Settings* settings, const struct SharedState* state) {
  const struct FanSettings* f = &(settings->global.fan);
  const uint32_t power_watts = state->current_ma * state->loaded_mv / 1000000;
  if (power_watts < f->min_watts) {
    return 0;
  }
  if (f->min_watts >= f->max_watts) {
    // this would create match errors below
    return 65535;
  }
  const uint32_t percent_base = (uint32_t)f->min_percent * 65535 / 100;
  const uint32_t percent_range = 65535 - percent_base;
  const uint32_t percent_add = (
    percent_range *
    (uint32_t)(power_watts - f->min_watts) /
    (f->max_watts - f->min_watts));
  return (uint16_t)(percent_base + percent_add);
}

static inline int32_t calc_new_level(
  const struct Settings* settings, const struct SharedState* state) {
    const uint16_t temp_level = calc_new_level_by_temp(settings, state);
    const uint16_t power_level = calc_new_level_by_power(settings, state);
    return temp_level > power_level ? temp_level : power_level;
}

void fan_control(
  const struct Settings* settings, struct SharedState* state) {
  if (state->uptime_ms < state->next_fan_change_ms) {
    return;
  }
  state->next_fan_change_ms = state->uptime_ms + MIN_FAN_CHANGE_MS;

  const int32_t old_level = state->fan_level;
  int32_t level = calc_new_level(settings, state); 
  int32_t difference = level - old_level;

  if (difference > 0) {
    if (difference > MAX_FAN_CHANGE) {
      difference = MAX_FAN_CHANGE;
    }
  } else {
    if (difference < -MAX_FAN_CHANGE) {
      difference = -MAX_FAN_CHANGE;
    }
  }
  level += difference;

  if (level <= settings->global.fan.min_percent) {
    level = 0;
  } else if (level >= 0xFFFF) {
    level = 0xFFFF;
  }

  state->fan_level = level;
  set_pwm(state);
}