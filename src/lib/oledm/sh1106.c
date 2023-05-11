#include "oledm.h"

#include <string.h>
#include <error_codes.h>

#include "oledm_driver_common.inc"

#define ROTATE_180

enum ESH1106Commands
{
    SH1106_SETLOWCOLUMN     = 0x00,
    SH1106_SETHIGHCOLUMN    = 0x10,
    //SH1106_DEACTIVATE_SCROLL = 0x2E,
    SH1106_SETSTARTLINE     = 0x40,
    //SH1106_DEFAULT_ADDRESS  = 0x78,
    SH1106_SETCONTRAST      = 0x81,
    SH1106_SEGREMAP         = 0xA0,
    SH1106_DISPLAYALLON_RESUME = 0xA4,
    SH1106_DISPLAYALLON     = 0xA5,
    SH1106_NORMALDISPLAY    = 0xA6,
    SH1106_INVERTDISPLAY    = 0xA7,
    SH1106_SETMULTIPLEX     = 0xA8,
    SH1106_DISPLAYOFF       = 0xAE,
    SH1106_DISPLAYON        = 0xAF,
    SH1106_SETPAGE          = 0xB0,
    SH1106_COMSCANINC       = 0xC0,
    SH1106_COMSCANDEC       = 0xC8,
    SH1106_SETDISPLAYOFFSET = 0xD3,
    SH1106_SETDISPLAYCLOCKDIV = 0xD5,
    SH1106_SETPRECHARGE     = 0xD9,
    SH1106_SETCOMPINS       = 0xDA,
    SH1106_SETVCOMDETECT    = 0xDB,
    //SH1106_SWITCHCAPVCC     = 0x02,
    SH1106_NOP              = 0xE3,
    SH1106_READMODIFYWRITE  = 0xE0,
    SH1106_END              = 0xEE,
    SH1106_SETPUMPVOLTAGE   = 0x30,
    SH1106_DCDC_MODE        = 0xAD,
};

// Globals.  The SH1106 does not have the nice horozontal addressing
// mode that mades display communication more efficient.  So this
// driver emulates it for the rest of the code base, which can then simply
// assume it exists.
// 
// The downside of this "transparent" implementation is that the resulting
// globals prevent multiple SH1106 from being used at the same time (well,
// it's possible if you take care not to set the window before doing ops).
//
// Another approach would be to unhide the implementation and move these vars
// into the OLEDM structure.  This would waste a few bytes of memory when using
// SSD1306.
//
// Note: It's legal for start > end just as it's supported in the hardware.
// When this is the case, we implicitly wrap at the memory boundary (as the
// hardware does)
column_t address_start_column;
column_t address_end_column;
uint8_t address_start_page;
uint8_t address_end_page;
column_t address_column;
uint8_t address_page;
bool_t address_wrap_triggered;

// Memory rows.  Even if using a smaller screen, like 64x32, the SSD1306 chip
// still has an internal memory of 128x64.  This is important in some cases,
// such as while scrolling.
#define DISPLAY_MEMORY_ROWS 8
#define DISPLAY_MEMORY_COLUMNS 132
// % is expensive on AVR so let's use a heuristic that assumes we are not
// too far away.  It's reasonable since our types are uint8_t which can't be too
// far away.
#define WRAP_COLUMN(x) ((x) >= DISPLAY_MEMORY_COLUMNS ? \
                       (x) - DISPLAY_MEMORY_COLUMNS : (x))


static void _reset_globals(void) {
  address_start_column = 0;
  address_end_column = DISPLAY_MEMORY_COLUMNS - 1;
  address_start_page = 0;
  address_end_page = 0;
  address_column = 0;
  address_page = 0;
  address_wrap_triggered = FALSE;
}

void oledm_basic_init(struct OLEDM* display) {
  memset(display, 0, sizeof(struct OLEDM));
	display->column_offset = 2;
  display->visible_columns = 128;
  display->visible_rows = DISPLAY_MEMORY_ROWS;
  display->memory_columns = DISPLAY_MEMORY_COLUMNS;
  display->memory_rows = DISPLAY_MEMORY_ROWS;
  display->option_bits = OLEDM_ALTERNATE_COM_CONFIG;
  _reset_globals();
}

