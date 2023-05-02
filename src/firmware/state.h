#ifndef BATTD_STATE_H
#define BATTD_STATE_H

#include <inttypes.h>
#include <oledm/oledm.h>
#include <oledm/text.h>

enum State {
  PROFILE_SELECTION,
  SETTINGS_MESSAGE,
  DETROY_BATTERY_CONFIRMATION,
  DRAINING_BATTERY,
  FINISHED,
};

struct SharedState {
  enum State state;
  uint8_t active_profile_index;  // if == the profile count, then we are on the Settings option
  struct OLEDM display;
  struct Text text;
};

void state_init(struct SharedState* ss);

#endif

