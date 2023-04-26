#include "bitmap.h"
#include "oledm.h"
#include "avr_flash.h"
#include <string.h>

void bitmap_render_fast(
    struct OLEDM* display, const struct Bitmap* bitmap, column_t column, uint8_t row) {
  oledm_set_bounds(
      display,
      column,
      row,
      column + bitmap->columns - 1,  // inclusive
      row + bitmap->rows - 1);  // also inclusive
  oledm_start_pixels(display);
  uint8_t* ptr = bitmap->data;
  const uint8_t* end = bitmap->data + (bitmap->rows * bitmap->columns);
  for (; ptr != end; ++ptr) {
    oledm_write_pixels(display, *ptr);
  }
  oledm_stop(display);
}

void bitmap_render(
    struct OLEDM* display, const struct Bitmap* bitmap, column_t column, uint8_t row) {
  column_t max_column = column + bitmap->columns;
  if (max_column >= display->visible_columns) {
    max_column = display->visible_columns;
  }

  uint8_t max_row = row + bitmap->rows;
  if (max_row >= display->visible_rows) {
    max_row = display->visible_rows;
  }

  uint8_t bitmap_row = 0;
  for (; row < max_row; ++row, ++bitmap_row) {
    oledm_set_bounds(display, column, row, max_column, row);
    oledm_start_pixels(display);
    uint8_t* ptr = bitmap->data + (bitmap_row * bitmap->columns);
    const uint8_t* end = ptr + (max_column - column);
    for (; ptr != end; ++ptr) {
      oledm_write_pixels(display, *ptr);
    }
    oledm_stop(display);
  }
}

void bitmap_fill(struct Bitmap* bitmap, uint8_t byte) {
  uint8_t* ptr = bitmap->data;
  uint8_t* end = bitmap->data + (bitmap->rows * bitmap->columns);
  for (; ptr < end; ++ptr) {
    *ptr = byte;
  }
}

void bitmap_OR(uint8_t* b, uint8_t p) {
		*b |= p;
}

void bitmap_AND(uint8_t* b, uint8_t p) {
		*b &= p;
}

void bitmap_SET(uint8_t* b, uint8_t p) {
		*b = p;
}

void bitmap_NSET(uint8_t* b, uint8_t p) {
		*b = ~p;
}

void bitmap_NAND(uint8_t* b, uint8_t p) {
		*b &= ~p;
}

void bitmap_NOR(uint8_t* b, uint8_t p) {
		*b |= ~p;
}

void bitmap_XOR(uint8_t* b, uint8_t p) {
		*b ^= p;
}

void bitmap_XNOR(uint8_t* b, uint8_t p) {
		*b ^= ~p;
}

static void _bitmap_hline_nocheck(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    column_t w,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  const uint8_t row_bit = y & 0x07;
  uint8_t* ptr = bitmap->data + ((y >> 3) * bitmap->columns) + x;
  const uint8_t* end = ptr + w;
  for (; ptr != end; ++ptr) {
    bit_op(ptr, 1 << row_bit);
  }
}

void bitmap_hline(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    column_t w,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  if (x >= bitmap->columns) {
    return;
  }
  if (y >= (bitmap->rows << 3)) {
    return;
  }
  if ((x + w) > bitmap->columns) {
    w = bitmap->columns - x;
  }
  _bitmap_hline_nocheck(bitmap, x, y, w, bit_op);
}

void _bitmap_vline_nocheck(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    uint8_t h,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  // First row, which may be partial
  uint8_t row_bit = 1 << (y & 0x07);
  uint8_t* ptr = bitmap->data + ((y >> 3) * bitmap->columns) + x; 
  uint8_t mask = 0x00;
  if ((y & 0x07) || (h & 0x07)) { // Skip to full rows if a multiple of 8
    for (; row_bit && h; row_bit <<= 1, --h) {
      mask |= row_bit;
    }
    bit_op(ptr, mask);
    ptr += bitmap->columns;
  }

  // Now the "middle" rows, if they exist.  These are
  // full 8 bit masks
  for (; h >= 8; h -= 8) {
    bit_op(ptr, 0xFF);
    ptr += bitmap->columns;
  }

  // Finally, finish off the final row as needed
  if (h) {
    row_bit = 0x01;
    mask = 0x00;
    for (; h; row_bit <<= 1, --h) {
      mask |= row_bit;
    }
    bit_op(ptr, mask);
  }
}

