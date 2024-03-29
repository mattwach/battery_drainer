#include "settings.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#include <string.h>
#include <stdio.h>


static inline size_t calc_sector_size(void) {
  const int num_sectors = 1 + (sizeof(struct Settings) / FLASH_SECTOR_SIZE);
  return num_sectors * FLASH_SECTOR_SIZE;
}

static inline size_t calc_page_size(void) {
  const int num_pages = 1 + (sizeof(struct Settings) / FLASH_PAGE_SIZE);
  return num_pages * FLASH_PAGE_SIZE;
}

static inline uint32_t flash_offset(void) {
  return (PICO_FLASH_SIZE_BYTES - calc_sector_size());
}

static inline uint8_t* flash_address(void) {
  return ((uint8_t*)(XIP_BASE + flash_offset()));
}

static uint16_t calc_checksum(const struct Settings* s) {
  const uint8_t* start = ((const uint8_t*)s) + sizeof(uint16_t);
  size_t len = sizeof(struct Settings) - sizeof(uint16_t);
  uint16_t sum = 0;
  for (size_t i=0; i<len; ++i) {
    sum += start[i] ^ i;
  }
  return sum;
}

// default resistance is 1000 low and 6800 + 2200 high
// at 30V we would have I = 30 / 10000 = 3 mA
// that would read as 0.003 * 1000 = 3V on the low side
// this would give an ADC reading of 4096.
// thus our default ratio is 30000 mv / 4096 ~= 7.3242
//#define DEFAULT_VCAL_RATIO 7.3242
#define DEFAULT_VCAL_RATIO 7.2464  // measured

static void global_settings_default(struct GlobalSettings* gs) {
  gs->ical_mohms = 133;
  gs->vcal_ratio = DEFAULT_VCAL_RATIO;

  gs->vsag.interval_seconds = 10;
  gs->vsag.settle_ms = 1000;

  gs->fan.min_percent = 20;
  gs->fan.min_celsius = 40;
  gs->fan.max_celsius = 70;
  gs->fan.min_watts = 5;
  gs->fan.max_watts = 70;

  gs->response.max_velocity = 10.0;
  gs->response.min_velocity = 0.05;
  gs->response.acceleration = 0.1;
  gs->response.deceleration = 0.5;

  gs->finish_display = 1.0;
}

static void init_profile(struct ProfileSettings* ps) {
  strcpy(ps->name, "Default");
  ps->drop_mv = 0;
  ps->cell_count = 0;

  ps->cell.target_mv = 3800;
  ps->cell.damage_mv = 3300;
  ps->cell.max_vsag_mv = 300;

  ps->max_ma = 5000;
  ps->max_celsius = 80;
  ps->max_watts = 100;
}

void settings_try_add_profile(struct Settings* settings) {
  if (settings->profile_count >= MAX_PROFILE_COUNT) {
    printf("Can not add a profile: max allowed is MAX_PROFILE_COUNT\n");
    return;
  }

  struct ProfileSettings* ps = settings->profile + settings->profile_count;
  init_profile(ps);
  printf("Added profile: %d %s (not saved)\n", settings->profile_count, ps->name);
  ++settings->profile_count;
}

void settings_try_duplicate_profile(struct Settings* settings, uint8_t source_idx) {
  if (source_idx >= settings->profile_count) {
    printf("Invalid source profile index\n");
    return;
  }
  if (settings->profile_count >= MAX_PROFILE_COUNT) {
    printf("Can not add a profile: max allowed is MAX_PROFILE_COUNT\n");
    return;
  }

  const struct ProfileSettings* source_ps = settings->profile + source_idx;
  struct ProfileSettings* dest_ps = settings->profile + settings->profile_count;
  memcpy(dest_ps, source_ps, sizeof(struct ProfileSettings));
  printf("Duplicated profile: %d %s -> %d (not saved)\n", source_idx, dest_ps->name, settings->profile_count);
  ++settings->profile_count;
}

