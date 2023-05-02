add_library(DEBOUNCE INTERFACE)
target_include_directories(DEBOUNCE  INTERFACE ${CMAKE_CURRENT_LIST_DIR}/..)
target_sources(DEBOUNCE  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/debounce.c
)