void bitmap_vline(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    uint8_t h,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  if (x >= bitmap->columns) {
    return;
  }
  const uint8_t height = bitmap->rows << 3;
  if (y >= height) {
    return;
  }
  if ((y + h) > height) {
     h = height - y;
  }
  _bitmap_vline_nocheck(bitmap, x, y, h, bit_op);
}

void bitmap_line(
    struct Bitmap* bitmap,
    column_t x0,
    uint8_t y0,
    column_t x1,
    uint8_t y1,
    void (*bit_op)(uint8_t* b, uint8_t p)) {

  const int8_t xdir = x1 > x0 ? 1 : -1;
  const int8_t ydir = y1 > y0 ? 1 : -1;

  const int16_t dx = (x1 - x0) * xdir;
  const int8_t dy = (y1 - y0) * ydir;
  const uint8_t height = bitmap->rows << 3;

  int16_t D = 2 * dy - dx;
  column_t x = x0;
  uint8_t y = y0;
  uint8_t row = y >> 3;
  uint8_t row_bit = y & 0x07;

  if (x < bitmap->columns && y < height) {
    bit_op(bitmap->data + (row * bitmap->columns) + x, 1 << row_bit);
  }

  while (x != x1 || y != y1) {
    if (D > 0) {
      y += ydir;
      row = y >> 3;
      row_bit = y & 0x07;
      D -= 2 * dx;
    } 
    
    if (D <= 0) {
      x += xdir;
      D += 2 * dy;
    }

    if (x >= bitmap->columns || y >= height) {
      // off the edge
      break;
    }

    bit_op(bitmap->data + (row * bitmap->columns) + x, 1 << row_bit);
  }
}

void bitmap_rect(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    column_t w,
    uint8_t h,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  if ((w == 0) || (h == 0)) {
    return;
  } 
  if (x >= bitmap->columns) {
    return;
  }
  const uint8_t height = bitmap->rows << 3;
  if (y >= height) {
    return;
  }

  uint8_t rh = h;
  if ((y + rh) > height) {
    // Constrain height
    rh = height - y;
  }
  // draw the left side
  _bitmap_vline_nocheck(bitmap, x, y, rh, bit_op);
  if (w < 2) {
    // one pixel wide
    return;
  }

  if ((x + w) <= bitmap->columns) {
    // draw the right side
    _bitmap_vline_nocheck(bitmap, x + w - 1, y, rh, bit_op);
  }

  if (w < 3) {
    // no middle gap
    return;
  }

  // account for drawn vertical lines
  ++x;
  if (x >= bitmap->columns) {
    // nothing to draw
    return;
  }
  column_t rw = w - 2;
  if ((x + rw) > bitmap->columns) {
    rw = bitmap->columns - x;
  }

  // draw top line
  _bitmap_hline_nocheck(bitmap, x, y, rw, bit_op);
  
  if (h < 2) {
    // done
    return;
  }
  if ((y + h) <= height) {
    // draw the bottom line
    _bitmap_hline_nocheck(bitmap, x, y + h - 1, rw, bit_op);
  }
}

static void fill_row(
    uint8_t* ptr,
    column_t w,
    uint8_t byte,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  const uint8_t* end_ptr = ptr + w;
  for (; ptr < end_ptr; ++ptr) {
    bit_op(ptr, byte);
  }
}

