// LIPO Battery Drainer
//
// C 2023: Matt Wachowski

#include "buttons.h"
#include "console.h"
#include "draining_battery.h"
#include "power.h"
#include "profile_selection.h"
#include "settings.h"
#include "settings_message.h"
#include "state.h"

#include "pico/stdlib.h"

#define LOOP_SLEEP_TIME 20
#define LED_PIN PICO_DEFAULT_LED_PIN

struct Settings settings;
struct SharedState state;

static void init(void) {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);
  settings_load(&settings);
  console_init(&settings);
  state_init(&state);
  buttons_init();
}

static void loop(void) {
  // TODO: read these from the hardware later
  const uint16_t vcal_adc_reading = 3426;
  const uint16_t current_mv = 25100;
  buttons_update(&state);
  if (state.state != DRAINING_BATTERY) {
    console_poll(vcal_adc_reading);
  }
  switch (state.state) {
    case PROFILE_SELECTION:
      profile_selection(&settings, &state, current_mv);
      break;
    case SETTINGS_MESSAGE:
      settings_message(&state);
      break;
    case DRAINING_BATTERY:
      draining_battery(&settings, &state, current_mv);
      break;
    default:
      // if we are here, then the state is not yet implemented
      break;
  }
}

int main() {
  power_hold();
  init();
  uint32_t frame_idx = 0;
  while (1) {
    loop();
    sleep_ms(LOOP_SLEEP_TIME);
    gpio_put(LED_PIN, frame_idx & 0x10 ? 1 : 0);
    ++frame_idx;
  }
  return 0;
}

