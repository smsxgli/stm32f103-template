
add_library(app_arch OBJECT)

target_include_directories(
    app_arch
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(
    app_arch
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/arch.c
)

target_link_libraries(
    app_arch
    PRIVATE
        compiler_intf
        compiler_strict_intf
        arch_intf
)

set_target_properties(
    app_arch
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
)