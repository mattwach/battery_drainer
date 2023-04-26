#include <twi/twi.h>
#include <error_codes.h>
#include "oledm.h"

// Default address that has be changed by defining this constant when compiling
// this object.
#ifndef I2C_ADDRESS
#define I2C_ADDRESS 0x3C
#endif

#define COMMAND_BYTE 0x80  // Should this be 0x80?
#define DATA_BYTE 0x40

void oledm_ifaceInit(void) {
  twi_init();
}

void oledm_startCommands(error_t* err) {
  twi_startWrite(I2C_ADDRESS, err);
}

#ifndef SSD1680
void oledm_start_pixels(struct OLEDM* display) {
  error_t* err = &display->error;
  twi_startWrite(I2C_ADDRESS, err);
  twi_writeNoStop(DATA_BYTE, err);
}
#endif

void oledm_command(uint8_t cmd, error_t* err) {
  twi_writeNoStop(COMMAND_BYTE, err);
  twi_writeNoStop(cmd, err);
}

void oledm_ifaceWriteData(uint8_t data, error_t* err) {
  twi_writeNoStop(data, err);
}

void oledm_stop(struct OLEDM* display) {
  twi_stop(&(display->error));
}
