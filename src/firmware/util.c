#include "util.h"
#include "state.h"

void fill_line(char* line, const char* str) {
  uint8_t i = 0;
  for (; i<OLED_COLUMNS; ++i) {
    if (!str[i]) {
      break;
    }
    line[i] = str[i];
  }
  for (; i<OLED_COLUMNS; ++i) {
    line[i] = ' ';
  }
}

