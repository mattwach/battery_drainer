#include "pico/stdlib.h"
#include <stdio.h>
#include "uart_console/console.h"

static void hello_cmd(uint8_t argc, char* argv[]) {
  printf("Hello World!\n");
}

struct ConsoleCallback callbacks[] = {
    {"hello", "Prints message", 0, hello_cmd},
};

// program entry point
int main() {
  struct ConsoleConfig cc;
  uart_console_init(&cc, callbacks, 1, CONSOLE_VT102);

  while (1) {
    uart_console_poll(&cc, "> ");
    sleep_ms(20);
  }
  return 0;
}
