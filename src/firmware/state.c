#include "state.h"
#include <oledm/font/terminus8x16.h>
#include <string.h>

void state_init(struct SharedState* ss) {
  memset(ss, 0, sizeof(*ss));
  ss->state = PROFILE_SELECTION;

  struct OLEDM* display = &ss->display;
  oledm_basic_init(display);
  oledm_start(display);
  oledm_clear(display, 0x00);

  text_init(&ss->text, terminus8x16, display);
}

