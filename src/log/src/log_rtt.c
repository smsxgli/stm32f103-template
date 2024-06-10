#include "utils.h"
#include "bitops.h"
#include "rbuf.h"
#include "mpmc.h"
#include "arch.h"
#include "log.h"
#include <stdint.h>
#include <stdbool.h>

enum RTT_CONSTANT {
  RTT_UP_BUF_MAX_NUM = 3,
  RTT_DOWN_BUF_MAX_NUM = RTT_UP_BUF_MAX_NUM,
  RTT_NAME_SIZE = 16,
  RTT_NORMAL_UP_SIZE = 1024,
  RTT_NORMAL_DOWN_SIZE = 64,
  RTT_NORMAL_CH = 0,
};

enum RTT_MODE {
  RTT_MODE_NO_BLOCK_SKIP = 0, // Skip. Do not block, output nothing. (Default)
  RTT_MODE_NO_BLOCK_TRIM = 1, // Trim: Do not block, output as much as fits.
  RTT_MODE_BLOCK_IF_FIFO_FULL =
      2, // Block: Wait until there is space in the buffer.
  RTT_MODE_DEFAULT = RTT_MODE_NO_BLOCK_SKIP,
};

enum RTT_MAGIC {
  RTT_INIT_MAGIC = 0x451d351e,
};

enum RTT_MPMC_CONSTANT {
  RTT_MPMC_RTS_MAX_DIST = 128,
};

enum RTT_LOCKLESS_TYPE {
  RTT_SPMC_CONS,
  RTT_MPSC_PROD,
};

struct rtt_buf {
  const char *name;
  char *buf;
  unsigned int buf_size;
  unsigned int in;
  unsigned int out;
  unsigned int flags;
};

struct rtt_ctrl_blk {
  char name[RTT_NAME_SIZE];
  int max_up_buf_num;
  int max_down_buf_num;
  struct rtt_buf up_bufs[RTT_UP_BUF_MAX_NUM];
  struct rtt_buf down_bufs[RTT_DOWN_BUF_MAX_NUM];
};

struct rtt_lockless_meta {
  struct mpmc_ht_rts_u32 ht;
  int lock_in;
  struct rtt_buf *buf;
  int lock_out;
};

struct rtt_mpmc_desc {
  struct mpmc_ht_rts_u32 *ht;
  unsigned int *another;
  char *buf;
  uint16_t cap;
};

static inline __attribute__((__always_inline__)) uint16_t
available_space_common(struct mpmc_ht_rts_u32 *, union mpmc_rts_pc_u32,
                       uint16_t, uint16_t);
static inline __attribute__((__always_inline__)) uint16_t
load_another_common(struct mpmc_ht_rts_u32 *, void *__restrict);
static inline __attribute__((__always_inline__)) void
post_update_prod(struct mpmc_ht_rts_u32 *, union mpmc_rts_pc_u32);
static inline __attribute__((__always_inline__)) void
post_update_cons(struct mpmc_ht_rts_u32 *, union mpmc_rts_pc_u32);
static inline __attribute__((__always_inline__)) bool
skip_head_wait_common(struct mpmc_ht_rts_u32 *, union mpmc_rts_pc_u32 *);

static struct rtt_ctrl_blk _SEGGER_RTT;
static struct rtt_lockless_meta up_buf_meta[RTT_UP_BUF_MAX_NUM];
static struct rtt_lockless_meta down_buf_meta[RTT_DOWN_BUF_MAX_NUM];
static char normal_up_buf[RTT_NORMAL_UP_SIZE];
static char normal_down_buf[RTT_NORMAL_DOWN_SIZE];

static const struct mpmc_operations_u32 RTT_PROD_OPTS = {
    .available_space = available_space_common,
    .load_another = load_another_common,
    .post_update = post_update_prod,
    .skip_head_wait = skip_head_wait_common,
};

static const struct mpmc_operations_u32 RTT_CONS_OPTS = {
    .available_space = available_space_common,
    .load_another = load_another_common,
    .post_update = post_update_cons,
    .skip_head_wait = skip_head_wait_common,
};

static inline uint16_t available_space_common(struct mpmc_ht_rts_u32 *ht,
                                              union mpmc_rts_pc_u32 pc,
                                              uint16_t cap, uint16_t out) {
  uint16_t pos = pc.val.pos & (cap - 1);
  int left = 0;

  (void)ht;
  if (out <= pos) {
    left = cap - 1 - pos + out;
  } else {
    left = out - pos - 1;
  }
  return (uint16_t)left;
}

