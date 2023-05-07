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

static uint16_t calc_new_level(
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
    (state->temperature_c - f->min_celsius) /
    (f->max_celsius - f->min_celsius));
  return (uint16_t)(percent_base + percent_add);
}

void fan_control(
  const struct Settings* settings, struct SharedState* state) {
  const uint16_t old_level = state->fan_level;
  state->fan_level = calc_new_level(settings, state);
  if (old_level != state->fan_level) {
    set_pwm(state);
  }
}