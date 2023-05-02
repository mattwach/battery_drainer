#include "buttons.h"
#include "debounce/debounce.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define NEXT_BUTTON_GPIO 7
#define OK_BUTTON_GPIO 8

#define DEBOUNCE_MS 10

struct Debounce next_db;
struct Debounce ok_db;
volatile uint8_t button_bit_array;

static inline uint32_t uptime_ms() {
  return to_ms_since_boot(get_absolute_time());
}

// Callback for gpio_set_irq_callback.  Since it's an ISR,
// try to avoid expensive operations.
static void button_pressed_callback(uint gpio, uint32_t events) {
  switch (gpio) {
    case NEXT_BUTTON_GPIO:
      // The callback helper helps filter away debounce glitches
      // The debounce_gpio_irq_callback_helper has a side effect (updates state in
      // the debounce structure so it should always be called.
      if (debounce_gpio_irq_callback_helper(&next_db, uptime_ms(), events) &&
          next_db.val) {
        button_bit_array |= NEXT_PRESSED;
      }
      break;
    case OK_BUTTON_GPIO:
      // The comment for SELECT_BUTTON_GPIO above also apply here.
      if (debounce_gpio_irq_callback_helper(&ok_db, uptime_ms(), events) &&
          ok_db.val) {
        button_bit_array |= OK_PRESSED;
      }
      break;
  }
}

// Sets up a pin to sense a button press.  Hardware-wise the button is
// connected to the pin and to ground.  The code below sets the internal
// pullup for the button so the pin will sit at 3.3V when the button
// is not pressed.  When the button is pressed, the 3.3V will be pulled
// to ground which is an event that the pico is configured to recognize
// and raise an interrupt for.
static void setup_gpio(uint gpio) {
  gpio_init(gpio);
  gpio_set_dir(gpio, GPIO_IN);
  gpio_pull_up(gpio);
  sleep_ms(1);  // give the pullup some time to do it's thing. Maybe not needed.
  gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

void buttons_init(void) {
  setup_gpio(NEXT_BUTTON_GPIO);
  setup_gpio(OK_BUTTON_GPIO);
  irq_set_enabled(IO_IRQ_BANK0, true);
  debounce_init(&next_db, DEBOUNCE_MS);
  debounce_init(&ok_db, DEBOUNCE_MS);
  button_bit_array = 0x00;
}

void buttons_update(struct SharedState* ss) {
  ss->button = button_bit_array;
  button_bit_array = 0x00;  // reset
}

