cmake_minimum_required(VERSION 3.18...3.28)

project(
    demo
    VERSION 0.0.99
    LANGUAGES ASM C
    DESCRIPTION "template project for stm32f103"
)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(LOG_BACKEND rtt)

add_executable(${PROJECT_NAME})

include(${CMAKE_CURRENT_LIST_DIR}/cmake/compiler-flags.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/cortex-m3.cmake)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/firmware)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        compiler_intf
        compiler_strict_intf
        arch_intf
        firmware
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
        LINK_DEPENDS "${compiler_linkdeps}"
        SUFFIX .elf
)

if (EXISTS ${CMAKE_OBJCOPY})
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}>
                ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_NAME:${PROJECT_NAME}>.hex
    )
endif()

if (EXISTS ${CMAKE_SIZE})
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE} --format=berkeley -x $<TARGET_FILE:${PROJECT_NAME}>
    )
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src)
