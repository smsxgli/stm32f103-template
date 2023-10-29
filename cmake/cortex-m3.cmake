add_library(arch_intf INTERFACE)

target_compile_options(
    arch_intf
    INTERFACE
        -mcpu=cortex-m3
        -mthumb
)


target_link_options(
    arch_intf
    INTERFACE
        -mcpu=cortex-m3
        -mthumb
)