static inline uint16_t load_another_common(struct mpmc_ht_rts_u32 *ht,
                                           void *__restrict other) {
  unsigned int *other_idx = other;
  (void)ht;
  return (uint16_t)__atomic_load_n(other_idx, __ATOMIC_RELAXED);
}

static inline void post_update_common(struct rtt_buf *buf, unsigned int *p,
                                      union mpmc_rts_pc_u32 pc) {
  unsigned int pos = pc.val.pos & (buf->buf_size - 1);
  __atomic_store_n(p, pos, __ATOMIC_RELEASE);
}

static inline void post_update_prod(struct mpmc_ht_rts_u32 *ht,
                                    union mpmc_rts_pc_u32 pc) {
  struct rtt_lockless_meta *meta =
      container_of(ht, struct rtt_lockless_meta, ht);
  struct rtt_buf *buf = meta->buf;
  post_update_common(buf, &buf->in, pc);
}

static void post_update_cons(struct mpmc_ht_rts_u32 *ht,
                             union mpmc_rts_pc_u32 pc) {
  struct rtt_lockless_meta *meta =
      container_of(ht, struct rtt_lockless_meta, ht);
  struct rtt_buf *buf = meta->buf;
  post_update_common(buf, &buf->out, pc);
}

static inline bool skip_head_wait_common(struct mpmc_ht_rts_u32 *ht,
                                         union mpmc_rts_pc_u32 *pc) {
  (void)ht;
  (void)pc;
  /* skip wait while we are in irq or exception */
  return !arch_is_in_thread();
}

static uint16_t rtt_mpsc_put(struct rtt_mpmc_desc *desc,
                             const char *__restrict src, uint16_t len,
                             bool fixed) {
  uint16_t prev, left;

  len = mpmc_rts_move_ht_u32(desc->ht, desc->another, &prev, &left, len,
                             desc->cap, fixed);
  if (len) {
    rbuf_put_u16(desc->buf, desc->cap, prev, src, len);
    mpmc_update_tail_u32(desc->ht);
  }
  (void)left;

  return len;
}

static uint16_t rtt_spmc_get(struct rtt_mpmc_desc *desc, char *__restrict dst,
                             uint16_t len, bool fixed) {
  uint16_t prev, left;

  len = mpmc_rts_move_ht_u32(desc->ht, desc->another, &prev, &left, len,
                             desc->cap, fixed);
  if (len) {
    rbuf_get_u16(desc->buf, desc->cap, prev, dst, len);
    mpmc_update_tail_u32(desc->ht);
  }
  (void)left;

  return len;
}

static void rtt_init_common(void) {
  struct rtt_ctrl_blk *pcb = &_SEGGER_RTT;
  pcb->max_up_buf_num = RTT_UP_BUF_MAX_NUM;
  pcb->max_down_buf_num = RTT_DOWN_BUF_MAX_NUM;
}

static inline __attribute__((__always_inline__)) void
rtt_init_buf_common(struct rtt_buf *r, struct rtt_lockless_meta *meta,
                    const char *name, char *buf,
                    const struct mpmc_operations_u32 *opt,
                    enum RTT_LOCKLESS_TYPE type, unsigned int buf_size) {
  int magic;

  r->name = name;
  r->buf = buf;
  r->buf_size = buf_size;
  r->flags = RTT_MODE_DEFAULT;
  magic = __atomic_exchange_n(&meta->lock_in, RTT_INIT_MAGIC, __ATOMIC_RELAXED);
  if (magic != RTT_INIT_MAGIC) {
    /* not init yet */
    r->in = 0;
    if (RTT_MPSC_PROD == type) {
      meta->ht.head.raw = 0;
      meta->ht.tail.raw = 0;
    }
  }
  magic =
      __atomic_exchange_n(&meta->lock_out, RTT_INIT_MAGIC, __ATOMIC_RELAXED);
  if (magic != RTT_INIT_MAGIC) {
    /* not init yet */
    r->out = 0;
    if (RTT_SPMC_CONS == type) {
      meta->ht.head.raw = 0;
      meta->ht.tail.raw = 0;
    }
  }
  meta->ht.opt = opt;
  meta->ht.htd_max = RTT_MPMC_RTS_MAX_DIST;
  meta->buf = r;
}

