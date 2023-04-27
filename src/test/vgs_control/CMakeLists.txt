add_executable(test_vgs_control
        main.c
        )

# pull in common dependencies
target_link_libraries(
    test_vgs_control
    UART_CONSOLE
    pico_stdlib)

# enable usb output, disable uart output
pico_enable_stdio_usb(test_vgs_control 1)
pico_enable_stdio_uart(test_vgs_control 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(test_vgs_control)
