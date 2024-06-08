#ifndef RBUF_IMPL_H_DEFINED
#define RBUF_IMPL_H_DEFINED

#include "rbuf.h"
#include "utils.h"
#include <stdint.h>
#include <string.h>

#ifndef rbuf_impl_val_t
#define rbuf_impl_val_t uint16_t
#endif

#ifndef rbuf_impl_suffix
#define rbuf_impl_suffix u16
#endif

void rbuf_put(rbuf_impl_suffix)(char *__restrict buf, rbuf_impl_val_t cap,
                                rbuf_impl_val_t in, const char *__restrict src,
                                rbuf_impl_val_t len) {
  rbuf_impl_val_t mask = cap - 1;
  rbuf_impl_val_t tmp = (cap - (in & mask));
  rbuf_impl_val_t pass1 = min(len, tmp);
  rbuf_impl_val_t pass2 = len - pass1;
  memcpy(&buf[in & mask], src, pass1);
  memcpy(buf, &src[pass1], pass2);
}

void rbuf_get(rbuf_impl_suffix)(const char *__restrict buf, rbuf_impl_val_t cap,
                                rbuf_impl_val_t out, char *__restrict dst,
                                rbuf_impl_val_t len) {
  rbuf_impl_val_t mask = cap - 1;
  rbuf_impl_val_t tmp = (cap - (out & mask));
  rbuf_impl_val_t pass1 = min(len, tmp);
  rbuf_impl_val_t pass2 = len - pass1;
  memcpy(dst, &buf[out & mask], pass1);
  memcpy(&dst[pass1], buf, pass2);
}

#undef RBUF_IMPL_H_DEFINED
#endif /* RBUF_IMPL_H_DEFINED */
