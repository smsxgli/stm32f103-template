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

include(${CMAKE_CURRENT_LIST_DIR}/cmake/cortex-m3.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/bsp.cmake)
include(FetchContent)

add_compile_options(
    ${ARCH_FLAGS}
    -Wall
    -Wextra
    -Wconversion
    -Wsign-conversion
    -Wpedantic
    -Wshadow
    -Wdouble-promotion
    -Wundef
    -Wpadded
    -Wformat=2
    -Wformat-overflow
    -Wformat-truncation
    -Werror=format-security
    -Wstack-usage=1024
    -fmessage-length=0
    -fdiagnostics-color=always
    -fstack-usage
    -fstack-protector-strong
    -ffunction-sections
    -fdata-sections
    -fno-common
    -fno-short-enums
    -MMD
    -MP
    -ggdb3
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Release>:-flto=auto>
    $<$<CONFIG:Release>:-fno-fat-lto-objects>
)

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

add_executable(${PROJECT_NAME})

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

target_link_libraries(${PROJECT_NAME} PRIVATE firmware rtt)

target_link_options(
    ${PROJECT_NAME}
    PRIVATE
    ${ARCH_FLAGS}
    $<$<CONFIG:Release>:-flto=auto>
    $<$<CONFIG:Release>:-fno-fat-lto-objects>
    --specs=nosys.specs
    --specs=nano.specs
    -nostartfiles
    -T${CMAKE_CURRENT_LIST_DIR}/lds/memory.lds
    -T${CMAKE_CURRENT_LIST_DIR}/lds/sections.lds
    LINKER:--gc-sections
    LINKER:--print-memory-usage
    LINKER:-Map,${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
    LINKER:-O1
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
        SUFFIX .elf
        LINK_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/lds/sections.lds;${CMAKE_CURRENT_LIST_DIR}/lds/memory.lds"
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
