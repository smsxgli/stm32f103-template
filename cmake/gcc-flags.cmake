add_library(compiler_intf INTERFACE)
add_library(compiler_strict_intf INTERFACE)
# common compiler options for all targets
target_compile_options(
    compiler_intf
    INTERFACE
        -Wall
        -Wextra
        -Wpedantic
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
        $<$<COMPILE_LANGUAGE:C>:-funwind-tables>
        $<$<COMPILE_LANGUAGE:C>:-mpoke-function-name>
        -mno-unaligned-access # aligned access default
        # -fno-short-enums
        -pipe
        -MMD
        -MP
        -ggdb3
        --specs=nano.specs
        $<$<CONFIG:Debug>:-O1>
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-UNDEBUG>
        $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-fsanitize=undefined>
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-Os>
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-finline-small-functions>
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-findirect-inlining>
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-flto=auto>
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-fno-fat-lto-objects>
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-Werror=odr>
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-Werror=lto-type-mismatch>
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-Werror=strict-aliasing>
        $<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>
        $<$<COMPILE_LANGUAGE:C>:-Werror=implicit-function-declaration>
)
# more strict warnings
target_compile_options(
    compiler_strict_intf
    INTERFACE
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wdouble-promotion
        -Wundef
        # -Wpadded
)

target_link_options(
    compiler_intf
    INTERFACE
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-flto=auto>
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-fno-fat-lto-objects>
        --specs=nano.specs
        -nostartfiles
        -T${CMAKE_SOURCE_DIR}/lds/memory.lds
        -T${CMAKE_SOURCE_DIR}/lds/sections.lds
        LINKER:--gc-sections
        LINKER:--print-memory-usage
        LINKER:-Map,${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
        LINKER:-O2
)

set(compiler_linkdeps ${CMAKE_SOURCE_DIR}/lds/sections.lds;${CMAKE_SOURCE_DIR}/lds/memory.lds)
