/* SPDX-License-Identifier: MIT */
#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include <stddef.h>
#include <stdint.h>

extern int main(int argc, char *argv[]);
extern void _exit(int status) __attribute__((__noreturn__));

extern void _start(void)
    __attribute__((__noreturn__, __used__, __section__(".after_vectors")));
extern void Default_Handler(void)
    __attribute__((__used__, __section__(".after_vectors"), __weak__));
extern void NMI_Handler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void HardFault_Handler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void MemManage_Handler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void BusFault_Handler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void UsageFault_Handler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void SVC_Handler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DebugMon_Handler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void PendSV_Handler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void SysTick_Handler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void WWDG_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void PVD_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TAMPER_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void RTC_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void FLASH_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void RCC_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void EXTI0_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void EXTI1_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void EXTI2_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void EXTI3_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void EXTI4_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DMA1_Channel1_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DMA1_Channel2_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DMA1_Channel3_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DMA1_Channel4_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DMA1_Channel5_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DMA1_Channel6_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DMA1_Channel7_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void ADC1_2_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void USB_HP_CAN1_TX_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void USB_LP_CAN1_RX0_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void CAN1_RX1_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void CAN1_SCE_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void EXTI9_5_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM1_BRK_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM1_UP_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM1_TRG_COM_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM1_CC_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM2_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM3_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM4_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void I2C1_EV_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void I2C1_ER_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void I2C2_EV_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void I2C2_ER_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void SPI1_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void SPI2_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void USART1_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void USART2_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void USART3_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void EXTI15_10_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void RTCAlarm_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void USBWakeUp_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM8_BRK_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM8_UP_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM8_TRG_COM_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM8_CC_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void ADC3_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void FSMC_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void SDIO_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM5_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void SPI3_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void UART4_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void UART5_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM6_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void TIM7_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DMA2_Channel1_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DMA2_Channel2_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DMA2_Channel3_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));
extern void DMA2_Channel4_5_IRQHandler(void)
    __attribute__((__weak__, __alias__("Default_Handler")));

extern uint32_t _stack;

static inline void init_data(void);
static inline void init_bss(void);
static inline void init_hw_early(void);

static inline void run_init_array(void);
static inline void init_main_args(int *, char ***);
static inline void run_finit_array(void);

static void *isr_vector[121]
    __attribute__((__used__, __section__(".isr_vector")));

void _start(void) {
  /* prepare c runtime */
  init_data();
  init_bss();
  /* config clock and other stuff */
  init_hw_early();
  /* run init code before main */
  run_init_array();
  int argc;
  char **argv;
  init_main_args(&argc, &argv);
  int code = main(argc, argv);
  /* run finit code after main */
  run_finit_array();
  _exit(code);

  /* infinite loop here */
  for (;;) {
    __NOP();
  }

  __builtin_unreachable();
}

void init_data(void) {
  extern uint32_t _sidata, _sdata, _edata;

  const uint32_t *src = &_sidata;
  uint32_t *dst = &_sdata;

  while (dst < &_edata) {
    *dst++ = *src++;
  }
}

void init_bss(void) {
  extern uint32_t _sbss, _ebss;

  uint32_t *dst = &_sbss;

  while (dst < &_ebss) {
    *dst++ = 0;
  }
}

void init_hw_early(void) {
  SystemInit();
  SystemCoreClockUpdate();
}

void run_init_array(void) {
  extern void (*__init_array_start[])(void);
  extern void (*__init_array_end[])(void);

  const int count = __init_array_end - __init_array_start;

  for (int i = 0; i < count; i++) {
    __init_array_start[i]();
  }
}

void init_main_args(int *argc, char ***argv) {
  *argc = 0;
  *argv = NULL;
}

void run_finit_array(void) {
  extern void (*__fini_array_start[])(void);
  extern void (*__fini_array_end[])(void);

  const int count = __fini_array_end - __fini_array_start;

  for (int i = 0; i < count; i++) {
    __fini_array_start[i]();
  }
}

void Default_Handler(void) {
  for (;;) {
    __NOP();
  }
}

#define BOOT_RAM (void *)0xF1E0F85F

static void *isr_vector[121] = {&_stack,
                                _start,
                                NMI_Handler,
                                HardFault_Handler,
                                MemManage_Handler,
                                BusFault_Handler,
                                UsageFault_Handler,
                                0,
                                0,
                                0,
                                0,
                                SVC_Handler,
                                DebugMon_Handler,
                                0,
                                PendSV_Handler,
                                SysTick_Handler,
                                WWDG_IRQHandler,
                                PVD_IRQHandler,
                                TAMPER_IRQHandler,
                                RTC_IRQHandler,
                                FLASH_IRQHandler,
                                RCC_IRQHandler,
                                EXTI0_IRQHandler,
                                EXTI1_IRQHandler,
                                EXTI2_IRQHandler,
                                EXTI3_IRQHandler,
                                EXTI4_IRQHandler,
                                DMA1_Channel1_IRQHandler,
                                DMA1_Channel2_IRQHandler,
                                DMA1_Channel3_IRQHandler,
                                DMA1_Channel4_IRQHandler,
                                DMA1_Channel5_IRQHandler,
                                DMA1_Channel6_IRQHandler,
                                DMA1_Channel7_IRQHandler,
                                ADC1_2_IRQHandler,
                                USB_HP_CAN1_TX_IRQHandler,
                                USB_LP_CAN1_RX0_IRQHandler,
                                CAN1_RX1_IRQHandler,
                                CAN1_SCE_IRQHandler,
                                EXTI9_5_IRQHandler,
                                TIM1_BRK_IRQHandler,
                                TIM1_UP_IRQHandler,
                                TIM1_TRG_COM_IRQHandler,
                                TIM1_CC_IRQHandler,
                                TIM2_IRQHandler,
                                TIM3_IRQHandler,
                                TIM4_IRQHandler,
                                I2C1_EV_IRQHandler,
                                I2C1_ER_IRQHandler,
                                I2C2_EV_IRQHandler,
                                I2C2_ER_IRQHandler,
                                SPI1_IRQHandler,
                                SPI2_IRQHandler,
                                USART1_IRQHandler,
                                USART2_IRQHandler,
                                USART3_IRQHandler,
                                EXTI15_10_IRQHandler,
                                RTCAlarm_IRQHandler,
                                USBWakeUp_IRQHandler,
                                TIM8_BRK_IRQHandler,
                                TIM8_UP_IRQHandler,
                                TIM8_TRG_COM_IRQHandler,
                                TIM8_CC_IRQHandler,
                                ADC3_IRQHandler,
                                FSMC_IRQHandler,
                                SDIO_IRQHandler,
                                TIM5_IRQHandler,
                                SPI3_IRQHandler,
                                UART4_IRQHandler,
                                UART5_IRQHandler,
                                TIM6_IRQHandler,
                                TIM7_IRQHandler,
                                DMA2_Channel1_IRQHandler,
                                DMA2_Channel2_IRQHandler,
                                DMA2_Channel3_IRQHandler,
                                DMA2_Channel4_5_IRQHandler,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                BOOT_RAM};