void oledm_start(struct OLEDM* display) {
  _reset_globals();
  oledm_ifaceInit();

  error_t* err = &(display->error);

  oledm_startCommands(err);
  oledm_command(SH1106_DISPLAYOFF, err);

  // For proper up to down display, COMSCAN
  // needs to be inverted from default
#ifdef ROTATE_180
  oledm_command(SH1106_COMSCANINC, err);
#else
  oledm_command(SH1106_COMSCANDEC, err);
#endif

  oledm_command(SH1106_SETSTARTLINE | display->start_line, err);

  // Default: SH1106_SETCONTRAST, 0x7F

  // For proper left-to-right display, thh SH1106_SEGREMAP
  // needs to be revered from default.
#ifdef ROTATE_180
  oledm_command(SH1106_SEGREMAP | 0x00, err);
#else
  oledm_command(SH1106_SEGREMAP | 0x01, err);
#endif   

  // Default: SH1106_NORMALDISPLAY
  // Default: SH1106_SETMULTIPLEX, 63
  // Default: SH1106_SETDISPLAYOFFSET, 0x00
  // Default: SH1106_SETDISPLAYCLOCKDIV, 0x00
  // Default: SH1106_SETPRECHARGE, 0x22

  uint8_t compins = 0x02;
  if (display->option_bits & OLEDM_ALTERNATE_COM_CONFIG) {
    compins |= 1 << 4;
  }
  if (display->option_bits & OLEDM_COM_LEFT_RIGHT_REMAP) {
    compins |= 1 << 5;
  }
  oledm_command(SH1106_SETCOMPINS, err);
  oledm_command(compins, err);

  // Default: SH1106_SETVCOMDETECT, 0x20

  // Turn the display back on
  oledm_command(SH1106_DISPLAYALLON_RESUME, err);
  oledm_command(SH1106_DISPLAYON, err);
  oledm_stop(display);
}

void oledm_set_memory_bounds(
    struct OLEDM* display,
    column_t left_column,
    uint8_t top_row,
    column_t right_column,
    uint8_t bottom_row) {

  address_start_column = WRAP_COLUMN(left_column);
  address_end_column = WRAP_COLUMN(right_column);
  address_start_page = top_row & (DISPLAY_MEMORY_ROWS - 1);
  address_end_page = bottom_row & (DISPLAY_MEMORY_ROWS - 1);
  address_column = address_start_column;
  address_page = address_start_page;
  address_wrap_triggered = FALSE;

  error_t* err = &(display->error);

  oledm_startCommands(err);
  oledm_command(SH1106_SETPAGE | (address_page & 0x07), err);
  oledm_command(SH1106_SETLOWCOLUMN | (address_column & 0x0F), err);
  oledm_command(SH1106_SETHIGHCOLUMN | ((address_column >> 4) & 0x0F), err);
  oledm_stop(display);
}

void oledm_write_pixels(struct OLEDM* display, uint8_t byte){ 
  if (address_wrap_triggered) {
    address_wrap_triggered = FALSE;

    // next row
    address_column = address_start_column;
    address_page = (address_page + 1) & (DISPLAY_MEMORY_ROWS - 1);

    if (address_page == ((address_end_page + 1) & (DISPLAY_MEMORY_ROWS - 1))) {
      address_page = address_start_page;
    }

    // tell the hardware
    error_t* err = &(display->error);
    oledm_stop(display); // <-- need to stop the data transfer
    oledm_startCommands(err);
    oledm_command(SH1106_SETPAGE | (address_page & 0x07), err);
    oledm_command(SH1106_SETLOWCOLUMN | (address_column & 0x0F), err);
    oledm_command(SH1106_SETHIGHCOLUMN | ((address_column >> 4) & 0x0F), err);
    oledm_stop(display);
    oledm_start_pixels(display); // <-- resume data transfer
  }

  oledm_ifaceWriteData(byte, &(display->error));

  address_column = WRAP_COLUMN(address_column + 1);
  if (address_column == WRAP_COLUMN(address_end_column + 1)) {
    // We need to wrap for the next byte, but this could be the last byte
    // we'll see anyway so defer the page set until a bye is actually received.
    address_wrap_triggered = TRUE;
  }
}

void oledm_vscroll(struct OLEDM* display, int8_t rows) {
  // Even though there may be less than display->visible_rows visible on
  // a smaller display, the memory still supports 8 rows.  The logic
  // needs to respect this fact or you'll end up showing regions
  // that can not be updated.
  display->start_line = (display->start_line + rows) & (DISPLAY_MEMORY_ROWS - 1);

  error_t* err = &(display->error);
  oledm_startCommands(err);
  oledm_command(SH1106_SETSTARTLINE | (display->start_line * 8), err);
  oledm_stop(display);
}

void oledm_clear(struct OLEDM* display, uint8_t byte) {
  if (display->error) {
    return;
  }

  error_t* err = &(display->error);
  uint8_t row = 0;
  for (; row < DISPLAY_MEMORY_ROWS; ++row) {
    oledm_startCommands(err);
    oledm_command(SH1106_SETPAGE | row, err);
    oledm_command(SH1106_SETLOWCOLUMN | ((DISPLAY_MEMORY_COLUMNS - 1) & 0x0F), err);
    oledm_command(SH1106_SETHIGHCOLUMN | ((DISPLAY_MEMORY_COLUMNS - 1) >> 4), err);
    oledm_stop(display);

    column_t col = 0;
    oledm_start_pixels(display);
    for (; col < DISPLAY_MEMORY_COLUMNS; ++col) {
      oledm_ifaceWriteData(byte, &(display->error));
    }
    oledm_stop(display);
  }
}

void oledm_display_off(struct OLEDM* display) {
  error_t* err = &(display->error);
  oledm_startCommands(err);
  oledm_command(SH1106_DISPLAYOFF, err);
  oledm_stop(display);
}

void oledm_display_on(struct OLEDM* display) {
  error_t* err = &(display->error);
  oledm_startCommands(err);
  oledm_command(SH1106_DISPLAYON, err);
  oledm_stop(display);
}
