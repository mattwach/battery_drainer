#ifndef OLEDM_BITMAP_H
#define OLEDM_BITMAP_H

#include "oledm.h"
#include "text.h"

// Library for double-buffering and bitmaps
// 
// This library allows manipulations of graphics data in memory.  This has the
// advantage of easy compositions, for example, building an image from shapes.
// The big disadvantage is that AVR chips have little memory.  Using a bitmap of
// 128x64, for example, will require 128 * 8 = 1024 bytes of memory which is
// half of the available ram on an ATMega328P.
//
// You can also opt to make a bitmap that is smaller than the entire screen
// to save memory.  This can be useful if most of the screen can be done with
// simple direct commands but there is one area that you would like to
// customize
//
// I suggest always spending a few minutes to determine if a bitmap can be
// avoided.  Fro example, if you want to make a bar graph, this can be often
// done with direct draw commands, saving // memory
//
// Example Usage:
//
// uint8_t bitmap_data[128 * 8];
//
// void main(void) {
//   struct OLEDM display;
//   struct Bitmap bitmap;
//   bitmap.columns = 128;  // 128x64 bitmap
//   bitmap.rows = 8;
//   bitmap.data = bitmap_data;
//   oledm_init(&display);
//
//   // Clear the bitmap
//   bitmap_fill(&bitmap, 0);
//
//   // Draw a 100x100 grid on the bitmap with
//   // 10x10 pixel grid squares
//   int i=0;
//   for (; i <= 100; i += 10) {
//     bitmap_hline(&bitmap, 0, 100, i, bitmap_OR);
//     bitmap_vline(&bitmap, i, 0, 100, bitmap_OR);
//   }
//
//   // Copy the bitmap to the OLED
//   bitmap_render_fast(&display, &bitmap, 0, 0);
//
//   // hang forever.  Use power management to save on power in the real
//   // world
//   while (1) { };
// }

struct Bitmap {
  column_t columns;  // Number of display columns
  uint8_t rows;     // Number of display rows.  Each row has vertical pixels.
  uint8_t* data;    // location of bitmap data
};

// Copy bitmap to te display
// The "fast" version does not handle the case where the bitmap goes beyond
// the right side or bottom of the display.  The non-fast version does
// handle it and will clip the bitmap as needed
//   display: Where to write the bitmap
//   bitmap: The bitmap
//   column: Starting column
//   row: Starting row.  Note that each row contains 8 vertical pixels
void bitmap_render_fast(
    struct OLEDM* display,
    const struct Bitmap* bitmap,
    column_t column,
    uint8_t row); 
void bitmap_render(
    struct OLEDM* display,
    const struct Bitmap* bitmap,
    column_t column,
    uint8_t row); 

// Rendering operators.  These determine how new shapes merge with
// the exsiting pixels.
// b <- bitmap
// p <- incoming pixels

void bitmap_OR(uint8_t* b, uint8_t p);  // b | p
void bitmap_AND(uint8_t* b, uint8_t p);  // b & p
void bitmap_SET(uint8_t* b, uint8_t p);  // p
void bitmap_NSET(uint8_t* b, uint8_t p);  // ~p
void bitmap_NAND(uint8_t* b, uint8_t p);  // b & ~p
void bitmap_NOR(uint8_t* b, uint8_t p);  // b | ~p
void bitmap_XOR(uint8_t* b, uint8_t p);  // b ^ p
void bitmap_XNOR(uint8_t* b, uint8_t p);  // b ^ ~p

// Fills the bitmap with a given byte of data
void bitmap_fill(struct Bitmap* bitmap, uint8_t byte);

// Fills in a column of 8-bit data.  This function can shift between
// rows.
//   bitmap: The bitmap to draw to
//   x: x coordinate
//   y: y coordicate
//   byte: Data byte
//   bit_op: Bit operator.  e.g. bitmap_OR
void bitmap_byte(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    uint8_t byte,
    void (*bit_op)(uint8_t* b, uint8_t p));

// Draws a horizontal line
//   bitmap: The bitmap to draw to
//   x: starting x
//   y: y coordicate
//   w: Width (line draws to the right)
//   bit_op: Bit operator.  e.g. bitmap_OR
void bitmap_hline(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    column_t w,
    void (*bit_op)(uint8_t* b, uint8_t p));

// Draws a vertical line
//   bitmap: The bitmap to draw to
//   x: x coordinate
//   y: starting y coordinate
//   h: height
//   bit_op: Bit operator.  e.g. bitmap_OR
void bitmap_vline(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    uint8_t h,
    void (*bit_op)(uint8_t* b, uint8_t p));

