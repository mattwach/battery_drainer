#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uart_console/console.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define VSENSE_GPIO 26
#define ADC_BASE_GPIO 26

#define POLL_MS 1000

int16_t vdrop_mv = 0;
int16_t vmax_mv = 3300;
uint8_t started = 1;

static inline uint32_t uptime_ms() {
  return to_ms_since_boot(get_absolute_time());
}

static uint8_t get_num(const char* name, const char* vstr, int min, int max, int16_t* val) {
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
    printf("%s is above the maximum of %d\n", name, min);
    return 0;
  }

  *val = (uint16_t)v;
  return 1;
}

static void start_stop_cmd(uint8_t argc, char* argv[]) {
  started = 1 - started;
  printf(started ? "started\n" : "stopped\n");
}

static void vmax_mv_cmd(uint8_t argc, char* argv[]) {
  get_num("vmax_mv", argv[0], 1000, 30000, &vmax_mv);
}

static void vdrop_mv_cmd(uint8_t argc, char* argv[]) {
  get_num("vdrop_mv", argv[0], -1000, 5000, &vdrop_mv);
}

struct ConsoleCallback callbacks[] = {
    {"s", "Start/Stop adc reads", 0, start_stop_cmd},
    {"vmax_mv", "Sets maximum voltage (accounts for voltage division)\n", 1, vmax_mv_cmd},
    {"vdrop_mv", "Sets voltage drop (accounts for inline diodes/etc)\n", 1, vdrop_mv_cmd},
};

static void init_adc(void) {
  adc_init();
  adc_gpio_init(VSENSE_GPIO);
  adc_select_input(VSENSE_GPIO - ADC_BASE_GPIO);
}

static void read_adc(void) {
  const uint16_t val = adc_read();
  const uint32_t mv = ((uint32_t)val * vmax_mv / (1 << 12)) + vdrop_mv;
  printf("val: %u, vmax_mv: %u, vdrop_mv: %d, mv: %d\n",
      val, vmax_mv, vdrop_mv, mv);
}

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
  uart_console_init(&cc, callbacks, 3, CONSOLE_VT102);
  init_adc();

  uint32_t next_read_ms = 0;

  while (1) {
    const uint32_t current_ms = uptime_ms();
    if (started && (current_ms >= next_read_ms)) {
      read_adc();
      next_read_ms = current_ms + POLL_MS;
    }
    uart_console_poll(&cc, "> ");
    sleep_ms(20);
  }
  return 0;
}
