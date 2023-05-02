#include "profile_selection.h"

#include <stdio.h>
#include <string.h>

static void status_line(
    const struct Settings* settings, struct SharedState* state) {
  char line[32];
  // TODO: read this from the hardware later
  const uint16_t current_mv = 25100;
  uint8_t cell_count;
  uint16_t target_mv;
  settings_calc_voltage_and_cell_count(
      settings,
      state->active_profile_index,
      current_mv,
      &cell_count,
      &target_mv);

  struct Text* text = &(state->text);
  text->row = 0;
  text->column = 0;
  const char sign = current_mv > target_mv ? '>' : '<';
  sprintf(
      line,
      "%dS %2d.%dV %c %2d.%dV",
      cell_count,
      current_mv / 1000,
      (current_mv % 1000) / 100,
      sign,
      target_mv / 1000,
      (target_mv % 1000) / 100);
  text_strLen(text, line, 16);
}

static void current_profile(
    const struct Settings* settings, struct SharedState* state) {
  uint8_t first_index = state->active_profile_index;
  // scroll backwards up to 2 lines
  if (first_index > 0) {
    --first_index;
  }
  if (first_index > 0) {
    --first_index;
  }

  // last_index is inclusive
  uint8_t last_index = first_index + 2;
  if (last_index > settings->profile_count) {
    last_index = settings->profile_count;
  }

  struct Text* text = &(state->text);
  text->row = 2;
  char line[16];
  for (uint8_t i = first_index; i <= last_index; ++i, text->row += 2) {
    text->column = 0;
    text->options = i == state->active_profile_index ?
      TEXT_OPTION_INVERTED :
      0x00;
    memset(line, ' ', sizeof(line));
    if (i < settings->profile_count) {
      strcpy(line, settings->profile[i].name);
    } else {
      strcpy(line, "Settings...");
    }
    text_strLen(text, line, 16);
  }
}

void profile_selection(
    const struct Settings* settings, struct SharedState* state) {
  status_line(settings, state);
  current_profile(settings, state);
}

