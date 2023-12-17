cmake_minimum_required(VERSION 3.18...3.27)

project(
    demo
    VERSION 0.0.99
    LANGUAGES C
    DESCRIPTION "template project for stm32f103"
)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(${PROJECT_NAME})

include(${CMAKE_CURRENT_LIST_DIR}/cmake/compiler-flags.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/cortex-m3.cmake)
include(FetchContent)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/firmware)

FetchContent_Declare(
    RTT
    GIT_REPOSITORY https://github.com/SEGGERMicro/RTT.git
    GIT_TAG ff57c3d516ea59b0a2d4d5cafb33a48572b09c0b # V7.54
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
FetchContent_GetProperties(RTT)
if (NOT RTT_POPULATED)
    FetchContent_Populate(RTT)
    file(
        CREATE_LINK
            ${CMAKE_CURRENT_LIST_DIR}/cmake/CMakeListsForRTT.txt
            ${rtt_SOURCE_DIR}/CMakeLists.txt
        SYMBOLIC
        COPY_ON_ERROR
    )
endif()
add_subdirectory(${rtt_SOURCE_DIR})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/main.c
        ${CMAKE_CURRENT_LIST_DIR}/src/syscall.c
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        compiler_intf
        compiler_strict_intf
        arch_intf
        firmware
        rtt
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
