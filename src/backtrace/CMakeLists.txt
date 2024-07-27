add_library(app_backtrace OBJECT)

target_sources(
    app_backtrace
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/backtrace.c
)

target_include_directories(
    app_backtrace
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(
    app_backtrace
    PRIVATE
        compiler_intf
        compiler_strict_intf
        arch_intf
        app_arch
)

set_target_properties(
    app_backtrace
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
)
