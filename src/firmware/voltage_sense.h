#ifndef VOLTAGE_SENSE_H
#define VOLTAGE_SENSE_H

#include "settings.h"
#include "state.h"

void voltage_sense_init(void);

void voltage_sense_update(const struct Settings* settings, struct SharedState* state);

#endif
