#ifndef RBUF_H_DEFINED
#define RBUF_H_DEFINED

#include "impl/rbuf_decl.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

RBUF_DECL(uint16_t, u16);
RBUF_DECL(uint32_t, u32);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* RBUF_H_DEFINED */
