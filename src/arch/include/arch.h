#ifndef ARCH_H_DEFINED
#define ARCH_H_DEFINED

#include "arch_feature.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct arch_irqsave arch_irqsave_t;

struct arch_irqsave {
  unsigned long inner;
};

extern arch_irqsave_t arch_disable_irq(void);
extern void arch_restore_irq(arch_irqsave_t *) __attribute__((__nonnull__));

extern arch_irqsave_t arch_disable_frq(void);
extern void arch_restore_frq(arch_irqsave_t *) __attribute__((__nonnull__));

extern bool arch_is_in_nmi(void);
extern bool arch_is_in_fault(void);
extern bool arch_is_in_irq(void);
extern bool arch_is_in_thread(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ARCH_H_DEFINED */
