#include "temperature_sense.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define TEMP_SENSE_GPIO 28
#define ADC_BASE_GPIO 26

#define ZERO_C_MV 400
#define TEMP_UV_PER_C 19500

void temperature_sense_init(void) {
  adc_gpio_init(TEMP_SENSE_GPIO);
}

void temperature_sense_update(
  const struct Settings* settings, struct SharedState* state) {
  adc_select_input(TEMP_SENSE_GPIO - ADC_BASE_GPIO);
  const int32_t adc_val = adc_read();
  // adc_val ranges from 0-4096 and represents a 0-3000 mv range.
  const int32_t adc_mv = (adc_val * 3000) / 4096;
  // Apply the datasheet-specified 0C offset
  int32_t shifted_uv = (adc_mv - ZERO_C_MV) * 1000;
  if (shifted_uv < 0) {
    // should not happen as it indicates < 0 C conditions
    // but should not != can not
    shifted_uv = 0;
  }
  // Apply the datasheet-specified scaling factor
  state->temperature_c = (uint8_t)(shifted_uv / TEMP_UV_PER_C);
}
