#include "state.h"
#include "vgs_control.h"
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

void state_change(struct SharedState* ss, enum State s) {
  ss->state_started_ms = 0;
  ss->state = s;
  ss->damage_confirm = 0;
  ss->last_charge_sample_ms = 0;
  ss->charge_mas = 0;
}
