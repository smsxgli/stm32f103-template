#include "stm32f10x.h"
#include "firmware.h"
#include <string.h>

enum exc_pri {
  EXC_PRI_MEM_FAULT = 0,
  EXC_PRI_BUS_FAULT = 1,
  EXC_PRI_USG_FAULT = 2,
  EXC_PRI_SYS_TICK = 7,
  EXC_PRI_PEND_SV = 14,
  EXC_PRI_SVC = 15,
};

extern void hardware_init_hook(void);

uint32_t jiffies;

static void exc_pri_init(void);
static void systick_init(void);

void hardware_init_hook(void) {
  SystemCoreClockUpdate();
  exc_pri_init();
  systick_init();
}

static void exc_pri_init(void) {
  NVIC_SetPriority(MemoryManagement_IRQn, EXC_PRI_MEM_FAULT);
  NVIC_SetPriority(BusFault_IRQn, EXC_PRI_BUS_FAULT);
  NVIC_SetPriority(UsageFault_IRQn, EXC_PRI_USG_FAULT);
  NVIC_SetPriority(SysTick_IRQn, EXC_PRI_SYS_TICK);
  NVIC_SetPriority(PendSV_IRQn, EXC_PRI_PEND_SV);
  NVIC_SetPriority(SVCall_IRQn, EXC_PRI_SVC);
}

static void systick_init(void) {
  uint32_t reload = 0;
  uint32_t ctrl = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
  uint32_t calib = SysTick->CALIB;
  uint32_t clock;
  if (0 == (calib & SysTick_CALIB_SKEW_Msk)) {
    /* TENMS value is exact */
    reload = calib & SysTick_CALIB_TENMS_Msk;
  } else {
    /* calculate the calibration value */
    if (0 == (calib & SysTick_CALIB_NOREF_Msk)) {
      /* reference clock provided */
      ctrl &= ~SysTick_CTRL_CLKSOURCE_Msk;
      clock = SystemCoreClock / 8;
    } else {
      /* use processor's clock */
      ctrl |= SysTick_CTRL_CLKSOURCE_Msk;
      clock = SystemCoreClock;
    }
    /* use clock to get 10ms */
    reload = clock / 100;
  }
  SysTick->LOAD = reload;
  SysTick->VAL = 0;
  SysTick->CTRL = ctrl;
}

void SysTick_Handler(void) {
  (void)__atomic_fetch_add(&jiffies, 1, __ATOMIC_RELAXED);
}
