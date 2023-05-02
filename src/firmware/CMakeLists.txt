add_compile_options( -DI2C_WRITE_BUFFER_SIZE=2048 )

add_executable(
    lipo_battery_drainer
    buttons.c
    console.c
    main.c
    profile_selection.c
    settings.c
    state.c
)

target_link_libraries(
    lipo_battery_drainer
    FIXED_TEXT_TERMINUS8X16
    hardware_flash
    DEBOUNCE
    OLEDM_FIXED_TEXT
    OLEDM_I2C
    OLEDM_SH1106
    pico_stdlib
    UART_CONSOLE
)

pico_add_extra_outputs(lipo_battery_drainer)