void bitmap_filled_rect(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    column_t w,
    uint8_t h,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  const uint8_t height = bitmap->rows << 3;
  if (w == 0 || h == 0 || x >= bitmap->columns || y >= height) {
    // nothing to do
    return;
  }

  if ((x + w) >= bitmap->columns) {
    w = bitmap->columns - x;
  }

  if ((y + h) >= height) {
    h = height - y;
  }

  // First row, which may be partial
  uint8_t row_bit = 1 << (y & 0x07);
  uint8_t* ptr = bitmap->data + ((y >> 3) * bitmap->columns) + x; 
  uint8_t mask = 0x00;
  if ((y & 0x07) || (h & 0x07)) { // Skip to full rows if a multiple of 8
    for (; row_bit && h; row_bit <<= 1, --h) {
      mask |= row_bit;
    }
    fill_row(ptr, w, mask, bit_op);
    ptr += bitmap->columns;
  }

  // Now the "middle" rows, if they exist.  These are
  // full 8 bit masks
  for (; h >= 8; h -= 8) {
    fill_row(ptr, w, 0xFF, bit_op);
    ptr += bitmap->columns;
  }

  // Finally, finish off the final row as needed
  if (h) {
    row_bit = 0x01;
    mask = 0x00;
    for (; h; row_bit <<= 1, --h) {
      mask |= row_bit;
    }
    fill_row(ptr, w, mask, bit_op);
  }
}

static void _symmetric_point(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    column_t dx,
    uint8_t dy,
    void (*bit_op)(uint8_t* b, uint8_t p)) {

  const column_t left = cx - dx;
  const uint8_t top = cy - dy;
  const column_t right = cx + dx;
  const uint8_t bottom = cy + dy;
  const uint8_t bm_height = bitmap->rows << 3;

  if (dx > 0) {
    if ((cy >= dy) && (cx >= dx)) {
      bitmap_point_nocheck(bitmap, left, top, bit_op);  // Top left
    }
    if ((bottom < bm_height) && (right < bitmap->columns)) {
      bitmap_point_nocheck(bitmap, right, bottom, bit_op);  // Bottom right
    }
  }
  
  if (dy > 0) {
    if ((cy >= dy) && (right < bitmap->columns)) {
      bitmap_point_nocheck(bitmap, right, top, bit_op);  // Top right
    }
    if ((bottom < bm_height) && (cx >= dx)) {
      bitmap_point_nocheck(bitmap, left, bottom, bit_op);  // Bottom left
    }
  }
}

static void _symmetric_hfill(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    column_t dx,
    uint8_t dy,
    void (*bit_op)(uint8_t* b, uint8_t p)) {

  const column_t left = dx > cx ? 0 : cx - dx;
  const column_t right = cx + dx >= bitmap->columns ? bitmap->columns - 1 : cx + dx;

  bool_t drawn = 0;
  const uint8_t top = cy - dy;
  if (dy <= cy) {
    _bitmap_hline_nocheck(bitmap, left, top, right - left + 1, bit_op);
    drawn = 1;
  }

  const uint8_t bottom = cy + dy;
  if (bottom < (bitmap->rows << 3) && ((top != bottom) || !drawn)) {
    _bitmap_hline_nocheck(bitmap, left, bottom, right - left + 1, bit_op);
  }
}

static void _next_arc_point(uint16_t rsq, column_t* dx, uint8_t* dy) {
  // Three possibilities:
  //  left, (dx--)
  //  up, (dy--)
  //  left,up (dx--. dy--)
  //
  // Each will have an associated error vs rsq.  We pick the choice with the
  // lowest error.

  // calc dx*dx and (dx-1) * (dx-1)
  int16_t d = (int16_t)(*dx);
  const int16_t dxsq = d * d;
  --d;
  const int16_t dxmmsq = d * d;

  // calc dy*dy and (dy-1) * (dy-1)
  d = (int16_t)(*dy);
  const int16_t dysq = d * d;
  ++d;
  const int16_t dyppsq = d * d;

  // calc each error
  int16_t up_err = dxsq + dyppsq;
  int16_t left_err = dxmmsq + dysq;
  int16_t up_left_err = dxmmsq + dyppsq;

  up_err = up_err > rsq ? up_err - rsq : rsq - up_err;
  left_err = left_err > rsq ? left_err - rsq : rsq - left_err;
  up_left_err = up_left_err > rsq ? up_left_err - rsq : rsq - up_left_err;

  // find the minimum error
  if ((up_left_err <= up_err) && (up_left_err <= left_err)) {
    --(*dx);
    ++(*dy);
  } else if ((up_err <= left_err) && (up_err <= up_left_err)) {
    // up has the smallest error
    ++(*dy);
  } else {
    // left has the smallest error
    --(*dx);
  }
}

