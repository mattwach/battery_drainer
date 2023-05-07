#include "message.h"
#include "util.h"

static const char* settings_message[] = {
// 0123456789012345
  "",
  "Please",
  "connect USB at",
  "115200 Baud",
};

static const char* condition_met_message[] = {
// 0123456789012345
  "Battery is",
  "already at",
  "target",
  "voltage",
};

static const char* zero_cells_message[] = {
// 0123456789012345
  "",
  "Cell count",
  "is zero",
  "",
};


void message(struct SharedState* state) {
  const char** message = 0;
  switch (state->state) {
    case CONDITION_MET_MESSAGE:
      message = condition_met_message;
      break;
    case SETTINGS_MESSAGE:
      message = settings_message;
      break;
    case ZERO_CELLS_MESSAGE:
      message = zero_cells_message;
      break;
    default:
      return;
  }
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


