#ifndef BATT_CONSOLE_H
#define BATT_CONSOLE_H

#include "settings.h"
#include "state.h"

void console_init(struct Settings* s, struct SharedState* ss);

void console_poll(uint16_t vcal_adc_reading);

#endif

