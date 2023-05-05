#include "console.h"
#include "state.h"

#include "uart_console/console.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct ConsoleConfig cc;
static struct Settings* settings;
static uint16_t vcal_adc_reading;

static uint8_t parse_int(const char* name, const char* vstr, int min, int max, int* val) {
  const int v = atoi(vstr);
  if (v == 0 && strcmp(vstr, "0")) {
    printf("Syntax error parsing %s\n", name);
    return 0;
  }

  if (v < min) {
    printf("%s is below the minimum of %d\n", name, min);
    return 0;
  }

  if (v > max) {
    printf("%s is above the maximum of %d\n", name, max);
    return 0;
  }

  *val = v;
  return 1;
}

static uint8_t is_float_zero(const char* vstr) {
  int idx = 0;
  uint8_t point_found = 0;
  for (; *vstr; ++vstr, ++idx) {
    switch (*vstr) {
      case '0':
        break;
      case '.':
        if (point_found) {
          return 0;
        }
        point_found = 1;
        break;
      default:
        return 0;
    }
  }
  return 1;
}

static uint8_t parse_float(const char* name, const char* vstr, float min, float max, float* val) {
  const float v = (float)atof(vstr);
  if (v == 0.0 && !is_float_zero(vstr)) {
    printf("Syntax error parsing %s\n", name);
    return 0;
  }

  if (v < min) {
    printf("%s is below the minimum of %f\n", name, min);
    return 0;
  }

  if (v > max) {
    printf("%s is above the maximum of %f\n", name, max);
    return 0;
  }

  *val = v;
  return 1;
}

static void discard_cmd(uint8_t argc, char* argv[]) {
  settings_load(settings);
}

static void save_cmd(uint8_t argc, char* argv[]) {
  settings_save(settings);
}

static void reset_cmd(uint8_t argc, char* argv[]) {
  settings_default(settings);
  printf("Settings reset (not saved)\n");
}

static void dump_global_settings(void) {
  const struct GlobalSettings* g = &(settings->global);
  printf("Global settings:\n");
  printf("  ical:                 %d milliOhms\n", g->ical_mohms);
  printf("  vcal ratio:           %.4f\n", g->vcal_ratio);
  printf("  finish display:       %.1f * mah_drained\n", g->finish_display);
  printf("  voltage sag:\n");
  printf("    interval:           %d seconds\n", g->vsag.interval_seconds);
  printf("    settle time:        %d ms\n", g->vsag.settle_ms);
  printf("  fan settings:\n");
  printf("    minimum level:      %d%\n", g->fan.min_percent);
  printf("    minimum temp:       %d C\n", g->fan.min_celsius);
  printf("    100%% temp:          %d C\n", g->fan.max_celsius);
  printf("  FET settings:\n");
  printf("    voltage slew:       %.2f seconds\n", g->slew.volts);
  printf("    current slew:       %.2f seconds\n", g->slew.amps);
  printf("    temp slew:          %.2f seconds\n", g->slew.celsius);
}

static void dump_profile(int i) {
  const struct ProfileSettings* p = settings->profile + i;
  printf("\n");
  printf("Profile %d:\n", i);
  printf("  name:           %s\n", p->name);
  printf("  vdrop:          %d mV\n", p->drop_mv);
  printf("  max current:    %.1f A\n", (float)p->max_ma / 1000.0);
  printf("  max temp:       %d C\n", p->max_celsius);
  printf("  max power:      %d Watts\n", p->max_watts);
  if (p->cell_count) {
    printf("  cell_count:     %d\n", p->cell_count);
  } else {
    printf("  cell_count:     AUTO\n");
  }
  printf("  per_cell:\n");
  printf("    target volts: %d mV\n", p->cell.target_mv);
  printf("    damage volts: %d mV\n", p->cell.damage_mv);
  printf("    max sag:      %d mV\n", p->cell.max_vsag_mv);
}

static void show_cmd(uint8_t argc, char* argv[]) {
  dump_global_settings();

  if (argc == 0) {
    for (int i=0; i<settings->profile_count; ++i) {
      dump_profile(i);
    }
  } else {
    for (int i=0; i<argc; ++i) {
      int v;
      if (parse_int("profile_index", argv[i], 0, settings->profile_count - 1, &v)) {
        dump_profile(v);
      }
    }
  }
}

static void ical_cmd(uint8_t argc, char* argv[]) {
  float v = 0.0;
  if (!parse_float("ical", argv[0], 0.01, 1.0, &v)) {
    return;
  }
  settings->global.ical_mohms = (uint16_t)(v * 1000);
  printf("ical changed to %d milliohms (not saved)\n", settings->global.ical_mohms);
}

