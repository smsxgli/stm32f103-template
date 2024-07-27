#include "backtrace.h"
#include "arch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern uint32_t jiffies;
extern int _write(int fd, const char *ptr, size_t len);

static void msleep(uint32_t ms) {
  uint32_t now = __atomic_load_n(&jiffies, __ATOMIC_RELAXED);

  for (;;) {
    uint32_t cur = __atomic_load_n(&jiffies, __ATOMIC_RELAXED);
    if ((cur - now) * 10 >= ms) {
      break;
    }
  }
}

static int backtrace_print(const struct backtrace *bt) {
  char buf[64];
  (void)snprintf(buf, sizeof(buf), "%p - %s@%p\n", bt->addr, bt->func_name,
                 bt->func);
  _write(1, buf, strlen(buf));
  return 0;
}

static inline int __attribute__((__always_inline__)) dump_stack(void) {
  struct backtrace_frame frame = {0};
  register uint32_t pc = (uint32_t)arch_get_pc();
  frame.pc = pc;
  frame.lr = (uint32_t)__builtin_return_address(0);
  frame.sp = (uint32_t)__builtin_frame_address(0);
  frame.fp = (uint32_t)__builtin_frame_address(0);

  return backtrace_unwind_stack(&frame, backtrace_print);
}

static int ping(int ball) __attribute__((__noinline__));
static int pong(int ball) __attribute__((__noinline__));

static int ping(int ball) {
  if (ball > 0) {
    return pong(ball - 1);
  } else {
    return dump_stack();
  }
}

static int pong(int ball) {
  if (ball > 0) {
    return ping(ball - 1);
  } else {
    return dump_stack();
  }
}

/* monitor rtt start */
int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  unsigned int sec = 0;
  printf("template project for stm32f103zet6, build data: %s, time: %s\n",
         __DATE__, __TIME__);
  ping(10);
  for (;;) {
    printf("sec: %d\n", sec);
    msleep(1000);
    sec++;
  }
  return EXIT_SUCCESS;
}

void SysTick_Handler(void) {
  const char msg[] = "in systick:\n";
  (void)__atomic_fetch_add(&jiffies, 1, __ATOMIC_RELAXED);
  _write(1, msg, strlen(msg));
  dump_stack();
}
