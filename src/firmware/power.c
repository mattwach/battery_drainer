#include "power.h"
#include "pico/stdlib.h"

#define ENABLE_GPIO 21

void power_hold(void) {
  gpio_init(ENABLE_GPIO);
  gpio_set_dir(ENABLE_GPIO, GPIO_OUT);
  gpio_put(ENABLE_GPIO, 1);
}

void power_off(void) {
  gpio_put(ENABLE_GPIO, 0);
}