static void finish_display_cmd(uint8_t argc, char* argv[]) {
  float v = 0.0;
  if (!parse_float("finish_display", argv[0], 0.0, 60.0, &v)) {
    return;
  }
  settings->global.finish_display = v;
  printf("finish display changed to %.2f * mah_drained seconds (not saved)\n",
      settings->global.finish_display);
}

static void fet_slew_volts_seconds_cmd(uint8_t argc, char* argv[]) {
  float v = 0.0;
  if (!parse_float("volts_slew_seconds", argv[0], 0.1, 60.0, &v)) {
    return;
  }
  settings->global.slew.volts = v;
  printf("volts slew  changed to %.2f seconds (not saved)\n",
      settings->global.slew.volts);
}

static void fet_slew_amps_seconds_cmd(uint8_t argc, char* argv[]) {
  float v = 0.0;
  if (!parse_float("volts_amps_seconds", argv[0], 0.1, 60.0, &v)) {
    return;
  }
  settings->global.slew.amps = v;
  printf("amps slew  changed to %.2f seconds (not saved)\n",
      settings->global.slew.amps);
}

static void fet_slew_celsius_seconds_cmd(uint8_t argc, char* argv[]) {
  float v = 0.0;
  if (!parse_float("celsius_amps_seconds", argv[0], 0.1, 60.0, &v)) {
    return;
  }
  settings->global.slew.celsius = v;
  printf("celsius slew  changed to %.2f seconds (not saved)\n",
      settings->global.slew.celsius);
}

static void vcal_cmd(uint8_t argc, char* argv[]) {
  if (vcal_adc_reading < 100) {
    printf("vcal_adc_reading is too low for calibration: %d\n", vcal_adc_reading);
    return;
  }
  float v = 0.0;
  if (!parse_float("vcal", argv[0], 4.0, 29.0, &v)) {
    return;
  }
  // We want to current vcal_adc_reading to equate to the measured voltage
  // vcal_adc_reading * vcal_ratio = (v * 1000)
  settings->global.vcal_ratio = ((v * 1000.0) / (float)vcal_adc_reading);
  printf("vcal_ratio changed to %.4f (%d * %.4f = %.1f mv) (not saved)\n",
      settings->global.vcal_ratio,
      vcal_adc_reading,
      settings->global.vcal_ratio,
      v * 1000.0);
}

static void vsag_interval_seconds_cmd(uint8_t argc, char* argv[]) {
  int v = 0;
  if (!parse_int("vsag_inteval", argv[0], 1, 30, &v)) {
    return;
  }
  settings->global.vsag.interval_seconds = (uint8_t)v;
  printf(
      "vsag interval changed to %d seconds (not saved)\n",
      settings->global.vsag.interval_seconds);
}

static void vsag_settle_ms_cmd(uint8_t argc, char* argv[]) {
  int v = 0;
  if (!parse_int("vsag_settle", argv[0], 100, 5000, &v)) {
    return;
  }
  settings->global.vsag.settle_ms = (uint16_t)v;
  printf(
      "vsag settle changed to %d ms (not saved)\n",
      settings->global.vsag.settle_ms);
}

static void fan_cmd(uint8_t argc, char* argv[]) {
  int min_percent = 0;
  if (!parse_int("min_percent", argv[0], 1, 100, &min_percent)) {
    return;
  }
  int min_celsius = 0;
  if (!parse_int("min_celsius", argv[1], 20, 200, &min_celsius)) {
    return;
  }
  int max_celsius = 0;
  if (!parse_int("max_celsius", argv[2], 20, 200, &max_celsius)) {
    return;
  }

  if (min_celsius > max_celsius) {
    printf("min_celsius must be less than max_celsius\n");
    return;
  }

  struct FanSettings* fan = &(settings->global.fan);
  fan->min_percent = (uint8_t)min_percent;
  fan->min_celsius = (uint8_t)min_celsius;
  fan->max_celsius = (uint8_t)max_celsius;

  printf("fan settings changed.  min %% = %d, min temp = %d C, 100%% temp = %d C (not saved)\n",
      fan->min_percent,
      fan->min_celsius,
      fan->max_celsius);
}

static void new_cmd(uint8_t argc, char* argv[]) {
  settings_try_add_profile(settings);
}

static void duplicate_cmd(uint8_t argc, char* argv[]) {
  int v = 0;
  if (!parse_int("profile_index", argv[0], 0, settings->profile_count - 1, &v)) {
    return;
  }
  settings_try_duplicate_profile(settings, (uint8_t)v);
}

