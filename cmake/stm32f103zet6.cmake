add_library(bsp_intf INTERFACE)

target_sources(
    bsp_intf
    INTERFACE
        ${CMAKE_SOURCE_DIR}/firmware/cmsis/src/startup_stm32f10x_hd.s
)
