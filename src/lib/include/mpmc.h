#ifndef MPMC_H_DEFINED
#define MPMC_H_DEFINED

#include "impl/mpmc_decl.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

MPMC_DECL(uint32_t, uint16_t, u32);
MPMC_DECL(uint64_t, uint32_t, u64);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* MPMC_H_DEFINED */
