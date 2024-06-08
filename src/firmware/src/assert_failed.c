#include "stm32f10x.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#if defined(USE_FULL_ASSERT)

void assert_failed(uint8_t *file, uint32_t line) {
  (void)fprintf(stderr,
                "stm32 std-peripheral lib assert failed at %s, line %" PRIu32
                "\n",
                file, line);
  abort();
}

#endif /* defined (USE_FULL_ASSERT) */

void fw_reset(void) {
  NVIC_SystemReset();
}
