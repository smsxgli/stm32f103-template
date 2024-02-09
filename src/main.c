#include "stm32f10x.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  printf("template project for stm32f103zet6, build data: %s, time: %s\n",
         __DATE__, __TIME__);
  for (;;) {
    __NOP();
  }
  return EXIT_SUCCESS;
}
