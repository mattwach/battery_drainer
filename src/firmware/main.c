// LIPO Battery Drainer
//
// C 2023: Matt Wachowski

#include "buttons.h"
#include "console.h"
#include "settings.h"
#include "state.h"
#include "pico/stdlib.h"

#define LOOP_SLEEP_TIME 20

struct Settings settings;
struct SharedState state;

static void init(void) {
  settings_load(&settings);
  console_init(&settings);
  state_init(&state);
  buttons_init();
}

static void loop(void) {
  buttons_update(&state);
  if (state.state != DRAINING_BATTERY) {
    console_poll();
  }
  switch (state.state) {
    case PROFILE_SELECTION:
      //profile_selection(&settings, &state);
      break;
    default:
      // if we are here, then the state is not yet implemented
      break;
  }
}

int main() {
  init();
  while (1) {
    loop();
    sleep_ms(LOOP_SLEEP_TIME);
  }
  return 0;
}

