#include "console.h"

#include "uart_console/console.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct ConsoleConfig cc;
static struct Settings* settings;

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
  printf("  vcal ratio:           %f\n", g->vcal_ratio);
  printf("  finish display ratio: %f\n", g->finish_display_ratio);
  printf("  voltage sag:\n");
  printf("    interval:           %d seconds\n", g->vsag.interval_seconds);
  printf("    settle time:        %d ms\n", g->vsag.settle_ms);
  printf("  fan settings:\n");
  printf("    minimum level:      %d%\n", g->fan.min_percent);
  printf("    minimum temp:       %d C\n", g->fan.min_celsius);
  printf("    100% temp:          %d C\n", g->fan.max_celsius);
  printf("  FET settings:\n");
  printf("    voltage slew:       %f seconds\n", g->slew.volts);
  printf("    current slew:       %f seconds\n", g->slew.amps);
  printf("    temp slew:          %f seconds\n", g->slew.celsius);
}

static void dump_profile(int i) {
  const struct ProfileSettings* p = settings->profile + i;
  printf("\n");
  printf("Profile %d:\n", i);
  printf("  name:             %s\n", p->name);
  printf("  vdrop:            %d mV\n", p->drop_mv);
  printf("  max current:      %f A\n", (float)p->max_ma * 1000.0);
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

struct ConsoleCallback callbacks[] = {
    {"discard", "Discard changes / reload flash", 0, discard_cmd},
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

void console_poll(void) {
  uart_console_poll(&cc, "> ");
}