void _draw_circle(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    uint8_t r,
    bool_t filled,
    void (*bit_op)(uint8_t* b, uint8_t p),
    void (*fill_op)(
        struct Bitmap* bitmap,
        column_t cx,
        uint8_t cy,
        column_t dx,
        uint8_t dy,
        void (*bit_op)(uint8_t* b, uint8_t p))) {
  if (r == 0) {
    return;
  }

  // find the center and radius squared
  uint16_t rsq = r * r;

  // circles have symmetry everywhere so we can just calculate
  // a small part and use simple reflection to find other parts
  // for this one, we'll calculate 0-90 degrees
  column_t dx = r;
  uint8_t dy = 0;
  uint8_t last_dy = dy + 1;

  while ((dy < r) || (dx > 0)) {
    if (!filled || (last_dy != dy)) {
      fill_op(bitmap, cx, cy, dx, dy, bit_op);
    }
    last_dy = dy;
    _next_arc_point(rsq, &dx, &dy);
  }
  if (!filled || (last_dy != dy)) {
    fill_op(bitmap, cx, cy, dx, dy, bit_op);
  }
}

void _draw_wide_oval(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    uint8_t rx,
    uint8_t ry,
    bool_t filled,
    void (*bit_op)(uint8_t* b, uint8_t p),
    void (*fill_op)(
        struct Bitmap* bitmap,
        column_t cx,
        uint8_t cy,
        column_t dx,
        uint8_t dy,
        void (*bit_op)(uint8_t* b, uint8_t p))) {
  if ((rx == 0) || (ry == 0)) {
    return;
  }

  // w is our guiding circle, then scale for h
  uint16_t rsq = rx * rx;

  column_t dx = rx;
  uint8_t dy = 0;
  uint16_t sdy = 0;
  column_t last_dx = dx + 1;
  uint8_t last_sdy = sdy + 1;

  while ((dy < rx) || (dx > 0)) {
    if ((!filled && (last_dx != dx)) || (last_sdy != sdy)) {
      fill_op(bitmap, cx, cy, dx, sdy, bit_op);
      last_dx = dx;
      last_sdy = sdy;
    }
    _next_arc_point(rsq, &dx, &dy);
    sdy = ((uint16_t)dy * ry + 1) / rx;
  }
  if ((!filled && (last_dx != dx)) || (last_sdy != sdy)) {
    fill_op(bitmap, cx, cy, dx, sdy, bit_op);
  }
}

void _draw_narrow_oval(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    uint8_t rx,
    uint8_t ry,
    bool_t filled,
    void (*bit_op)(uint8_t* b, uint8_t p),
    void (*fill_op)(
        struct Bitmap* bitmap,
        column_t cx,
        uint8_t cy,
        column_t dx,
        uint8_t dy,
        void (*bit_op)(uint8_t* b, uint8_t p))) {
  if ((rx == 0) || (ry == 0)) {
    return;
  }

  // h is our guiding circle, then scale for w
  uint16_t rsq = ry * ry;

  column_t dx = rx;
  uint8_t dy = 0;
  column_t sdx = ((uint16_t)dx * rx + 1) / ry;
  column_t last_sdx = sdx + 1;
  uint8_t last_dy = dy + 1;

  while ((dy < rx) || (dx > 0)) {
    if ((!filled && (last_sdx != sdx)) || (last_dy != dy)) {
      fill_op(bitmap, cx, cy, sdx, dy, bit_op);
      last_sdx = sdx;
      last_dy = dy;
    }
    _next_arc_point(rsq, &dx, &dy);
    sdx = ((uint16_t)dx * rx + 1) / ry;
  }
  if ((!filled && (last_sdx != sdx)) || (last_dy != dy)) {
    fill_op(bitmap, cx, cy, sdx, dy, bit_op);
  }
}

