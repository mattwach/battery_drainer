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
  float finish_display_ratio;
};

struct PerCellSettings {
  uint16_t target_mv;
  uint16_t damage_mv;
  uint16_t max_vsag_mv;
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

#endif

