add_executable(test_vsense
        main.c
        )

# pull in common dependencies
target_link_libraries(
    test_vsense
    UART_CONSOLE
    pico_stdlib
    hardware_gpio
    hardware_adc)

# enable usb output, disable uart output
pico_enable_stdio_usb(test_vsense 1)
pico_enable_stdio_uart(test_vsense 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(test_vsense)
