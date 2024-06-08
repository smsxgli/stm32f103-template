
add_library(app_newlib OBJECT)

target_include_directories(
    app_newlib
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(
    app_newlib
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/syscall.c
        ${CMAKE_CURRENT_LIST_DIR}/src/startup.c
)

target_link_libraries(
    app_newlib
    PRIVATE
        compiler_intf
        compiler_strict_intf
        arch_intf
        app_log
        app_firmware
)

set_target_properties(
    app_newlib
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
)