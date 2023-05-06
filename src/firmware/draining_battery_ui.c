#include "draining_battery_ui.h"
#include "state.h"
#include "util.h"
#include <stdio.h>

static void render_line0(
    struct Text* text, const struct DrainingBatteryUIFields* fields) {
  char line[20];
  const uint8_t hours = fields->time_seconds / 3600;
  const uint8_t minutes = (fields->time_seconds % 3600) / 60;
  const uint8_t seconds = (fields->time_seconds % 60);

  const uint8_t ah = fields->charge_mah / 1000;
  const uint8_t ah_frac = (fields->charge_mah / 10) % 100;
  sprintf(
      line,
      "%02d:%02d:%02d %2d.%02dAh",
      hours,
      minutes,
      seconds,
      ah,
      ah_frac);
  text->row = 0;
  text->column = 0;
  text_strLen(text, line, OLED_COLUMNS);
}

static void render_line1_active(
    struct Text* text,
    const struct DrainingBatteryUIFields* fields,
    uint8_t cells,
    uint16_t target_mv) {
  char line[20];

  sprintf(line, "%dS ", cells);
  text->row = 2;
  text->column = 0;
  text_strLen(text, line, 3);

  const uint8_t volts = fields->current_mv / 1000;
  const uint8_t volts_frac = (fields->current_mv / 100) % 10;
  sprintf(line, "%2d.%dV", volts, volts_frac);
  text->options =
    fields->limiter == VOLTAGE_SAG ?
    TEXT_OPTION_INVERTED :
    0x00;
  text_strLen(text, line, 5);
  text->options = 0x00;

  const char sign =
    fields->current_mv > target_mv ?
    '>' :
    '<';
  const uint8_t target_volts = target_mv / 1000;
  const uint8_t target_volts_frac = (target_mv / 100) % 10;
  sprintf(line, " %c %2d.%dV", sign, target_volts, target_volts_frac);
  text_strLen(text, line, 8);
}

static void render_line1_finished(
    struct Text* text,
    const struct DrainingBatteryUIFields* fields,
    uint8_t cells,
    uint16_t finish_seconds) {
  char line[20];
  const uint8_t volts = fields->current_mv / 1000;
  const uint8_t volts_frac = (fields->current_mv / 100) % 10;
  sprintf(
      line,
      "%dS %2d.%dV ",
      cells,
      volts,
      volts_frac);
  text->row = 2;
  text->column = 0;
  text_strLen(text, line, 9);

  sprintf(line, "%6dS", finish_seconds);
  text->options = TEXT_OPTION_INVERTED;
  text_strLen(text, line, 7);
  text->options = 0x00;
}

static void render_line2(
    struct Text* text, const struct DrainingBatteryUIFields* fields) {
  char line[20];
  const uint8_t amps = fields->current_ma / 1000;
  const uint8_t amps_frac = (fields->current_ma / 100) % 10;
  text->row = 4;
  text->column = 0;
  text->options =
    fields->limiter == CURRENT_LIMIT ?
    TEXT_OPTION_INVERTED :
    0x00;
  sprintf(line, "%2d.%dA", amps, amps_frac);
  text_strLen(text, line, 5);
  text->options = 0x00;

  text_strLen(text, "      ", 6);

  text->options =
   fields->limiter == POWER_LIMIT ?
   TEXT_OPTION_INVERTED :
   0x00;
  sprintf(line, "%4dW", fields->power_watts);
  text_strLen(text, line, 5);
  text->options = 0x00;
}

static void render_line3(
    struct Text* text, const struct DrainingBatteryUIFields* fields) {
  char line[20];
  text->row = 6;
  text->column = 0;
  text->options =
    fields->limiter == TEMPERATURE_LIMIT ?
    TEXT_OPTION_INVERTED :
    0x00;
  sprintf(line, "%3dC", fields->temp_c);
  text_strLen(text, line, 4);
  text->options = 0x00;

  sprintf(line, " P%3d%% F%3d%%", fields->fet_percent, fields->fan_percent);
  text_strLen(text, line, 12);
}

void draining_battery_ui_render_active(
    struct Text* text,
    const struct DrainingBatteryUIFields* fields,
    uint8_t cells,
    uint16_t target_mv) {
  render_line0(text, fields);
  render_line1_active(text, fields, cells, target_mv);
  render_line2(text, fields);
  render_line3(text, fields);
}

void draining_battery_ui_render_finished(
    struct Text* text,
    const struct DrainingBatteryUIFields* fields,
    uint8_t cells,
    uint16_t finish_seconds) {
  render_line0(text, fields);
  render_line1_finished(text, fields, cells, finish_seconds);
  render_line2(text, fields);
  render_line3(text, fields);
}
