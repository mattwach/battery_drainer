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
#include "voltage_sense.h"

#include "pico/stdlib.h"
#include "hardware/adc.h"

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
  adc_init();
  voltage_sense_init();  // call adc_init() first
}

static void loop(void) {
  voltage_sense_update(&settings, &state);
  buttons_update(&state);
  if (state.state != DRAINING_BATTERY) {
    console_poll(state.vcal_adc_reading);
  }
  switch (state.state) {
    case PROFILE_SELECTION:
      profile_selection(&settings, &state);
      break;
    case SETTINGS_MESSAGE:
      settings_message(&state);
      break;
    case DRAINING_BATTERY:
      draining_battery(&settings, &state);
      break;
    default:
      // if we are here, then the state is not yet implemented
      break;
  }
}

int main() {
  sleep_ms(50);  // Allow OLED voltage to settle
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

