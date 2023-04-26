add_compile_options( -DI2C_WRITE_BUFFER_SIZE=2048 )

add_executable(test_oled
        main.c
        )

# pull in common dependencies
target_link_libraries(test_oled
    pico_stdlib
    OLEDM_I2C
    OLEDM_SH1106
    OLEDM_FIXED_TEXT
    FIXED_TEXT_TERMINUS8X16
)

pico_add_extra_outputs(test_oled)
