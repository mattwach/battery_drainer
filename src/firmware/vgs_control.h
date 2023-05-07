#ifndef VGS_CONTROL_H
#define VGS_CONTROL_H

#include <inttypes.h>

void vgs_control_init(void);

// level ranges from 0 (off) to 0xFFFF (both fets fully on)
// The response curve is not linear.
void vgs_control(uint16_t level);

#endif
