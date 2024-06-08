
add_library(app_firmware OBJECT)

target_include_directories(
    app_firmware
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(
    app_firmware
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/assert_failed.c
        ${CMAKE_CURRENT_LIST_DIR}/src/handlers.c
)

target_link_libraries(
    app_firmware
    PRIVATE
        compiler_intf
        compiler_strict_intf
        arch_intf
        firmware
)

set_target_properties(
    app_firmware
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
)