void bitmap_oval(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    uint8_t rx,
    uint8_t ry,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  if (rx == ry) {
    _draw_circle(bitmap, cx, cy, rx, FALSE, bit_op, _symmetric_point);
  } else if (rx > ry) {
    _draw_wide_oval(bitmap, cx, cy, rx, ry, FALSE, bit_op, _symmetric_point);
  } else {
    _draw_narrow_oval(bitmap, cx, cy, rx, ry, FALSE, bit_op, _symmetric_point);
  }
}

void bitmap_filled_oval(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    uint8_t rx,
    uint8_t ry,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  if (rx == ry) {
    _draw_circle(bitmap, cx, cy, rx, TRUE, bit_op, _symmetric_hfill);
  } else if (rx > ry) {
    _draw_wide_oval(bitmap, cx, cy, rx, ry, TRUE, bit_op, _symmetric_hfill);
  } else {
    _draw_narrow_oval(bitmap, cx, cy, rx, ry, TRUE, bit_op, _symmetric_hfill);
  }
}

void bitmap_circle(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    uint8_t r,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  _draw_circle(bitmap, cx, cy, r, FALSE, bit_op, _symmetric_point);
}

void bitmap_filled_circle(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    uint8_t r,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  _draw_circle(bitmap, cx, cy, r, TRUE, bit_op, _symmetric_hfill);
}

void bitmap_point(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  if (x >= bitmap->columns) {
    return;
  }
  if (y >= (bitmap->rows << 3)) {
    return;
  }
  bitmap_point_nocheck(bitmap, x, y, bit_op);
}

bool_t bitmap_is_set(
    const struct Bitmap* bitmap,
    column_t x,
    uint8_t y) {
  if (x >= bitmap->columns) {
    return FALSE;
  }
  if (y >= (bitmap->rows << 3)) {
    return FALSE;
  }
  return bitmap_is_set_nocheck(bitmap, x, y);
}

void bitmap_byte(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    uint8_t byte,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  if (x >= bitmap->columns) {
    return;
  }
  const uint8_t row = y >> 3;
  if (row >= bitmap->rows) {
    return;
  }
  uint8_t* ptr = bitmap->data + ((y >> 3) * bitmap->columns) + x;
  const uint8_t shift = y & 7;
  bit_op(ptr, byte << shift);
  if (shift && ((row + 1) < bitmap->rows)) {
    // bottom half
    ptr += bitmap->columns;
    bit_op(ptr, byte >> (8 - shift));
  }
}

static void _check_fill_point(
  const struct Bitmap* bitmap,
  uint16_t* queue,
  uint8_t queue_mask,
  uint8_t head,
  uint8_t* tail,
  column_t x,
  uint8_t y,
  bool_t set) {
  const uint8_t end = *tail;
  if (((end + 1) & queue_mask) == head) {
    // queue is full
    return;
  }

  if ((bitmap_is_set_nocheck(bitmap, x, y) != 0) == set) {
    // occupied
    return;
  }

  const uint16_t value = (y << 8) | x;

  //uint8_t idx = head;
  //for (; idx != end; idx = (idx + 1) & queue_mask) {
  //  if (queue[idx] == value) {
  //    // already in the queue
  //    return;
  //  }
  //}

  // Add it
  queue[end] = value;
  *tail = (end + 1) & queue_mask;
}

