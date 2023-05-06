add_compile_options( -DI2C_WRITE_BUFFER_SIZE=2048 )

add_executable(
    lipo_battery_drainer
    buttons.c
    console.c
    damage_warning.c
    draining_battery.c
    draining_battery_ui.c
    finished.c
    main.c
    power.c
    profile_selection.c
    settings.c
    settings_message.c
    state.c
    util.c
    voltage_sense.c
)

target_link_libraries(
    lipo_battery_drainer
    FIXED_TEXT_TERMINUS8X16
    hardware_adc
    hardware_flash
    DEBOUNCE
    OLEDM_FIXED_TEXT
    OLEDM_I2C
    OLEDM_SH1106
    pico_stdlib
    UART_CONSOLE
)

# enable usb output, disable uart output
pico_enable_stdio_usb(lipo_battery_drainer 1)
pico_enable_stdio_uart(lipo_battery_drainer 0)

pico_add_extra_outputs(lipo_battery_drainer)


