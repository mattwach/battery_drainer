#ifndef UINT16_AVG
#define UINT16_AVG

#define NUM_SAMPLES 64

#include <inttypes.h>

struct Uint16Avg {
  uint16_t sample[NUM_SAMPLES];
  uint16_t sample_index;
  uint32_t total;
};

void uint16_avg_init(struct Uint16Avg* u);

void uint16_avg_add(struct Uint16Avg* u, uint16_t v);

static inline uint16_t uint16_avg_get(struct Uint16Avg* u) {
  return u->total / NUM_SAMPLES;
}

#endif
