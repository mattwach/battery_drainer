#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uart_console/console.h"
#include "hardware/pwm.h"

#define FAN_GPIO 10
// These map to FAN_GPIO via the hardware spec.
// there are also pwm_gpio_to_pwm_slice and pwm_gpio_to_channel
// functions available but that create uneeded runtime variables
// for values that are static
#define FAN_PWM_SLICE 5
#define FAN_CHAN 0

#define DIVIDER 10
#define MAX_FREQ 125000000
#define MAX_PWM_FREQ (MAX_FREQ / DIVIDER)
#define FREQ 25000
#define WRAP (MAX_PWM_FREQ / FREQ)

uint8_t update_status_flag = 1;
uint16_t fan_duty_cycle = 10;

static uint16_t calc_level(uint16_t duty_cycle) {
  return (WRAP * (uint32_t)duty_cycle) / 100;
}

static void update_pwm(void) {
  pwm_set_enabled(FAN_PWM_SLICE, 0);
  pwm_set_chan_level(FAN_PWM_SLICE, FAN_CHAN, calc_level(fan_duty_cycle));
  pwm_set_enabled(FAN_PWM_SLICE, 1);
  update_status_flag = 1;
}

static uint8_t get_num(const char* name, const char* vstr, int min, int max, uint16_t* val) {
  const int v = atoi(vstr);
  if (v == 0 && strcmp(vstr, "0")) {
    printf("Syntax error parsing %s\n", name);
    return 0;
  }

  if (v < min) {
    printf("%s is below the minimum of %d\n", name, min);
    return 0;
  }

  if (v > max) {
    printf("%s is above the maximum of %d\n", name, max);
    return 0;
  }

  *val = (uint32_t)v;
  return 1;
}

static void duty_cycle_cmd(uint8_t argc, char* argv[]) {
  if (get_num("fan_duty_cycle", argv[0], 0, 100, &fan_duty_cycle)) {
    update_pwm();
  }
}

static void init_pwm(void) {
  gpio_set_function(FAN_GPIO, GPIO_FUNC_PWM);
  pwm_set_clkdiv_int_frac(FAN_PWM_SLICE, DIVIDER, 0);
  pwm_set_wrap(FAN_PWM_SLICE, WRAP);
  update_pwm();
}

static void update_status() {
  printf("freq = %u Hz, fan_duty_cycle = %u%%", FREQ, fan_duty_cycle);
  update_status_flag = 0;
}

struct ConsoleCallback callbacks[] = {
    {"duty_cycle", "Sets duty_cycle <0-100>", 1, duty_cycle_cmd},
};

static void enable() {
    const uint ENABLE_PIN = 21;
    gpio_init(ENABLE_PIN);
    gpio_set_dir(ENABLE_PIN, GPIO_OUT);
    gpio_put(ENABLE_PIN, 1);
}

// program entry point
int main() {
  enable();
  struct ConsoleConfig cc;
  uart_console_init(&cc, callbacks, 1, CONSOLE_VT102);
  init_pwm();

  while (1) {
    if (update_status_flag) {
      update_status();
    }
    uart_console_poll(&cc, "> ");
    sleep_ms(20);
  }
  return 0;
}
