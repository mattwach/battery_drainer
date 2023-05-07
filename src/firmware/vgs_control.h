#ifndef VGS_CONTROL_H
#define VGS_CONTROL_H

#include "settings.h"
#include "state.h"

void vgs_control_init(void);

void vgs_control(
  const struct Settings* settings, struct SharedState* state);

#endif
