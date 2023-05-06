#ifndef TEMPERATURE_SENSE_H
#define TEMPERATURE_SENSE_H

#include "settings.h"
#include "state.h"

void temperature_sense_init(void);

void temperature_sense_update(
  const struct Settings* settings, struct SharedState* state);

#endif
