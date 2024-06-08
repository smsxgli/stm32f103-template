#ifndef MPMC_DECL_H_DEFINED
#define MPMC_DECL_H_DEFINED

#ifndef MPMC_H_DEFINED
#error do not include 'mpmc_decl.h' directly, include 'mpmc.h' instead!
#endif /* MPMC_H_DEFINED */

#include "utils.h"
#include <stdalign.h>
#include <stdbool.h>

#define mpmc_rts_pc_t(_suffix) union token_cat(mpmc_rts_pc_, _suffix)

#define MPMC_DECL_RTS_PC(_raw_t, _val_t, _suffix)                              \
  mpmc_rts_pc_t(_suffix) {                                                     \
    _raw_t raw;                                                                \
    struct {                                                                   \
      _val_t cnt;                                                              \
      _val_t pos;                                                              \
    } val;                                                                     \
  };                                                                           \
  static_assert(                                                               \
      (sizeof(mpmc_rts_pc_t(_suffix)) == sizeof(_raw_t)) &&                    \
          (alignof(mpmc_rts_pc_t(_suffix)) == alignof(_raw_t)),                \
      token_to_str(mpmc_rts_pc_t(_suffix)) " mismatch " token_to_str(_raw_t))

#define mpmc_operations_t(_suffix) struct token_cat(mpmc_operations_, _suffix)

#define MPMC_DECL_OPERATIONS(_val_t, _suffix)                                  \
  mpmc_operations_t(_suffix) {                                                 \
    _val_t (*available_space)(mpmc_ht_rts_t(_suffix) *,                        \
                              mpmc_rts_pc_t(_suffix), _val_t cap,              \
                              _val_t another);                                 \
    _val_t (*load_another)(mpmc_ht_rts_t(_suffix) *, void *__restrict);        \
    void (*post_update)(mpmc_ht_rts_t(_suffix) *, mpmc_rts_pc_t(_suffix));     \
    bool (*skip_head_wait)(mpmc_ht_rts_t(_suffix) *,                           \
                           mpmc_rts_pc_t(_suffix) *);                          \
    void (*yield)(mpmc_ht_rts_t(_suffix) *);                                   \
  }

#define mpmc_ht_rts_t(_suffix) struct token_cat(mpmc_ht_rts_, _suffix)

#define MPMC_DECL_HT_RTS(_val_t, _suffix)                                      \
  mpmc_ht_rts_t(_suffix) {                                                     \
    volatile mpmc_rts_pc_t(_suffix) tail;                                      \
    volatile mpmc_rts_pc_t(_suffix) head;                                      \
    const mpmc_operations_t(_suffix) * opt;                                    \
    _val_t htd_max;                                                            \
  }

#define mpmc_rts_move_ht(_suffix) token_cat(mpmc_rts_move_ht_, _suffix)

#define MPMC_DECL_FN_MOVE_HT(_suffix, _val_t)                                  \
  extern _val_t mpmc_rts_move_ht(_suffix)(                                     \
      mpmc_ht_rts_t(_suffix) *, void *__restrict, _val_t *__restrict,          \
      _val_t *__restrict, _val_t, _val_t, bool) __attribute__((__nonnull__))

#define mpmc_update_tail(_suffix) token_cat(mpmc_update_tail_, _suffix)

#define MPMC_DECL_FN_UPDATE_TAIL(_suffix)                                      \
  extern void mpmc_update_tail(_suffix)(mpmc_ht_rts_t(_suffix) *)              \
      __attribute__((__nonnull__))

#define MPMC_DECL(_raw_t, _val_t, _suffix)                                     \
  mpmc_ht_rts_t(_suffix);                                                      \
  MPMC_DECL_RTS_PC(_raw_t, _val_t, _suffix);                                   \
  MPMC_DECL_OPERATIONS(_val_t, _suffix);                                       \
  MPMC_DECL_HT_RTS(_val_t, _suffix);                                           \
  MPMC_DECL_FN_MOVE_HT(_suffix, _val_t);                                       \
  MPMC_DECL_FN_UPDATE_TAIL(_suffix)

#endif /* MPMC_DECL_H_DEFINED */
