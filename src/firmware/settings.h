#ifndef SETTINGS_H
#define SETTINGS_H

#include <inttypes.h>

struct FanSettings {
  uint8_t min_percent;
  uint8_t min_celsius;
  uint8_t max_celsius;
};

struct VSagSettings {
  uint8_t interval_seconds;
  uint16_t settle_ms;
};

struct SlewSettings {
  float volts;
  float amps;
  float celsius;
};

struct GlobalSettings {
  uint16_t ical_mohms;
  float vcal_ratio;
  struct VSagSettings vsag;
  struct FanSettings fan;
  struct SlewSettings slew;
  float finish_display;
};

struct PerCellSettings {
  uint16_t target_mv;
  uint16_t max_vsag_mv;
  uint8_t damage_warning;
};

struct ProfileSettings {
  char name[20];
  uint16_t drop_mv;
  uint8_t cell_count;  // zero is auto
  struct PerCellSettings cell;
  uint16_t max_ma;
  uint8_t max_celsius;
  uint16_t max_watts;
};

#define MAX_PROFILE_COUNT 16

struct Settings {
  uint16_t checksum;
  char eyecatcher[4];  // set to BATD
  uint8_t profile_count;
  struct GlobalSettings global;
  struct ProfileSettings profile[MAX_PROFILE_COUNT];
};

// default settings, usually, load is preferred.
void settings_default(struct Settings* settings);

void settings_load(struct Settings* settings);

// checksum will be updated before save
void settings_save(struct Settings* settings);

void settings_try_add_profile(struct Settings* settings);

void settings_try_duplicate_profile(struct Settings* settings, uint8_t source_idx);

void settings_try_delete_profile(struct Settings* settings, uint8_t index);

void settings_try_move_profile(struct Settings* settings, uint8_t src_idx, uint8_t dest_idx);

// calculate the cell count and target voltage for a given
// input voltage
//
// If the numbers can not be determined, then both
// slots ar set to zero.
//
// Giving too high a profile index is acceptable, but
// both results will be set to zero.
void settings_calc_voltage_and_cell_count(
    const struct Settings* settings,
    int profile_index,
    uint16_t current_mv,
    uint8_t* cell_count,
    uint16_t* target_mv);

#endif

