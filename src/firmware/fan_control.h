#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include "settings.h"
#include "state.h"

void fan_control_init(void);

void fan_control(
  const struct Settings* settings, struct SharedState* state);

#endif
