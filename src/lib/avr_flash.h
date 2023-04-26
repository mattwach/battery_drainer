// provide flash interface for both AVR and pico

#if defined(__AVR_ARCH__)
  #include <avr/pgmspace.h>
#elif defined(LIB_PICO_PLATFORM)
  #include <pico/platform.h>
  #define PROGMEM __in_flash()
  #define pgm_read_byte_near(addr) *(addr)
#endif

