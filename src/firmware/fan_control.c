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

void fan_control(uint16_t level) {
  pwm_set_enabled(FAN_PWM_SLICE, 0);
  if (level == 0) {
    return;
  }
  pwm_set_chan_level(FAN_PWM_SLICE, FAN_CHAN, calc_level(level));
  pwm_set_enabled(FAN_PWM_SLICE, 1);
}
