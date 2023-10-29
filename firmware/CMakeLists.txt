cmake_minimum_required(VERSION 3.18...3.27)

project(
    firmware
    LANGUAGES C
)

include(${CMAKE_SOURCE_DIR}/cmake/bsp.cmake)

add_library(${PROJECT_NAME} OBJECT)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/periph/include
)

target_sources(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/cmsis/src/core_cm3.c
        ${CMAKE_CURRENT_LIST_DIR}/cmsis/src/system_stm32f10x.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/misc.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_adc.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_bkp.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_can.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_cec.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_crc.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_dac.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_dbgmcu.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_dma.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_exti.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_flash.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_fsmc.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_gpio.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_i2c.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_iwdg.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_pwr.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_rcc.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_rtc.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_sdio.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_spi.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_tim.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_usart.c
        ${CMAKE_CURRENT_LIST_DIR}/periph/src/stm32f10x_wwdg.c
)

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC
        $<$<CONFIG:Debug>:USE_FULL_ASSERT>
        USE_STDPERIPH_DRIVER
)

target_link_libraries(${PROJECT_NAME} PUBLIC bsp_intf PRIVATE compiler_intf arch_intf bsp_src)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
)