void settings_try_delete_profile(struct Settings* settings, uint8_t idx) {
  if (idx >= settings->profile_count) {
    printf("Invalid profile index\n");
    return;
  }
  if (settings->profile_count <= 1) {
    printf("At least one profile must be defined\n");
    return;
  }

  // say there are 5 profile and we want to remove #1
  // Thus we had 0, 1, 2, 3, 4 and want to have 0, 2, 3, 4
  //
  // copy length is 5 - 1 - 1 = 3
  const size_t copy_length = sizeof(struct ProfileSettings) * (settings->profile_count - idx - 1);
  if (copy_length > 1) {
    memmove(settings->profile + idx, settings->profile + idx + 1, copy_length);
  }
  --settings->profile_count;

  // Clear out the now-unused slot just for tidiness
  memset(settings->profile + settings->profile_count, 0, sizeof(struct ProfileSettings));
  printf("Removed profile %d (not saved)\n", idx);
}

void settings_try_move_profile(struct Settings* settings, uint8_t src_idx, uint8_t dest_idx) {
  if (src_idx >= settings->profile_count) {
    printf("Invalid source index\n");
    return;
  }
  if (dest_idx >= settings->profile_count) {
    printf("Invalid destination index\n");
    return;
  }
  if (src_idx == dest_idx) {
    printf("source == destination index\n");
    return;
  }

  // for out example, we will say we have 5 slots and want to move slot 1 to slot 3
  // e.g. from [0 1 2 3 4] to [0 2 3 1 4]

  // first, make a backup of the source profile
  struct ProfileSettings src_ps;
  memcpy(&src_ps, settings->profile + src_idx, sizeof(struct ProfileSettings));

  // next delete the source slot
  // Here we go from [0 1 2 3 4] to [0 2 3 4] with 1 backed up.
  size_t copy_length = sizeof(struct ProfileSettings) * (settings->profile_count - src_idx - 1);
  if (copy_length > 1) {
    memmove(settings->profile + src_idx, settings->profile + src_idx + 1, copy_length);
  }

  // now we want to make an opening at dest_idx
  // Here we go from [0 2 3 4] to [0 2 3 _ 4]
  copy_length = sizeof(struct ProfileSettings) * (settings->profile_count - dest_idx - 1);
  if (copy_length > 1) {
    memmove(settings->profile + dest_idx + 1, settings->profile + dest_idx, copy_length);
  }

  // finally, copy the backed-up profile into the empty slot
  memcpy(settings->profile + dest_idx, &src_ps, sizeof(struct ProfileSettings));
  printf("Moved profile %d to index %d (not saved)\n", src_idx, dest_idx);
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
  printf("Settings set to default values\n");
}

void settings_load(struct Settings* settings) {
  memcpy(settings, flash_address(), sizeof(struct Settings));
  const uint16_t checksum = calc_checksum(settings);
  if (checksum == settings->checksum) {
    printf("Settings loaded from flash\n");
  } else {
    settings_default(settings);
  }
}

// Do not call save() excessively as it could wear out the
// flash memory.
void settings_save(struct Settings* settings) {
  settings->checksum = calc_checksum(settings);
  uint32_t ints = save_and_disable_interrupts();
  flash_range_erase(flash_offset(), calc_sector_size());
  flash_range_program(flash_offset(), (const uint8_t*)settings, calc_page_size());
  restore_interrupts(ints);
  printf("Settings saved to flash.\n");
}

void settings_calc_voltage_and_cell_count(
    const struct Settings* settings,
    int profile_index,
    uint16_t current_mv,
    uint8_t* cell_count,
    uint16_t* target_mv) {
  *cell_count = 0;
  *target_mv = 0;
  if (profile_index >= settings->profile_count) {
    return;
  }
  const struct ProfileSettings* ps = settings->profile + profile_index;
  if (ps->cell.target_mv == 0) {
    return;
  }
  uint32_t cells = ps->cell_count > 0 ? ps->cell_count : current_mv / (ps->cell.target_mv - 150);
  if (cells > 6) {
      // Target_mv must be too low for the calculation to work
    return;
  }
  *cell_count = (uint8_t)cells;
  *target_mv = (uint16_t)(cells * ps->cell.target_mv);
}

