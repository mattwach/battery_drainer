#include "twi.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
// GP4 - Pin6 is the default
#define SDA_PIN PICO_DEFAULT_I2C_SDA_PIN
// GP5 - Pin 7 is the default
#define SCL_PIN PICO_DEFAULT_I2C_SCL_PIN

#ifndef TWI_FREQ
#define TWI_FREQ 400000L
#endif

#ifndef SET_TWDR
#define SET_TWDR(v) TWDR = (v)
#endif

#ifndef GET_TWDR
#define GET_TWDR() TWDR
#endif

// As one might expect, the PI implementation does not matchup to this API
// very well, thus we need some global state to bridge the gap
#define I2C_IDLE 0
#define I2C_WRITING 1
#define I2C_READING 2
static uint8_t i2c_state;  // if true, last operation was a write
static uint8_t i2c_address;
#ifndef I2C_WRITE_BUFFER_SIZE
  #error Please define I2C_WRITE_BUFFER_SIZE to the largest known I2C write.
#endif
static uint8_t i2c_write_buffer[I2C_WRITE_BUFFER_SIZE];
static uint16_t i2c_write_buffer_count;  // number of bytes in buffer

void twi_init(void) {
  static uint8_t already_initialized = 0;
  if (!already_initialized) {
    already_initialized = 1;
    twi_reinit();
  }
}

void twi_reinit(void) {
  i2c_init(i2c0, TWI_FREQ);
  gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
  // needed in the rare case where no one provided any external pullups.
  //gpio_pull_up(SDA_PIN);
  //gpio_pull_up(SCL_PIN);
}

static void flushBuffer(error_t* err, bool nostop) {
  int werr = i2c_write_blocking(i2c0, i2c_address, i2c_write_buffer, i2c_write_buffer_count, nostop);
  i2c_write_buffer_count = 0;
  if (werr < 0) {
    *err = TWI_INTERNAL_ERROR;
  }
  if (!nostop) {
    i2c_state = I2C_IDLE;
  }
}

// Instructs the TWI hardware to send a stop
void twi_stop(error_t* err) {
  uint8_t dst;
  switch (i2c_state) {
    case I2C_IDLE:
      // do nothing
      break;
    case I2C_WRITING:
      flushBuffer(err, false);
      break;
    case I2C_READING:
      i2c_read_blocking(i2c0, i2c_address, &dst, 0, false);
      break;
  }
  i2c_state = I2C_IDLE;
}

void twi_startWrite(uint8_t address, error_t* err) {
  twi_stop(err);
  if (*err) {
    return;
  }
  i2c_state = I2C_WRITING;  
  i2c_address = address;
}

void twi_writeNoStop(uint8_t byte, error_t* err) {
  if (*err) {
    return;
  }
  if (i2c_state != I2C_WRITING) {
    *err = TWI_MISSING_START_CON_ERROR;
    return;
  }
  if (i2c_write_buffer_count == I2C_WRITE_BUFFER_SIZE) {
    *err = TWI_BUFFER_FULL;
    return;
  }
  i2c_write_buffer[i2c_write_buffer_count] = byte;
  ++i2c_write_buffer_count;
}


void twi_readNoStop(uint8_t address, uint8_t* data, uint8_t length, error_t* err) {
  if (i2c_state == I2C_WRITING) {
    flushBuffer(err, true);
  }
  if (*err) {
    return;
  }
  i2c_address = address;
  i2c_state = I2C_READING;
  int werr = i2c_read_blocking(i2c0, address, data, length, true);
  if (werr < 0) {
    *err = TWI_INTERNAL_ERROR;
  }
}
