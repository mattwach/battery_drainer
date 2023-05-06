#include "settings_message.h"
#include "util.h"

static const char* message[] = {
  "",
  "Please",
  "connect USB at",
  "115200 Baud",
};


void settings_message(struct SharedState* state) {
  char line[OLED_COLUMNS];
  struct Text* text = &(state->text);
  for (int i=0; i<4; ++i) {
    text->row = i * 2;
    text->column = 0;
    fill_line(line, message[i]);
    text_strLen(text, line, OLED_COLUMNS);
  }

  if (state->button) {
    state_change(state, PROFILE_SELECTION);
  }
}


