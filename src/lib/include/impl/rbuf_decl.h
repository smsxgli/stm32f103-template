#ifndef RBUF_DECL_H_DEFINED
#define RBUF_DECL_H_DEFINED

#ifndef RBUF_H_DEFINED
#error do not include 'rbuf_decl.h' directly, include 'rrbuf.h' instead!
#endif /* RBUF_H_DEFINED */

#include "utils.h"

#define rbuf_put(_suffix) token_cat(rbuf_put_, _suffix)
#define rbuf_get(_suffix) token_cat(rbuf_get_, _suffix)

#define RBUF_DECL_PUT(_suffix, _val_t)                                         \
  extern void rbuf_put(_suffix)(char *__restrict, _val_t, _val_t,              \
                                const char *__restrict, _val_t)                \
      __attribute__((__nonnull__))
#define RBUF_DECL_GET(_suffix, _val_t)                                         \
  extern void rbuf_get(_suffix)(const char *__restrict, _val_t, _val_t,        \
                                char *__restrict, _val_t)                      \
      __attribute__((__nonnull__))

#define RBUF_DECL(_val_t, _suffix)                                             \
  RBUF_DECL_PUT(_suffix, _val_t);                                              \
  RBUF_DECL_GET(_suffix, _val_t)

#endif /* RBUF_DECL_H_DEFINED */
