#ifndef DRAINING_BATTERY_UI_H
#define DRAINING_BATTERY_UI_H

#include "state.h"

enum Limiter {
  NONE,
  VOLTAGE_SAG,
  CURRENT_LIMIT,
  POWER_LIMIT,
  TEMPERATURE_LIMIT,
  FINISHED_LIMIT,
};

struct DrainingBatteryUIFields {
  uint16_t time_seconds; 
  uint16_t charge_mah;
  uint8_t cells;
  uint16_t current_mv;

  // only used while in-progress
  uint16_t target_mv;

  // only used when finished
  uint16_t finish_seconds;

  uint16_t current_ma;
  uint16_t power_watts;
  uint8_t temp_c;
  uint8_t fet_percent;
  uint8_t fan_percent;

  enum Limiter limiter;
};

void draining_battery_ui_render(
    struct Text* text, const struct DrainingBatteryUIFields* fields);

#endif

