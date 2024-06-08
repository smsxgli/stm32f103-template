#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern uint32_t jiffies;

static void msleep(uint32_t ms) {
  uint32_t now = __atomic_load_n(&jiffies, __ATOMIC_RELAXED);

  for (;;) {
    uint32_t cur = __atomic_load_n(&jiffies, __ATOMIC_RELAXED);
    if ((cur - now) * 10 >= ms) {
      break;
    }
  }
}

/* monitor rtt start */
int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  unsigned int sec = 0;
  printf("template project for stm32f103zet6, build data: %s, time: %s\n",
         __DATE__, __TIME__);
  for (;;) {
    printf("sec: %d\n", sec);
    msleep(1000);
    sec++;
  }
  return EXIT_SUCCESS;
}
