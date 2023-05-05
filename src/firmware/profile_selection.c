#include "profile_selection.h"

#include <stdio.h>
#include <string.h>

static void check_buttons(
    const struct Settings* settings,
    struct SharedState* state,
    uint16_t current_mv) {
  if (!state->button) {
    return;
  }

  if (state->button & NEXT_PRESSED) {
    ++state->active_profile_index;
    // Not >= so that "Settings..." can be selected
    if (state->active_profile_index > settings->profile_count) {
      state->active_profile_index = 0;
    }
  }

  if (state->button & OK_PRESSED) {
    if (state->active_profile_index >= settings->profile_count) {
      state->state = SETTINGS_MESSAGE;
      return;
    }

    const struct ProfileSettings* ps = settings->profile + state->active_profile_index;
    uint8_t cell_count = 0;
    uint16_t target_mv = 0;
    settings_calc_voltage_and_cell_count(
        settings,
        state->active_profile_index,
        current_mv,
        &cell_count,
        &target_mv);
    if (cell_count == 0) {
      state->state = BAD_SETUP_MESSAGE;
    } else if (ps->cell.target_mv <= ps->cell.damage_mv) {
      state->state = DETROY_BATTERY_CONFIRMATION;
    } else {
      state->state = DRAINING_BATTERY;
    }
  }
}

static void status_line(
    const struct Settings* settings,
    struct SharedState* state,
    uint16_t current_mv) {
  char line[32];
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
  text_strLen(text, line, OLED_COLUMNS);
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
  char line[OLED_COLUMNS];
  for (uint8_t i = first_index; i <= last_index; ++i, text->row += 2) {
    text->column = 0;
    text->options = i == state->active_profile_index ?
      TEXT_OPTION_INVERTED :
      0x00;
    if (i < settings->profile_count) {
      fill_line(line, settings->profile[i].name);
    } else {
      fill_line(line, "Settings...");
    }
    text_strLen(text, line, OLED_COLUMNS);
  }
  text->options = 0x00;

  // need to fill in any remaining lines with white space
  fill_line(line, "");
  for (; text->row < 8; text->row += 2) {
    text->column = 0;
    text_strLen(text, line, OLED_COLUMNS);
  }
}

void profile_selection(
    const struct Settings* settings,
    struct SharedState* state,
    uint16_t current_mv) {
  // This handles sudden changes to the profile coount via the console
  // when == to profile count, we are on the "setttings" selection.
  if (state->active_profile_index > settings->profile_count) {
    state->active_profile_index = settings->profile_count;
  }
  check_buttons(settings, state, current_mv);
  status_line(settings, state, current_mv);
  current_profile(settings, state);
}

