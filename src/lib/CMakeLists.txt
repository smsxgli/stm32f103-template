
add_library(app_lib OBJECT)

target_include_directories(
    app_lib
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
)

target_sources(
    app_lib
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/mpmc.c
        ${CMAKE_CURRENT_LIST_DIR}/src/rbuf.c
)

target_link_libraries(
    app_lib
    PRIVATE
        compiler_intf
        compiler_strict_intf
        arch_intf
)

set_target_properties(
    app_lib
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
)