// LIPO Battery Drainer
//
// C 2023: Matt Wachowski

#include "buttons.h"
#include "console.h"
#include "current_sense.h"
#include "finished.h"
#include "damage_warning.h"
#include "draining_battery.h"
#include "fan_control.h"
#include "power.h"
#include "profile_selection.h"
#include "settings.h"
#include "settings_message.h"
#include "state.h"
#include "temperature_sense.h"
#include "util.h"
#include "vgs_control.h"
#include "voltage_sense.h"

#include "pico/stdlib.h"
#include "hardware/adc.h"

#define LOOP_SLEEP_TIME_MS 20
#define LED_PIN PICO_DEFAULT_LED_PIN

struct Settings settings;
struct SharedState state;

static inline uint32_t uptime_ms() {
  return to_ms_since_boot(get_absolute_time());
}

static void init(void) {
  sleep_ms(50);  // Allow OLED voltage to settle
  power_hold();
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);
  settings_load(&settings);
  console_init(&settings);
  state_init(&state);
  buttons_init();
  vgs_control_init();
  fan_control_init();
  adc_init();
  voltage_sense_init();  // call adc_init() first
  current_sense_init();  // call adc_init() first
  temperature_sense_init();  // call adc_init() first
}

static void loop(void) {
  voltage_sense_update(&settings, &state);
  current_sense_update(&settings, &state);
  temperature_sense_update(&settings, &state);
  fan_control(&settings, &state);
  vgs_control(&settings, &state);
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
    case DAMAGE_WARNING:
      damage_warning(&state);
      break;
    case DRAINING_BATTERY:
      draining_battery(&settings, &state);
      break;
    case FINISHED:
      finished(&state, settings.global.finish_display);
      break;
    default:
      // if we are here, then the state is not yet implemented
      break;
  }
}

int main() {
  init();
  uint32_t frame_idx = 0;
  while (1) {
    state.uptime_ms = uptime_ms();
    loop();
    gpio_put(LED_PIN, frame_idx & 0x10 ? 1 : 0);
    ++frame_idx;
    const uint32_t tdelta = uptime_ms() - state.uptime_ms;
    if (tdelta < LOOP_SLEEP_TIME_MS) {
      sleep_ms(LOOP_SLEEP_TIME_MS - tdelta);
    } else {
      sleep_ms(1);
    }
  }
  return 0;
}

