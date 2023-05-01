// LIPO Battery Drainer
//
// C 2023: Matt Wachowski

#include "console.h"
#include "settings.h"
#include "pico/stdlib.h"

#define LOOP_SLEEP_TIME 20

struct Settings settings;

static void init(void) {
  settings_load(&settings);
  console_init(&settings);
}

static void loop(void) {
  console_poll();
}

int main() {
  init();
  while (1) {
    loop();
    sleep_ms(LOOP_SLEEP_TIME);
  }
  return 0;
}

