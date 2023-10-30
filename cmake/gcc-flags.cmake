add_library(compiler_intf INTERFACE)

target_compile_options(
    compiler_intf
    INTERFACE
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
        # -fno-short-enums
        -pipe
        -MMD
        -MP
        -ggdb3
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Release>:-Os>
        $<$<CONFIG:Release>:-flto=auto>
        $<$<CONFIG:Release>:-fno-fat-lto-objects>
)

target_link_options(
    compiler_intf
    INTERFACE
        $<$<CONFIG:Release>:-flto=auto>
        $<$<CONFIG:Release>:-fno-fat-lto-objects>
        --specs=nosys.specs
        --specs=nano.specs
        -nostartfiles
        -T${CMAKE_SOURCE_DIR}/lds/memory.lds
        -T${CMAKE_SOURCE_DIR}/lds/sections.lds
        LINKER:--gc-sections
        LINKER:--print-memory-usage
        LINKER:-Map,${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
        LINKER:-O1
)

set(compiler_linkdeps ${CMAKE_SOURCE_DIR}/lds/sections.lds;${CMAKE_SOURCE_DIR}/lds/memory.lds)
