#ifndef BATTD_STATE_H
#define BATTD_STATE_H

#include <inttypes.h>
#include <oledm/oledm.h>
#include <oledm/text.h>

enum State {
  PROFILE_SELECTION,
  SETTINGS_MESSAGE,
  BAD_SETUP_MESSAGE,
  DETROY_BATTERY_CONFIRMATION,
  DRAINING_BATTERY,
  FINISHED,
};

#define NEXT_PRESSED 0x01
#define OK_PRESSED 0x02

#define OLED_ROWS 4
#define OLED_COLUMNS 16

struct SharedState {
  enum State state;
  struct OLEDM display;
  struct Text text;
  uint8_t active_profile_index;  // if == the profile count, then we are on the Settings option
  uint8_t button;  // NEXT_PRESSED | OK_PRESSED

  uint32_t uptime_ms;

  // use for calibration
  uint16_t vcal_adc_reading;
  // This is the measured mv + the offset.  The sag value
  // needs to be added to estimate the unloaded voltage
  uint16_t loaded_mv;
  uint16_t estimated_sag_mv;
};

static inline uint16_t estimate_unloaded_mv(
  const struct SharedState* state) {
  return state->loaded_mv + state->estimated_sag_mv;
}

void state_init(struct SharedState* ss);

#endif