// Draws a line.  Prefer bitmap_hline and bitmap_vline
// for best performance.
//   bitmap: The bitmap to draw to
//   x0, y0: starting coordinate
//   x1, y1: ending coordinate
//   bit_op: Bit operator.  e.g. bitmap_OR
void bitmap_line(
    struct Bitmap* bitmap,
    column_t x0,
    uint8_t y0,
    column_t x1,
    uint8_t y1,
    void (*bit_op)(uint8_t* b, uint8_t p));

// Draws a rectangle
//   bitmap: The bitmap to draw to
//   x: top x coordinate
//   y: left y coordinate
//   w: width
//   h: height
//   bit_op: Bit operator.  e.g. bitmap_OR
void bitmap_rect(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    column_t w,
    uint8_t h,
    void (*bit_op)(uint8_t* b, uint8_t p));
void bitmap_filled_rect(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    column_t w,
    uint8_t h,
    void (*bit_op)(uint8_t* b, uint8_t p));

// Draws an oval.  Preper bitmap_circle
// when drawing circles for a small code footprint.
//   bitmap: The bitmap to draw to
//   cx: Center x
//   cy: Center y
//   r: radius (circle only)
//   rx: 0 degree radius
//   ry: 90 degtee radius
//   bit_op: Bit operator.  e.g. bitmap_OR
void bitmap_oval(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    uint8_t rx,
    uint8_t ry,
    void (*bit_op)(uint8_t* b, uint8_t p));
void bitmap_filled_oval(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    uint8_t rx,
    uint8_t ry,
    void (*bit_op)(uint8_t* b, uint8_t p));
void bitmap_circle(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    uint8_t r,
    void (*bit_op)(uint8_t* b, uint8_t p));
void bitmap_filled_circle(
    struct Bitmap* bitmap,
    column_t cx,
    uint8_t cy,
    uint8_t r,
    void (*bit_op)(uint8_t* b, uint8_t p));


// Sets or clears a single point.  the _nocheck version is faster
// but does not do a bounds-check.
//   bitmap: The bitmap to draw to
//   x: X coordinate
//   y: Y coordinate
//   bit_op: Bit operator.  e.g. bitmap_OR
static inline void bitmap_point_nocheck(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  bit_op(bitmap->data + ((y >> 3) * bitmap->columns) + x, 1 << (y & 0x07));
}
void bitmap_point(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    void (*bit_op)(uint8_t* b, uint8_t p));

// Returns non-zero if the specified point is set, zero otherwise
// The _nocheck version skips boundary checks.  The slightly slower
// bitmap_is_set will return a zero if an out-of-bounds point
// is requested.
static inline uint8_t bitmap_is_set_nocheck(
    const struct Bitmap* bitmap,
    column_t x,
    uint8_t y) {
  return bitmap->data[(y >> 3) * bitmap->columns + x] & 1 << (y & 0x07);
}
bool_t bitmap_is_set(
    const struct Bitmap* bitmap,
    column_t x,
    uint8_t y);

// Sets or clears a given region using a "flood fill" algorithm.
//  bitmap: the bitmap
//  x: Starting x coordinate
//  y: Starting y coordinate
//  set: If != 0 then sert pixels, oterwise clear them
//  queue, queue_length: IMPORTANT: queue_length must be one of
//    one of 8, 16, 32, 64, 128, 256.
//    Flood fill needs some working memory to keep track
//    of regions-to-explore.  How much depends on the complexity of the
//    task.  Usually a queue_length of around 150 will work for most cases,
//    but much less for simple cases.  The amount of memory needed is
//    deterministic based on the bitmap contents and starting point.
//    Providing insufficient memory will result in the function returning
//    with some areas unfilled.
void bitmap_flood_fill(
    struct Bitmap* bitmap,
    column_t x,
    uint8_t y,
    bool_t set,
    uint16_t* queue,
    uint16_t queue_length);

// Writes a text string to a bitmap
// For performance, assumes that the provided font is good and supported
// (e.g. does not check) 
void bitmap_strLen(
    struct Bitmap* bitmap,
    const void* font,  // FontASCII
    const char* str,
    uint8_t length,
    column_t x,
    uint8_t y,
    void (*bit_op)(uint8_t* b, uint8_t p));
void bitmap_str(
    struct Bitmap* bitmap,
    const void* font,  // FontASCII
    const char* str,
    column_t x,
    uint8_t y,
    void (*bit_op)(uint8_t* b, uint8_t p));
// Pascal string.  Useful in combination with the pstr/pstr.h library,
// for printing formatted integers, etc.
static inline void bitmap_pstr(
    struct Bitmap* bitmap,
    const void* font,  // FontASCII
    const uint8_t* pstr,
    column_t x,
    uint8_t y,
    void (*bit_op)(uint8_t* b, uint8_t p)) {
  bitmap_strLen(
      bitmap,
      font,
      (const char*)(pstr + 1),
      *pstr,
      x,
      y,
      bit_op);
}

#endif
