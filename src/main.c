#include "SEGGER_RTT.h"
#include "stm32f10x.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  SEGGER_RTT_Init();
  printf("template project for stm32f103zet6, build data: %s, time: %s\n",
         __DATE__, __TIME__);
  for (;;) {
    __NOP();
  }
  return EXIT_SUCCESS;
}

void assert_failed(uint8_t *file, uint32_t line) {
  printf("stm32 std-peripheral lib assert failed at %s, line %" PRIu32 "\n",
         file, line);
  abort();
}
