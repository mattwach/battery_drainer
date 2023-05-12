#include "draining_battery_ui.h"
#include "util.h"
#include <stdio.h>

static void render_line0(struct SharedState* ss, uint8_t active) {
  char line[20];
  const uint32_t time_seconds = active ?
    (ss->uptime_ms - ss->state_started_ms) / 1000 :
    ss->max_values.time_seconds;
  const uint8_t hours = time_seconds / 3600;
  const uint8_t minutes = (time_seconds % 3600) / 60;
  const uint8_t seconds = (time_seconds % 60);

  const uint32_t charge_mah = active ?
    ss->charge_mas / 3600 :
    ss->max_values.charge_mah;
  const uint8_t ah = charge_mah / 1000;
  const uint8_t ah_frac = charge_mah % 1000;
  sprintf(
      line,
      //1:03:24 12345mAh
      "%d:%02d:%02d %05dmAh",
      hours,
      minutes,
      seconds,
      ah,
      ah_frac);
  struct Text* text = &(ss->text);
  text->row = 0;
  text->column = 0;
  text_strLen(text, line, OLED_COLUMNS);
}

static void render_line1_active(struct SharedState* ss) {
  char line[20];

  sprintf(line, "%dS ", ss->cells);
  struct Text* text = &(ss->text);
  text->row = 2;
  text->column = 0;
  text_strLen(text, line, 3);

  const uint8_t volts = ss->loaded_mv / 1000;
  const uint8_t volts_frac = (ss->loaded_mv / 100) % 10;
  sprintf(line, "%2d.%dV", volts, volts_frac);
  text->options =
    ss->voltage_sag_limited_ms > ss->uptime_ms ?
    TEXT_OPTION_INVERTED :
    0x00;
  text_strLen(text, line, 5);
  text->options = 0x00;

  const char sign =
    ss->loaded_mv > ss->target_mv ?
    '>' :
    '<';
  const uint8_t target_volts = ss->target_mv / 1000;
  const uint8_t target_volts_frac = (ss->target_mv / 100) % 10;
  sprintf(line, " %c %2d.%dV", sign, target_volts, target_volts_frac);
  text_strLen(text, line, 8);
}

static void render_line1_finished(struct SharedState* ss, uint16_t finish_seconds) {
  char line[20];
  const uint8_t volts = ss->loaded_mv / 1000;
  const uint8_t volts_frac = (ss->loaded_mv / 100) % 10;
  sprintf(
      line,
      "%dS %2d.%dV ",
      ss->cells,
      volts,
      volts_frac);
  struct Text* text = &(ss->text);
  text->row = 2;
  text->column = 0;
  text_strLen(text, line, 9);

  sprintf(line, "%6dS", finish_seconds);
  text->options = TEXT_OPTION_INVERTED;
  text_strLen(text, line, 7);
  text->options = 0x00;
}

static void render_line2(struct SharedState* ss, uint8_t active) {
  char line[20];
  const uint16_t current_ma =
    active ?
    ss->current_ma :
    ss->max_values.current_ma;
  const uint8_t amps = current_ma / 1000;
  const uint8_t amps_frac = (current_ma / 100) % 10;
  struct Text* text = &(ss->text);
  text->row = 4;
  text->column = 0;
  text->options =
    ss->current_limited_ms > ss->uptime_ms ?
    TEXT_OPTION_INVERTED :
    0x00;
  sprintf(line, "%2d.%dA", amps, amps_frac);
  text_strLen(text, line, 5);
  text->options = 0x00;

  text_strLen(text, "      ", 6);

  text->options =
   ss->power_limited_ms > ss->uptime_ms ?
   TEXT_OPTION_INVERTED :
   0x00;
  const uint16_t power_watts =
    active ?
    (uint32_t)ss->loaded_mv * (uint32_t)ss->current_ma / 1000000 :
    ss->max_values.power_watts;
  sprintf(line, "%4dW", power_watts);
  text_strLen(text, line, 5);
  text->options = 0x00;
}

static void render_line3(struct SharedState* ss, uint8_t active) {
  char line[20];
  struct Text* text = &(ss->text);
  text->row = 6;
  text->column = 0;
  text->options =
    ss->temperature_limited_ms > ss->uptime_ms ?
    TEXT_OPTION_INVERTED :
    0x00;
  const uint8_t temp_c =
    active ?
    ss->temperature_c :
    ss->max_values.temperature_c;
  sprintf(line, "%3dC", temp_c);
  text_strLen(text, line, 4);
  text->options = 0x00;

  uint8_t fet_percent =
    active ? (uint8_t)(((uint32_t)ss->vgs_level * 100) / 65535) :
    ss->max_values.fet_percent;
  if (active && ss->is_sampling_voltage) {
    fet_percent = 0;
  }
  const uint8_t fan_percent =
    active ? (uint8_t)(((uint32_t)ss->fan_level * 100) / 65535) :
    ss->max_values.fan_percent;
  sprintf(line, " P%3d%% F%3d%%", fet_percent, fan_percent);
  text_strLen(text, line, 12);
}

void draining_battery_ui_render_active(struct SharedState* state) {
  render_line0(state, 1);
  render_line1_active(state);
  render_line2(state, 1);
  render_line3(state, 1);
}

void draining_battery_ui_render_finished(
  struct SharedState* state, uint16_t finish_seconds) {
  render_line0(state, 0);
  render_line1_finished(state, finish_seconds);
  render_line2(state, 0);
  render_line3(state, 0);
}
