#include "damage_warning.h"
#include "util.h"

static const char* message[] = {
// 0123456789012345  
  "Batteries may be",
  "damaged!",
  "    Abort",
  "    Proceed",
};

static uint8_t text_options(struct SharedState* state, int i) {
  if ((i == 2) && (!state->damage_confirm)) {
    return TEXT_OPTION_INVERTED;
  }
  if ((i == 3) && state->damage_confirm) {
    return TEXT_OPTION_INVERTED;
  }
  return 0x00;
}

static void check_buttons(struct SharedState* state) {
  if (state->button & NEXT_PRESSED) {
    state->damage_confirm = 1 - state->damage_confirm;
  }
  if (state->button & OK_PRESSED) {
    if (state->damage_confirm) {
      state_change(state, DRAINING_BATTERY);
    } else {
      state_change(state, PROFILE_SELECTION);
    }
  }
}

void damage_warning(struct SharedState* state) {
  char line[OLED_COLUMNS];
  struct Text* text = &(state->text);
  for (int i=0; i<4; ++i) {
    text->row = i * 2;
    text->column = 0;
    text->options = text_options(state, i);
    fill_line(line, message[i]);
    text_strLen(text, line, OLED_COLUMNS);
  }
  text->options = 0x00;

  check_buttons(state);
}
