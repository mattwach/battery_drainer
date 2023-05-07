#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include <inttypes.h>

void fan_control_init(void);

// level ranges from 0 (off) to 0xFFFF (fully on)
void fan_control(uint16_t level);

#endif
