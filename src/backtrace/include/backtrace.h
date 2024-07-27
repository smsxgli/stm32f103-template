#ifndef BACKTRACE_H_DEFINED
#define BACKTRACE_H_DEFINED

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct backtrace_frame {
  uint32_t pc; /* r15 */
  uint32_t lr; /* r14 */
  uint32_t sp; /* r13 */
  uint32_t fp; /* r7 */
};

struct backtrace {
  const void *addr;
  const void *func;
  const char *func_name;
};

extern int backtrace_unwind_stack(struct backtrace_frame *,
                                  int (*cb)(const struct backtrace *));

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* BACKTRACE_H_DEFINED */
