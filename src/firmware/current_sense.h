#ifndef CURRENT_SENSE_H
#define CURRENT_SENSE_H

#include "settings.h"
#include "state.h"

void current_sense_init(void);

void current_sense_update(
  const struct Settings* settings, struct SharedState* state);

#endif