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

void vgs_control(uint16_t level) {
  pwm_set_enabled(PWM_SLICE, 0);
  if (level == 0) {
    return;
  }
  // note that values >= 32768 are capped at full-on within calc_level
  pwm_set_chan_level(PWM_SLICE, SLOW_CHAN, calc_level(level));
  if (level <= 32768) {
    pwm_set_chan_level(PWM_SLICE, FAST_CHAN, 0);
  } else {
    pwm_set_chan_level(PWM_SLICE, FAST_CHAN, calc_level(level - 32768));
  }
  pwm_set_enabled(PWM_SLICE, 1);
}
