
add_library(app_sanitizer OBJECT)

target_include_directories(
    app_sanitizer
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(
    app_sanitizer
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/ubsan.c
)

target_link_libraries(
    app_sanitizer
    PRIVATE
        compiler_intf
        compiler_strict_intf
        arch_intf
)

set_target_properties(
    app_sanitizer
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
)
