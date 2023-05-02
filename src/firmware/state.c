#include "state.h"

#include <string.h>

void state_init(struct SharedState* ss) {
  memset(ss, 0, sizeof(*ss));
  ss->state = PROFILE_SELECTION;
}