static void rtt_init_commit(void) {
  const char rtt_init_str_rev[] = "\0\0\0\0\0\0TTR REGGES";

  __atomic_thread_fence(__ATOMIC_ACQUIRE);
  struct rtt_ctrl_blk *pcb = &_SEGGER_RTT;
  for (size_t i = 0; i < sizeof(rtt_init_str_rev) - 1; i++) {
    pcb->name[i] = rtt_init_str_rev[sizeof(rtt_init_str_rev) - 2 - i];
  }
  __atomic_thread_fence(__ATOMIC_RELEASE);
}

static void rtt_do_init(void) {
  static_assert(is_power_of_two(sizeof(normal_up_buf)), "");
  static_assert(is_power_of_two(sizeof(normal_down_buf)), "");

  rtt_init_common();
  rtt_init_buf_common(&_SEGGER_RTT.up_bufs[0], &up_buf_meta[0], "Terminal",
                      normal_up_buf, &RTT_PROD_OPTS, RTT_MPSC_PROD,
                      sizeof(normal_up_buf));
  rtt_init_buf_common(&_SEGGER_RTT.down_bufs[0], &down_buf_meta[0], "Terminal",
                      normal_down_buf, &RTT_CONS_OPTS, RTT_SPMC_CONS,
                      sizeof(normal_down_buf));
  rtt_init_commit();
}

static void rtt_init(void) {
  volatile struct rtt_ctrl_blk *pcb = &_SEGGER_RTT;
  if (unlikely('S' != pcb->name[0])) {
    assert(__atomic_always_lock_free(sizeof(int), 0) &&
           __atomic_always_lock_free(sizeof(uint32_t), 0) &&
           __atomic_always_lock_free(sizeof(uint16_t), 0));
    /* not init yet */
    rtt_do_init();
  }
}

static uint16_t rtt_write(unsigned int idx, const char *src, uint16_t len) {
  struct rtt_mpmc_desc desc = {0};
  struct rtt_buf *buf = NULL;
  struct mpmc_ht_rts_u32 *ht = NULL;
  uint16_t retval = 0;

  rtt_init();

  if (idx != 0) {
    /* Now we just use channel 0 */
    return 0;
  }

  buf = &_SEGGER_RTT.up_bufs[idx];
  ht = &up_buf_meta[idx].ht;

  desc.buf = buf->buf;
  desc.cap = (uint16_t)buf->buf_size;
  desc.ht = ht;
  desc.another = &buf->out;
  /* TODO: do not hardcode 'fixed', write depend on RTT MODE */
  retval = rtt_mpsc_put(&desc, src, len, true);

  return retval;
}

static uint16_t rtt_read_block(unsigned int idx, char *dst, uint16_t len) {
  struct rtt_mpmc_desc desc = {0};
  struct rtt_buf *buf = NULL;
  struct mpmc_ht_rts_u32 *ht = NULL;
  uint16_t retval = 0;

  rtt_init();

  if (idx != 0) {
    /* Now we just use channel 0 */
    return 0;
  }

  buf = &_SEGGER_RTT.down_bufs[idx];
  ht = &down_buf_meta[idx].ht;

  desc.buf = buf->buf;
  desc.cap = (uint16_t)buf->buf_size;
  desc.ht = ht;
  desc.another = &buf->in;

  while (len) {
    uint16_t l = rtt_spmc_get(&desc, &dst[retval], len, false);
    len -= l;
    retval += l;
  }

  return retval;
}

static bool rtt_buf_empty(struct rtt_buf *buf) {
  unsigned int out = __atomic_load_n(&buf->out, __ATOMIC_RELAXED);
  unsigned int in = __atomic_load_n(&buf->in, __ATOMIC_RELAXED);

  return out == in;
}

int log_write(const char *data, size_t len) {
  return rtt_write(0, data, (uint16_t)len);
}

void log_flush(void) {
  struct rtt_buf *buf = &_SEGGER_RTT.up_bufs[0];
  struct mpmc_ht_rts_u32 *ht = &up_buf_meta[0].ht;

  rtt_init();

  while (!rtt_buf_empty(buf)) {
    if (ht->opt->skip_head_wait && !ht->opt->skip_head_wait(ht, NULL) &&
        ht->opt->yield) {
      ht->opt->yield(ht);
    }
  }
}

int log_read(char *buf, size_t len) {
  return rtt_read_block(0, buf, (uint16_t)len);
}
