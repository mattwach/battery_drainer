#ifndef BATTD_STATE_H
#define BATTD_STATE_H

#include <inttypes.h>
#include <oledm/oledm.h>
#include <oledm/text.h>

enum State {
  PROFILE_SELECTION,
  SETTINGS_MESSAGE,
  BAD_SETUP_MESSAGE,
  DAMAGE_WARNING,
  DRAINING_BATTERY,
  FINISHED,
};

#define NEXT_PRESSED 0x01
#define OK_PRESSED 0x02

#define OLED_ROWS 4
#define OLED_COLUMNS 16

enum Limiter {
  NO_LIMIT,
  VOLTAGE_SAG_LIMIT,
  CURRENT_LIMIT,
  POWER_LIMIT,
  TEMPERATURE_LIMIT,
};

struct MaxValues {
  uint16_t time_seconds;
  uint16_t charge_mah;
  uint16_t current_ma;
  uint16_t power_watts;
  uint8_t temperature_c;
  uint8_t fet_percent;
  uint8_t fan_percent;
};

struct SharedState {
  // Dont change directly so that state_started_ms can be reset
  enum State state;
  struct OLEDM display;
  struct Text text;
  uint8_t active_profile_index;  // if == the profile count, then we are on the Settings option
  uint8_t button;  // NEXT_PRESSED | OK_PRESSED

  uint32_t uptime_ms;
  uint32_t state_started_ms;
  uint8_t damage_confirm;

  // use for calibration
  uint16_t vcal_adc_reading;
  // This is the measured mv + the offset.  The sag value
  // needs to be added to estimate the unloaded voltage
  uint16_t loaded_mv;
  uint16_t last_unloaded_sample_mv;

  uint16_t current_ma;

  // mah would not allow for incremental accumulation so we use
  // mas (milliamp seconds) instead.
  uint32_t last_charge_sample_ms;
  uint32_t charge_mas;

  uint8_t temperature_c;

  // calculated at profile selection
  uint8_t cells;
  uint16_t target_mv;

  // ranges 0-65536
  uint16_t vgs_level;
  uint16_t fan_level;

  // response state
  float velocity;
  uint32_t last_response_update_ms;

  // what is limiting vgs output
  enum Limiter limiter;

  // Used for finish screen
  struct MaxValues max_values;
};

void state_init(struct SharedState* ss);

void state_change(struct SharedState* ss, enum State s);

#endif

