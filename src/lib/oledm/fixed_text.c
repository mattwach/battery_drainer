#include "text.h"

#include "oledm.h"
#include "avr_flash.h"
#include <string.h>

#include "text_common.inc"

void text_verifyFont(struct Text* text) {
  if (text->display->error) {
    return;
  }

  const struct FontASCII* font = (struct FontASCII*)(text->font);
  const uint8_t* id = font->id;
  if ((pgm_read_byte_near(id) != 'F') ||
      (pgm_read_byte_near(id + 1) != 'A') || 
      (pgm_read_byte_near(id + 2) != 'S') ||
      (pgm_read_byte_near(id + 3) != '1')) {
    text->display->error = SSD1306_BAD_FONT_ID_ERROR;
  }
}

// Write a full row of text for a given string
static inline void write_row_fixed(struct Text* text, const char* str, uint8_t len, uint8_t row) {
  const struct FontASCII* font = (struct FontASCII*)(text->font);
  const uint8_t font_width = pgm_read_byte_near(&font->width);
  const uint8_t font_first_char = pgm_read_byte_near(&font->first_char);
  const uint8_t font_last_char = pgm_read_byte_near(&font->last_char);
  const uint8_t font_height = pgm_read_byte_near(&font->height);
  const uint8_t row_offset = row * font_width;
  const uint8_t bytes_per_char = font_width * font_height;
  const column_t memory_columns = text->display->memory_columns;
  uint8_t i = 0;
  column_t column = text->column;
  // This loop breaks down the row into characters
  for (; i < len; ++i) {
    const char c = str[i];
    uint8_t j = 0;
    // This loop completes the byte sequence for a given character
    for (; (j < font_width) && (column < memory_columns); ++j, ++column) {
      uint8_t b = 0xFF;
      if (c == ' ') {
        b = 0x00;  // Space character
      } else if ((c >= font_first_char) && (c <= font_last_char)) {
        b = pgm_read_byte_near(
          font->data +  // baseline 
          ((c - font_first_char) * bytes_per_char) + // char offset
          row_offset +
          j
        );
      }
      if (text->options & TEXT_OPTION_INVERTED) {
        b ^= 0xFF;  // invert the data
      }
      oledm_write_pixels(text->display, b);
    }
  }
}

void text_strLen(struct Text* text, const char* str, uint8_t len) {
  text_verifyFont(text);
  if (len == 0) {
    // Nothing to do
    return;
  }

  if (text->display->error) {
    // display is in an error state
    return;
  }

  const struct FontASCII* font = (struct FontASCII*)(text->font);
  const uint8_t font_width = pgm_read_byte_near(&font->width);
  const uint8_t font_height = pgm_read_byte_near(&font->height);
  const uint8_t memory_rows = text->display->memory_rows;
  const column_t memory_columns = text->display->memory_columns;

  if (text->column >= memory_columns) { 
    // already at the edge
    return;
  }

  if (text->row >= memory_rows) {
    // already off the bottom
    return;
  }

  uint16_t max_column = (uint16_t)text->column + (len * (uint16_t)font_width) - 1;
  if (max_column > (memory_columns - 1)) {
    max_column = memory_columns - 1;
  }

  uint8_t max_row = text->row + font_height - 1;
  if (max_row > (memory_rows - 1)) {
    max_row = memory_rows - 1;
  }

  // We now can setup our boundaries
  oledm_set_bounds(
      text->display,
      text->column,
      text->row,
      max_column,
      max_row);

  oledm_start_pixels(text->display);

  // Fill in the data, one row at a time
  const uint8_t height = max_row - text->row + 1;
  uint8_t row = 0;
  // This is the per-row level
  for (; row < height; ++row) {
    write_row_fixed(text, str, len, row);
  }

  oledm_stop(text->display);
  text->column = max_column + 1;
}

void text_clear_row(struct Text* text) {
  text_verifyFont(text);

  if (text->display->error) {
    // display is in an error state
    return;
  }

  const uint8_t memory_rows = text->display->memory_rows;
  const column_t memory_columns = text->display->memory_columns;

  if (text->column >= memory_columns) { 
    // already at the edge
    return;
  }

  if (text->row >= memory_rows) {
    // already off the bottom
    return;
  }

  const struct FontASCII* font = (struct FontASCII*)(text->font);
  const uint8_t font_height = pgm_read_byte_near(&font->height);
  const column_t start_column = text->column;
  uint8_t max_row = text->row + font_height - 1;
  if (max_row > (memory_rows - 1)) {
    max_row = memory_rows - 1;
  }

  oledm_set_bounds(
      text->display,
      text->column,
      text->row,
      memory_columns - 1,
      max_row);

  oledm_start_pixels(text->display);
  for (uint8_t row = text->row; row <= max_row; ++row) {
    for (text->column = start_column;
         text->column < memory_columns;
         ++text->column) {
        oledm_write_pixels(text->display, 0);
    }
  }
  oledm_stop(text->display);
}
