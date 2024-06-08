#ifndef MPMC_IMPL_H_DEFINED
#define MPMC_IMPL_H_DEFINED

#include <stdint.h>
#include "mpmc.h"
#include "utils.h"

#ifndef mpmc_impl_val_t
#define mpmc_impl_val_t uint16_t
#endif

#ifndef mpmc_impl_raw_t
#define mpmc_impl_raw_t uint32_t
#endif

#ifndef mpmc_impl_suffix
#define mpmc_impl_suffix u32
#endif

#ifndef MPMC_IMPL_RTS_MAX_DIST
#define MPMC_IMPL_RTS_MAX_DIST (mpmc_impl_val_t)(128)
#endif

#ifndef mpmc_impl_rts_head_wait
#define mpmc_impl_rts_head_wait(_suffix) token_cat(mpmc_rts_head_wait_, _suffix)
#endif /* mpmc_impl_rts_head_wait */

static void mpmc_impl_rts_head_wait(mpmc_impl_suffix)(
    mpmc_ht_rts_t(mpmc_impl_suffix) * ht,
    mpmc_rts_pc_t(mpmc_impl_suffix) * pc) {
  mpmc_impl_val_t max = ht->htd_max;
  mpmc_impl_val_t tail = __atomic_load_n(&ht->tail.val.pos, __ATOMIC_RELAXED);

  while (pc->val.pos - tail > max) {
    if (ht->opt->skip_head_wait && ht->opt->skip_head_wait(ht, pc)) {
      break;
    }
    if (ht->opt->yield) {
      ht->opt->yield(ht);
    }
    pc->raw = __atomic_load_n(&ht->head.raw, __ATOMIC_ACQUIRE);
  }
}

void mpmc_update_tail(mpmc_impl_suffix)(mpmc_ht_rts_t(mpmc_impl_suffix) * ht) {
  mpmc_rts_pc_t(mpmc_impl_suffix) tmp = {0};
  mpmc_rts_pc_t(mpmc_impl_suffix) old = {0};
  mpmc_rts_pc_t(mpmc_impl_suffix) new = {0};

  old.raw = __atomic_load_n(&ht->tail.raw, __ATOMIC_ACQUIRE);

  do {
    tmp.raw = __atomic_load_n(&ht->head.raw, __ATOMIC_RELAXED);
    new.raw = old.raw;
    if (++new.val.cnt == tmp.val.cnt) {
      new.val.pos = tmp.val.pos;
    }
  } while (!__atomic_compare_exchange_n(&ht->tail.raw, &old.raw, new.raw, 0,
                                        __ATOMIC_RELEASE, __ATOMIC_ACQUIRE));
  /* here, if you need, call the pre-defined func */
  if (ht->opt->post_update) {
    ht->opt->post_update(ht, new);
  }
}

mpmc_impl_val_t mpmc_rts_move_ht(mpmc_impl_suffix)(
    mpmc_ht_rts_t(mpmc_impl_suffix) * ht, void *__restrict another,
    mpmc_impl_val_t *__restrict prev, mpmc_impl_val_t *__restrict left,
    mpmc_impl_val_t len, mpmc_impl_val_t cap, bool fixed) {
  mpmc_rts_pc_t(mpmc_impl_suffix) old = {0};
  mpmc_rts_pc_t(mpmc_impl_suffix) new = {0};
  mpmc_impl_val_t tmp_tail;
  mpmc_impl_val_t tmp_left;
  mpmc_impl_val_t n;

  old.raw = __atomic_load_n(&ht->head.raw, __ATOMIC_ACQUIRE);

  do {
    n = len;
    mpmc_impl_rts_head_wait(mpmc_impl_suffix)(ht, &old);
    tmp_tail = ht->opt->load_another(ht, another);
    tmp_left = ht->opt->available_space(ht, old, cap, tmp_tail);
    if (n > tmp_left) {
      n = fixed ? 0 : tmp_left;
    }
    if (unlikely(0 == n)) {
      break;
    }

    new.val.pos = old.val.pos + n;
    new.val.cnt = old.val.cnt + 1;
  } while (!__atomic_compare_exchange_n(&ht->head.raw, &old.raw, new.raw, 0,
                                        __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE));

  *left = tmp_left;
  *prev = old.val.pos;

  return n;
}

#undef MPMC_IMPL_H_DEFINED
#endif /* MPMC_IMPL_H_DEFINED */
