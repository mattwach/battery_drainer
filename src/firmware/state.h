#ifndef BATTD_STATE_H
#define BATTD_STATE_H

#include <inttypes.h>

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
};

void state_init(struct SharedState* ss);

#endif