static void delete_cmd(uint8_t argc, char* argv[]) {
  int v = 0;
  if (!parse_int("profile_index", argv[0], 0, settings->profile_count - 1, &v)) {
    return;
  }
  settings_try_delete_profile(settings, (uint8_t)v);
}

static void move_cmd(uint8_t argc, char* argv[]) {
  int src_idx = 0;
  if (!parse_int("src_index", argv[0], 0, settings->profile_count - 1, &src_idx)) {
    return;
  }
  int dest_idx = 0;
  if (!parse_int("dest_index", argv[1], 0, settings->profile_count - 1, &dest_idx)) {
    return;
  }
  settings_try_move_profile(settings, (uint8_t)src_idx, (uint8_t)dest_idx);
}

static void name_cmd(uint8_t argc, char* argv[]) {
  int v = 0;
  if (!parse_int("profile_index", argv[0], 0, settings->profile_count - 1, &v)) {
    return;
  }
  const int len = strlen(argv[1]);
  if (len == 0) {
    printf("Name too short.\n");
    return;
  }
  if (len > OLED_COLUMNS) {
    printf("Name too long: %d > %d max length\n", len, OLED_COLUMNS);
    return;
  }
  struct ProfileSettings* ps = settings->profile + v;
  memset(ps->name, 0, sizeof(ps->name));
  strcpy(ps->name, argv[1]);
  printf("Profile %d name updated to: %s (not saved)\n", v, ps->name);
}

static void list_cmd(uint8_t argc, char* argv[]) {
  for (uint8_t i=0; i<settings->profile_count; ++i) {
    printf("%d: %s\n", i, settings->profile[i].name);
  }
}

static void max_amps_cmd(uint8_t argc, char* argv[]) {
  int idx = 0;
  if (!parse_int("profile_index", argv[0], 0, settings->profile_count - 1, &idx)) {
    return;
  }
  float max_amps = 0.0;
  if (!parse_float("max_amps", argv[1], 0.01, 30.0, &max_amps)) {
    return;
  }
  settings->profile[idx].max_ma = (int16_t)(max_amps * 1000.0);
  printf("Set max_amps of profile %d to %d mA (not saved)\n", idx, settings->profile[idx].max_ma);
}

static void max_celsius_cmd(uint8_t argc, char* argv[]) {
  int idx = 0;
  if (!parse_int("profile_index", argv[0], 0, settings->profile_count - 1, &idx)) {
    return;
  }
  int max_temp = 0;
  if (!parse_int("max_celsius", argv[1], 30, 150, &max_temp)) {
    return;
  }
  settings->profile[idx].max_celsius = (uint8_t)(max_temp);
  printf("Set max_temp of profile %d to %d C (not saved)\n", idx, settings->profile[idx].max_celsius);
}

static void max_watts_cmd(uint8_t argc, char* argv[]) {
  int idx = 0;
  if (!parse_int("profile_index", argv[0], 0, settings->profile_count - 1, &idx)) {
    return;
  }
  int max_watts = 0;
  if (!parse_int("max_watts", argv[1], 30, 150, &max_watts)) {
    return;
  }
  settings->profile[idx].max_watts = (uint16_t)(max_watts);
  printf("Set max power of profile %d to %d Watts (not saved)\n", idx, settings->profile[idx].max_watts);
}

static void vdrop_cmd(uint8_t argc, char* argv[]) {
  int idx = 0;
  if (!parse_int("profile_index", argv[0], 0, settings->profile_count - 1, &idx)) {
    return;
  }
  float vdrop = 0.0;
  if (!parse_float("vdrop", argv[1], -1.0, 3.0, &vdrop)) {
    return;
  }
  settings->profile[idx].drop_mv = (int16_t)(vdrop * 1000.0);
  printf("Set vdrop of profile %d to %d mV (not saved)\n", idx, settings->profile[idx].drop_mv);
}

static void cell_count_cmd(uint8_t argc, char* argv[]) {
  int idx = 0;
  if (!parse_int("profile_index", argv[0], 0, settings->profile_count - 1, &idx)) {
    return;
  }
  int cell_count = 0;
  if (!parse_int("cell_count", argv[1], 0, 6, &cell_count)) {
    return;
  }
  settings->profile[idx].cell_count = (uint8_t)cell_count;
  printf("Set cell count of profile %d to %d (not saved)\n", idx, settings->profile[idx].cell_count);
}

static void per_cell_damage_volts_cmd(uint8_t argc, char* argv[]) {
  int idx = 0;
  if (!parse_int("profile_index", argv[0], 0, settings->profile_count - 1, &idx)) {
    return;
  }
  float damage_volts = 0.0;
  if (!parse_float("damage_volts", argv[1], 0.0, 30.0, &damage_volts)) {
    return;
  }
  settings->profile[idx].cell.damage_mv = (uint16_t)(damage_volts * 1000.0);
  printf("Set damage volts of profile %d to %d mV / Cell (not saved)\n", idx, settings->profile[idx].cell.damage_mv);
}

