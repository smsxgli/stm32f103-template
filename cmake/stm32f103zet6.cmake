add_library(bsp_intf INTERFACE)

target_include_directories(
    bsp_intf
    INTERFACE
        ${CMAKE_SOURCE_DIR}/firmware/cmsis/include
)

target_compile_definitions(
    bsp_intf
    INTERFACE
        STM32F10X_HD
)
# we shall not compile this source file here
add_library(bsp_src INTERFACE)

target_sources(
    bsp_src
    INTERFACE
        ${CMAKE_SOURCE_DIR}/firmware/cmsis/src/startup_stm32f10x_hd.c
)
