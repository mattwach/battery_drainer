#include "profile_selection.h"

#include <stdio.h>

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
  text->row = 1;
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

void profile_selection(
    const struct Settings* settings, struct SharedState* state) {
  status_line(settings, state);
}