static void per_cell_target_volts_cmd(uint8_t argc, char* argv[]) {
  int idx = 0;
  if (!parse_int("profile_index", argv[0], 0, settings->profile_count - 1, &idx)) {
    return;
  }
  float target_volts = 0.0;
  if (!parse_float("target_volts", argv[1], 0.0, 30.0, &target_volts)) {
    return;
  }
  settings->profile[idx].cell.target_mv = (uint16_t)(target_volts * 1000.0);
  printf("Set target volts of profile %d to %d mV / Cell (not saved)\n", idx, settings->profile[idx].cell.target_mv);
}

static void per_cell_max_vsag_cmd(uint8_t argc, char* argv[]) {
  int idx = 0;
  if (!parse_int("profile_index", argv[0], 0, settings->profile_count - 1, &idx)) {
    return;
  }
  float max_vsag = 0.0;
  if (!parse_float("max_vsag", argv[1], 0.05, 1.0, &max_vsag)) {
    return;
  }
  settings->profile[idx].cell.max_vsag_mv = (uint16_t)(max_vsag * 1000.0);
  printf("Set max vsag of profile %d to %d mV / Cell (not saved)\n", idx, settings->profile[idx].cell.max_vsag_mv);
}


struct ConsoleCallback callbacks[] = {
    {"cell_count", "Sets cell count (0 is auto): <profile_index> <cell_count>", 2, cell_count_cmd},
    {"discard", "Discard changes / reload flash", 0, discard_cmd},
    {"delete", "Deletes profile <index>", 1, delete_cmd},
    {"duplicate", "Duplicate profile <index> as a new profile", 1, duplicate_cmd},
    {"fan", "Sets fan profile <min_percent> <min_celsius> <max_celsius>", 3, fan_cmd},
    {"fet_slew_volts_seconds", "Sets the FET response speed for voltage targets <seconds>", 1, fet_slew_volts_seconds_cmd},
    {"fet_slew_amps_seconds", "Sets the FET response speed for current targets <seconds>", 1, fet_slew_amps_seconds_cmd},
    {"fet_slew_celsius_seconds", "Sets the FET response speed for temperature targets <seconds>", 1, fet_slew_celsius_seconds_cmd},
    {"finish_display", "Sets the finish display as <seconds_per_mah_drained>", 1, finish_display_cmd},
    {"ical", "Sets the current shunt resistance (ohms)", 1, ical_cmd},
    {"list", "List profile names", 0, list_cmd},
    {"max_amps", "Sets maximum amps: <profile_index> <amps>", 2, max_amps_cmd},
    {"max_celsius", "Sets maximum temperature: <profile_index> <temp>", 2, max_celsius_cmd},
    {"max_watts", "Sets maximum power: <profile_index> <watts>", 2, max_watts_cmd},
    {"move", "Move a profile: <src_index> <dest_idx>", 2, move_cmd},
    {"name", "Rename a profile: <index> \"<name>\"", 2, name_cmd},
    {"new", "Creates a new profile", 0, new_cmd},
    {"per_cell_damage_volts", "Sets damage voltage: <profile_index> <voltage>", 2, per_cell_damage_volts_cmd},
    {"per_cell_max_vsag", "Sets sag volts: <profile_index> <voltage>", 2, per_cell_max_vsag_cmd},
    {"per_cell_target_volts", "Sets target voltage: <profile_index> <voltage>", 2, per_cell_target_volts_cmd},
    {"reset", "resets settings without saving.", 0, reset_cmd},
    {"save", "Write configuration to flash memory", 0, save_cmd},
    {"show", "Display settings", -1, show_cmd},
    {"vcal", "Calibrates the voltage ratio", 1, vcal_cmd},
    {"vdrop", "Sets voltage drop: <profile_index> <voltage_drop>", 2, vdrop_cmd},
    {"vsag_interval_seconds", "Changes how often to check vsag", 1, vsag_interval_seconds_cmd},
    {"vsag_settle_ms", "Changes how long to let the voltage setting before measuring vsag", 1, vsag_settle_ms_cmd},
};

void console_init(struct Settings* s) {
  settings = s;
  uart_console_init(
      &cc,
      callbacks,
      sizeof(callbacks) / sizeof(callbacks[0]),
      CONSOLE_VT102);
}

void console_poll(uint16_t vcal_adc_reading_) {
  vcal_adc_reading = vcal_adc_reading_;
  uart_console_poll(&cc, "> ");
}


