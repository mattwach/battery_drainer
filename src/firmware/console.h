#ifndef BATT_CONSOLE_H
#define BATT_CONSOLE_H

#include "settings.h"

void console_init(struct Settings* s);

void console_poll(void);

#endif

