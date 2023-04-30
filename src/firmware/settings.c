#include "settings.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#include <string.h>
#include <stdio.h>

#define FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define FLASH_ADDRESS ((uint8_t*)(XIP_BASE + FLASH_OFFSET))

static uint16_t calc_checksum(const struct Settings* s) {
  const uint8_t* start = ((const uint8_t*)s) + sizeof(uint16_t);
  size_t len = sizeof(struct Settings) - sizeof(uint16_t);
  uint16_t sum = 0;
  for (size_t i=0; i<len; ++i) {
    sum += start[i] ^ i;
  }
  return sum;
}

#define LOW_R 1000.0
#define HIGH_R (6800.0 + 2200.0) 

static void global_settings_default(struct GlobalSettings* gs) {
  gs->ical_mohms = 133;
  gs->vcal_ratio = LOW_R / (LOW_R + HIGH_R);

  gs->vsag.interval_seconds = 10;
  gs->vsag.settle_ms = 1000;

  gs->fan.min_percent = 20;
  gs->fan.min_celsius = 40;
  gs->fan.max_celsius = 70;

  gs->slew.volts = 15.0;
  gs->slew.amps = 10.0;
  gs->slew.celsius = 30.0;

  gs->finish_display_ratio = 1.0;
}

static void init_profile(struct ProfileSettings* ps) {
  strcpy(ps->name, "Default");
  ps->drop_mv = 600;
  ps->cell_count = 0;

  ps->cell.target_mv = 3800;
  ps->cell.damage_mv = 3300;
  ps->cell.max_vsag_mv = 300;

  ps->max_ma = 25000;
  ps->max_celsius = 80;
  ps->max_watts = 400;
}

void settings_try_add_profile(struct Settings* settings) {
  if (settings->profile_count >= MAX_PROFILE_COUNT) {
    printf("Can not add a profile: max allowed is MAX_PROFILE_COUNT\n");
    return;
  }

  struct ProfileSettings* ps = settings->profile + settings->profile_count;
  init_profile(ps);
  printf("Added profile: %d %s\n", settings->profile_count, ps->name);
  ++settings->profile_count;
}

void settings_default(struct Settings* settings) {
  memset(settings, 0, sizeof(struct Settings));
  settings->eyecatcher[0] = 'B';
  settings->eyecatcher[0] = 'A';
  settings->eyecatcher[0] = 'T';
  settings->eyecatcher[0] = 'D';

  settings->profile_count = 0;
  global_settings_default(&(settings->global));
  settings_try_add_profile(settings);
}

void settings_load(struct Settings* settings) {
  memcpy(settings, FLASH_ADDRESS, sizeof(struct Settings));
  const uint16_t checksum = calc_checksum(settings);
  if (checksum != settings->checksum) {
    settings_default(settings);
  }
}

static inline size_t calc_sector_size(void) {
  const int num_sectors = 1 + (sizeof(struct Settings) / FLASH_SECTOR_SIZE);
  return num_sectors * FLASH_SECTOR_SIZE;
}

static inline size_t calc_page_size(void) {
  const int num_pages = 1 + (sizeof(struct Settings) / FLASH_PAGE_SIZE);
  return num_pages * FLASH_PAGE_SIZE;
}

// Do not call save() excessively as it could wear out the
// flash memory.
void settings_save(struct Settings* settings) {
  settings->checksum = calc_checksum(settings);
  flash_range_erase(FLASH_OFFSET, calc_sector_size());
  flash_range_program(FLASH_OFFSET, (const uint8_t*)settings, calc_page_size());
}

