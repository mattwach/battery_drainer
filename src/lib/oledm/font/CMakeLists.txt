# This is only used with PI PICO and NOT AVR.
cmake_minimum_required(VERSION 3.12)

#project(TWI C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


add_library(FIXED_TEXT_TERMINUS8X16 INTERFACE)
target_link_libraries(FIXED_TEXT_TERMINUS8X16  INTERFACE)
target_sources(FIXED_TEXT_TERMINUS8X16  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/terminus8x16.h
    ${CMAKE_CURRENT_LIST_DIR}/terminus8x16.c
)
