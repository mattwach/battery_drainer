#include "console.h"

#include "uart_console/console.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct ConsoleConfig cc;
static struct Settings* settings;
static uint16_t current_mv;

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
    printf("%s is above the maximum of %d\n", name, min);
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
    printf("%s is above the maximum of %f\n", name, min);
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
}

static void dump_global_settings(void) {
  const struct GlobalSettings* g = &(settings->global);
  printf("Global settings:\n");
  printf("  ical:                 %d milliOhms\n", g->ical_mohms);
  printf("  vcal ratio:           %.2f\n", g->vcal_ratio);
  printf("  finish display ratio: %.2f\n", g->finish_display_ratio);
  printf("  voltage sag:\n");
  printf("    interval:           %d seconds\n", g->vsag.interval_seconds);
  printf("    settle time:        %d ms\n", g->vsag.settle_ms);
  printf("  fan settings:\n");
  printf("    minimum level:      %d%\n", g->fan.min_percent);
  printf("    minimum temp:       %d C\n", g->fan.min_celsius);
  printf("    100% temp:          %d C\n", g->fan.max_celsius);
  printf("  FET settings:\n");
  printf("    voltage slew:       %.2f seconds\n", g->slew.volts);
  printf("    current slew:       %.2f seconds\n", g->slew.amps);
  printf("    temp slew:          %.2f seconds\n", g->slew.celsius);
}

static void dump_profile(int i) {
  const struct ProfileSettings* p = settings->profile + i;
  printf("\n");
  printf("Profile %d:\n", i);
  printf("  name:             %s\n", p->name);
  printf("  vdrop:            %d mV\n", p->drop_mv);
  printf("  max current:      %.1f A\n", (float)p->max_ma / 1000.0);
  printf("  max temp:         %d C\n", p->max_celsius);
  printf("  max power:        %d Watts\n", p->max_watts);
  if (p->cell_count) {
    printf("  cell_count:       %d\n", p->cell_count);
  } else {
    printf("  cell_count:       AUTO\n");
  }
  printf("  per_cell:\n");
  printf("    target volts:   %d mV\n", p->cell.target_mv);
  printf("    damage warning: %d mV\n", p->cell.damage_warning);
  printf("    max sag:        %d mV\n", p->cell.max_vsag_mv);
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

struct ConsoleCallback callbacks[] = {
    {"discard", "Discard changes / reload flash", 0, discard_cmd},
    {"ical", "Sets the current shunt resistance (ohms)", 1, ical_cmd},
    {"reset", "resets settings without saving.", 0, reset_cmd},
    {"save", "Write configuration to flash memory", 0, save_cmd},
    {"show", "Display settings", -1, show_cmd},
};

void console_init(struct Settings* s) {
  settings = s;
  uart_console_init(
      &cc,
      callbacks,
      sizeof(callbacks) / sizeof(callbacks[0]),
      CONSOLE_VT102);
}

void console_poll(uint16_t current_mv_) {
  current_mv = current_mv_;
  uart_console_poll(&cc, "> ");
}


