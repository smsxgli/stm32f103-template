
add_library(app_log OBJECT)

target_include_directories(
    app_log
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(
    app_log
    PRIVATE
        $<$<STREQUAL:${LOG_BACKEND},rtt>:${CMAKE_CURRENT_LIST_DIR}/src/log_rtt.c>
)

target_link_libraries(
    app_log
    PRIVATE
        compiler_intf
        compiler_strict_intf
        arch_intf
        app_arch
        app_lib
)

set_target_properties(
    app_log
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
)