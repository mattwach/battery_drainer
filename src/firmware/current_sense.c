#include "current_sense.h"
#include "uint16_avg.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define CSENSE_GPIO 27
#define ADC_BASE_GPIO 26

void current_sense_init(void) {
  adc_gpio_init(CSENSE_GPIO);
}

void current_sense_update(
  const struct Settings* settings, struct SharedState* state) {
  adc_select_input(CSENSE_GPIO - ADC_BASE_GPIO);
  const uint32_t adc_val = adc_read();
  // adc_val ranges from 0-4096 and represents a 0-3000 mv range.
  const uint32_t adc_mv = (adc_val * 3000) / 4096;
  // to convert to ma, we need to use I = V/R
  // example:
  //    adc_mv = 1500 mv
  //    r = 133 mOhms
  //    adc_mv / r = 1500 / 133 mv/mOhms = 11.278 A
  //    11.278 A * 1000 = 11278 mA
  uint16_avg_add(
    &(state->avg_ma), (adc_mv * 1000) / settings->global.ical_mohms);
}