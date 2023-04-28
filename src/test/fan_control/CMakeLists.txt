add_executable(test_fan_control
        main.c
        )

# pull in common dependencies
target_link_libraries(
    test_fan_control
    UART_CONSOLE
    pico_stdlib
    hardware_gpio
    hardware_pwm)

# enable usb output, disable uart output
pico_enable_stdio_usb(test_fan_control 1)
pico_enable_stdio_uart(test_fan_control 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(test_fan_control)
