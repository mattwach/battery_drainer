#include "voltage_sense.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define VSENSE_GPIO 26
#define ADC_BASE_GPIO 26

void voltage_sense_init(void) {
  adc_gpio_init(VSENSE_GPIO);
}

static uint16_t voltage_sense(const struct Settings* settings, struct SharedState* state) {
  if (state->fake_mv) {
    state->vcal_adc_reading = state->fake_mv / settings->global.vcal_ratio;
    return state->fake_mv;
  }
  adc_select_input(VSENSE_GPIO - ADC_BASE_GPIO);
  state->vcal_adc_reading = adc_read();
  const uint16_t vcal_mv = (uint16_t)((float)state->vcal_adc_reading * settings->global.vcal_ratio);
  return vcal_mv + settings->profile[state->active_profile_index].drop_mv;
}

void voltage_sense_update(const struct Settings* settings, struct SharedState* state) {
  if (state->state != DRAINING_BATTERY) {
    state->loaded_mv = voltage_sense(settings, state);
    state->last_unloaded_sample_mv = state->loaded_mv;
    state->last_voltage_sample_ms = state->uptime_ms;
    state->is_sampling_voltage = 0;
    return;
  }

  const uint32_t delta_ms = state->uptime_ms - state->last_voltage_sample_ms;

  if (state->is_sampling_voltage) {
    // wait for the settle time before making a measurement
    if (delta_ms >= settings->global.vsag.settle_ms) {
      state->last_unloaded_sample_mv = voltage_sense(settings, state);
      state->is_sampling_voltage = 0;
      state->last_voltage_sample_ms = state->uptime_ms;  // needed to delay loaded_mv
    }
    return;
  }

  if (delta_ms < settings->global.vsag.settle_ms) {
    // don't update loaded_mv yet because the load time
    // might be too short for a reliable measurement
    return;
  }

  if (delta_ms >= ((uint32_t)settings->global.vsag.interval_seconds * 1000)) {
    // time to enter the unloaded calculation phase
    state->is_sampling_voltage = 1;
    state->last_voltage_sample_ms = state->uptime_ms;
    return;
  }

  state->loaded_mv = voltage_sense(settings, state);
}