void bitmap_flood_fill(
    struct Bitmap* bitmap,
    column_t startx,
    uint8_t starty,
    bool_t set,
    uint16_t* queue,
    uint16_t queue_length) {
  if (queue_length < 8 || queue_length > 256) {
    return;
  }
  const uint8_t queue_mask = queue_length == 256 ? 0xFF : queue_length - 1;
  if (queue_length & queue_mask) {
    // not a power of 2
    return;
  }

  // normalize set
  set = set != 0;

  // initialize with x,y
  uint8_t head = 0;
  uint8_t tail = 1;
  queue[0] = starty << 8 | startx;

  const column_t bm_width_max = bitmap->columns - 1;
  const uint8_t bm_height_max = (bitmap->rows << 3) - 1;

  while (head != tail) {
    const column_t x = queue[head] & 0xFF;
    const uint8_t y = queue[head] >> 8;
    head = (head + 1) & queue_mask;

    if ((bitmap_is_set_nocheck(bitmap, x, y) != 0) == set) {
      // this point is already in the desired state
      continue;
    }

    // set the point
    if (set) {
      bitmap->data[(y >> 3) * bitmap->columns + x] |= 1 << (y & 0x07);
    } else {
      bitmap->data[(y >> 3) * bitmap->columns + x] &= ~(1 << (y & 0x07));
    }

    // left
    if (x > 0) {
      _check_fill_point(bitmap, queue, queue_mask, head, &tail, x-1, y, set);
    }

    // right
    if (x < bm_width_max) {
      _check_fill_point(bitmap, queue, queue_mask, head, &tail, x+1, y, set);
    }

    // up
    if (y > 0) {
      _check_fill_point(bitmap, queue, queue_mask, head, &tail, x, y - 1, set);
    }

    // down
    if (y < bm_height_max) {
      _check_fill_point(bitmap, queue, queue_mask, head, &tail, x, y + 1, set);
    }
  }
 }

void bitmap_strLen(
    struct Bitmap* bitmap,
    const void* fontv,
    const char* str,
    uint8_t len,
    column_t x,
    uint8_t y,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  if (len == 0) {
    // Nothing to do
    return;
  }

		const struct FontASCII* font = (const struct FontASCII*)fontv;
  const uint8_t font_width = pgm_read_byte_near(&font->width);
  const uint8_t font_first_char = pgm_read_byte_near(&font->first_char);
  const uint8_t font_last_char = pgm_read_byte_near(&font->last_char);
  const uint8_t font_height = pgm_read_byte_near(&font->height);
  const uint8_t bytes_per_char = font_width * font_height;

  uint8_t row = 0;
  uint8_t* row_start = bitmap->data + (y * bitmap->columns) + x; 
  uint8_t rowy = y;
  const char* ends = str + len;
  // Write each text row
  for (; row < font_height; ++row, rowy += 8, row_start += bitmap->columns) {
    uint8_t* ptr = row_start;
    const char* strp = str;
    const uint8_t row_offset = row * font_width;
    column_t rowx = x;

    // Write each character in the string
    for (; strp < ends; ++strp) {
      uint8_t j = 0;
      const char c = *strp;

      // Write each pixel row in the character
      for (; j < font_width; ++j, ++ptr, ++rowx) {
        uint8_t b = 0xFF;
        if (c == ' ') {
          b = 0x00;  // Space character
        } else if ((c >= font_first_char) && (c <= font_last_char)) {
          b = pgm_read_byte_near(
              font->data +                                // baseline
              ((c - font_first_char) * bytes_per_char) +  // char offset
              row_offset +
              j);
        }
        bitmap_byte(bitmap, rowx, rowy, b, bit_op);
      }
    }
  }
}

void bitmap_str(
    struct Bitmap* bitmap,
    const void* fontv,
    const char* str,
    column_t x,
    uint8_t y,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  bitmap_strLen(
    bitmap,
    fontv,
    str,
    strlen(str),
    x,
    y,
    bit_op);
}
