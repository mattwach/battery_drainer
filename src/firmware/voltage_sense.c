#include "voltage_sense.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define VSENSE_GPIO 26
#define ADC_BASE_GPIO 26

void voltage_sense_init(void) {
  adc_gpio_init(VSENSE_GPIO);
}

void voltage_sense_update(const struct Settings* settings, struct SharedState* state) {
  if (state->fake_mv) {
    state->loaded_mv = state->fake_mv;
    return;
  }
  adc_select_input(VSENSE_GPIO - ADC_BASE_GPIO);
  state->vcal_adc_reading = adc_read();
  const uint16_t vcal_mv = (uint16_t)((float)state->vcal_adc_reading * settings->global.vcal_ratio);
  // TODO: Add vsag calibration logic
  state->loaded_mv =
      vcal_mv +
      settings->profile[state->active_profile_index].drop_mv;
}