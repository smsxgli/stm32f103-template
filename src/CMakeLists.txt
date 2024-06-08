add_subdirectory(arch)
add_subdirectory(lib)
add_subdirectory(log)
add_subdirectory(firmware)
add_subdirectory(newlib)

target_sources(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main.c
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        app_arch
        app_firmware
        app_lib
        app_log
        app_newlib
)
