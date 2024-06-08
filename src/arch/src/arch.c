#include "arch.h"
#include <stdint.h>

static inline uint32_t arch_cm3_get_primask(void) {
  uint32_t val = 0;
  __asm__ volatile("mrs %0, primask" : "=r"(val));
  return val;
}

static inline void arch_cm3_set_primask(uint32_t val) {
  __asm__ volatile("msr primask, %0" : : "r"(val));
}

static inline uint32_t arch_cm3_get_faultmask(void) {
  uint32_t val = 0;
  __asm__ volatile("mrs %0, faultmask" : "=r"(val));
  return val;
}

static inline void arch_cm3_set_faultmask(uint32_t val) {
  __asm__ volatile("msr faultmask, %0" : : "r"(val));
}

static inline uint32_t arch_cm3_get_ipsr(void) {
  uint32_t val = 0;
  __asm__ volatile("mrs %0, ipsr" : "=r"(val));
  return val;
}

static inline void arch_cm3_disable_irq(void) { __asm__ volatile("cpsid i"); }

static inline void arch_cm3_disable_frq(void) { __asm__ volatile("cpsid f"); }

arch_irqsave_t arch_disable_irq(void) {
  uint32_t primask = arch_cm3_get_primask();
  arch_cm3_disable_irq();
  return (arch_irqsave_t){primask};
}

void arch_restore_irq(arch_irqsave_t *irq) { arch_cm3_set_primask(irq->inner); }

arch_irqsave_t arch_disable_frq(void) {
  uint32_t faultmask = arch_cm3_get_faultmask();
  arch_cm3_disable_frq();
  return (arch_irqsave_t){faultmask};
}

void arch_restore_frq(arch_irqsave_t *frq) {
  arch_cm3_set_faultmask(frq->inner);
}

bool arch_is_in_nmi(void) {
  uint32_t ipsr = 0;
  ipsr = arch_cm3_get_ipsr();
  return ipsr == 2;
}

bool arch_is_in_fault(void) {
  uint32_t ipsr = 0;
  ipsr = arch_cm3_get_ipsr();
  return ipsr >= 2 && ipsr <= 15;
}

bool arch_is_in_irq(void) {
  uint32_t ipsr = 0;
  ipsr = arch_cm3_get_ipsr();
  return ipsr > 15;
}

bool arch_is_in_thread(void) {
  uint32_t ipsr = 0;
  ipsr = arch_cm3_get_ipsr();
  return ipsr == 0;
}
