add_executable(
    lipo_battery_drainer
    main.c
    console.c
    settings.c
)

target_link_libraries(
    lipo_battery_drainer
    pico_stdlib
    hardware_flash
    UART_CONSOLE
)

pico_add_extra_outputs(lipo_battery_drainer)


