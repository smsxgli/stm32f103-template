#include "stm32f10x.h"

extern void hardware_init_hook(void);

uint32_t jiffies;

static inline void systick_init(void);

void hardware_init_hook(void) {
  SystemCoreClockUpdate();
  systick_init();
}

static void systick_init(void) {
  uint32_t reload = 0;
  uint32_t ctrl = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
  uint32_t calib = SysTick->CALIB;
  if (0 == (calib & SysTick_CALIB_NOREF_Msk)) {
    /* reference clock provided */
    ctrl &= ~SysTick_CTRL_CLKSOURCE_Msk;
  }
  if (0 == (calib & SysTick_CALIB_SKEW_Msk)) {
    /* TENMS value is exact */
    reload = calib & SysTick_CALIB_TENMS_Msk;
  } else {
    /* use processor's clock, 10ms */
    ctrl |= SysTick_CTRL_CLKSOURCE_Msk;
    reload = SystemCoreClock / 100000;
  }
  NVIC_SetPriority(SysTick_IRQn, (1U << __NVIC_PRIO_BITS) - 1);
  SysTick->LOAD = reload;
  SysTick->VAL = 0;
  SysTick->CTRL = ctrl;
}

void SysTick_Handler(void) {
  (void)__atomic_fetch_add(&jiffies, 1, __ATOMIC_RELEASE);
}
