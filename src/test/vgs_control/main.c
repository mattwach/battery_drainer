#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uart_console/console.h"
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

#define DIVIDER 1
#define MAX_FREQ 125000000
#define MAX_PWM_FREQ (MAX_FREQ / DIVIDER)

uint8_t update_status_flag = 1;
uint32_t freq = 32768;
uint16_t fast_duty_cycle = 0;
uint16_t slow_duty_cycle = 0;

static uint16_t calc_level(uint16_t duty_cycle, uint16_t wrap) {
  return ((uint32_t)(wrap) * (uint32_t)duty_cycle) / 10000;
}

static void update_pwm(void) {
  pwm_set_enabled(PWM_SLICE, 0);
  const uint16_t wrap = MAX_PWM_FREQ / freq;
  pwm_set_wrap(PWM_SLICE, wrap);
  pwm_set_chan_level(PWM_SLICE, FAST_CHAN, calc_level(fast_duty_cycle, wrap));
  pwm_set_chan_level(PWM_SLICE, SLOW_CHAN, calc_level(slow_duty_cycle, wrap));
  pwm_set_enabled(PWM_SLICE, 1);
  update_status_flag = 1;
}

static uint8_t get_num(const char* name, const char* vstr, int min, int max, uint32_t* val) {
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
  uint32_t fast = 0;
  if (!get_num("fast_duty_cycle", argv[0], 0, 10000, &fast)) {
    return;
  }
  uint32_t slow = 0;
  if (!get_num("slow_duty_cycle", argv[1], 0, 10000, &slow)) {
    return;
  }
  fast_duty_cycle = fast;
  slow_duty_cycle = slow;
  update_pwm();
}

static void freq_cmd(uint8_t argc, char* argv[]) {
  if (!get_num("frequency", argv[0], 0, 100000, &freq)) {
    return;
  }
  update_pwm();
}


static void init_pwm(void) {
  gpio_set_function(FAST_GPIO, GPIO_FUNC_PWM);
  gpio_set_function(SLOW_GPIO, GPIO_FUNC_PWM);
  pwm_set_clkdiv_int_frac(PWM_SLICE, DIVIDER, 0);
  update_pwm();
}

static void update_status() {
  printf("freq = %u Hz, fast_duty_cycle = %u / 10000, slow_duty_cycle = %u / 10000\n",
         freq, fast_duty_cycle, slow_duty_cycle);
  update_status_flag = 0;
}

static void enable() {
    const uint ENABLE_PIN = 21;
    gpio_init(ENABLE_PIN);
    gpio_set_dir(ENABLE_PIN, GPIO_OUT);
    gpio_put(ENABLE_PIN, 1);
}

struct ConsoleCallback callbacks[] = {
    {"duty_cycle", "Sets duty_cycle slow and fast, <0-10000> <0-10000>", 2, duty_cycle_cmd},
    {"freq", "Sets frequency in hz, <100-100000>", 1, freq_cmd},
};

// program entry point
int main() {
  enable();
  struct ConsoleConfig cc;
  uart_console_init(&cc, callbacks, 2, CONSOLE_VT102);
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
