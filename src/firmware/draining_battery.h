#ifndef DRAINING_BATTERY_H
#define DRAINING_BATTERY_H

#include "settings.h"
#include "state.h"

void draining_battery(
    const struct Settings* settings,
    struct SharedState* state,
    uint16_t current_mv);

#endif

