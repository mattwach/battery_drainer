#include "uint16_avg.h"

#include <string.h>

void uint16_avg_init(struct Uint16Avg* u) {
  memset(u, 0, sizeof(*u));
}

void uint16_avg_add(struct Uint16Avg* u, uint16_t v) {
  u->total = u->total + v - u->sample[u->sample_index];
  u->sample[u->sample_index] = v;
  ++u->sample_index;
  if (u->sample_index >= NUM_SAMPLES) {
    u->sample_index = 0;
  }
}

