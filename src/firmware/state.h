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
};

void state_init(struct SharedState* ss);

#endif

