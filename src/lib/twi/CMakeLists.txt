# This is only used with PI PICO and NOT AVR.
cmake_minimum_required(VERSION 3.12)

#project(TWI C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


add_library(TWI INTERFACE)
target_include_directories(TWI  INTERFACE ${CMAKE_CURRENT_LIST_DIR}/..)
target_link_libraries(TWI  INTERFACE pico_stdlib hardware_i2c)
target_sources(TWI  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/twi.h
    ${CMAKE_CURRENT_LIST_DIR}/twi_pi_pico.c
)

