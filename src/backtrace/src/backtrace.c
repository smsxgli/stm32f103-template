#include "backtrace.h"
#include "./libunwind.h"
#include "arch.h"
#include <stdint.h>
#include <string.h>

/* personality routine is not needed, strong symbols below can override weak
 * symbols from libgcc, thus reduce code size */
__attribute__((__used__)) _Unwind_Reason_Code
__aeabi_unwind_cpp_pr0(_Unwind_State state, _Unwind_Control_Block *ucbp,
                       _Unwind_Context *context) {
  (void)state;
  (void)ucbp;
  (void)context;
  return _URC_FAILURE;
}

__attribute__((__used__)) _Unwind_Reason_Code
__aeabi_unwind_cpp_pr1(_Unwind_State state, _Unwind_Control_Block *ucbp,
                       _Unwind_Context *context) {
  (void)state;
  (void)ucbp;
  (void)context;
  return _URC_FAILURE;
}

__attribute__((__used__)) _Unwind_Reason_Code
__aeabi_unwind_cpp_pr2(_Unwind_State state, _Unwind_Control_Block *ucbp,
                       _Unwind_Context *context) {
  (void)state;
  (void)ucbp;
  (void)context;
  return _URC_FAILURE;
}

struct unwind_index {
  uint32_t addr_offset;
  uint32_t insn;
};

struct backtrace_context {
  uint32_t vrs[16];
  const uint32_t *current;
  int byte;
  unsigned int remaining;
};

struct unwind_insn_entry {
  bool (*fit)(const struct backtrace_context *, uint8_t);
  void (*decoder)(struct backtrace_context *, uint8_t);
};

enum unwind_index_value {
  EXIDX_CANTUNWIND = 0x1,
};

enum vrs_regs { VRS_FP = 7, VRS_SP = 13, VRS_LR = 14, VRS_PC = 15 };

enum unwind_decoder_result {
  UNWIND_REFUSE = 0,
  UNWIND_FINISHED = 1,
  UNWIND_UNKNOWN_INSN = -1,
};

static inline const void *prel31_to_addr(const uint32_t *)
    __attribute__((__always_inline__));
static const struct unwind_index *
search_unwind_index(const struct unwind_index *start,
                    const struct unwind_index *end, const void *ip);
static const char *get_func_name(const void *addr);
static int context_init(struct backtrace_context *, const uint32_t *,
                        const struct backtrace_frame *);
static int unwind_get_next_byte(struct backtrace_context *);
static int unwind_execute_instruction(struct backtrace_context *);
static int unwind_stack_frame(struct backtrace_frame *,
                              const struct unwind_index *);

extern const struct unwind_index __exidx_start[];
extern const struct unwind_index __exidx_end[];

static inline const void *prel31_to_addr(const uint32_t *prel31) {
  int32_t offset;
  /* sign-extend to 32 bits */
  /* offset = (((int32_t)(*prel31) << 1) >> 1); */
  /* ubsan will caught overflow for signed int, so we keep such sign bit
   * manually */
  /*
   * convert unsigned int to signed is implementation-defined, gcc's document
   * says: 'For conversion to a type of width N, the value is reduced modulo 2^N
   * to be within range of the type; no signal is raised. '
   * https://gcc.gnu.org/onlinedocs/gcc/Integers-implementation.html
   */
  offset = (int32_t)(*prel31 << 1);
  /* gcc also promise shifting right for signed int will perform 'arithmetic
   * shift', which keeps the number’s sign unchanged by duplicating the sign
   * bit. */
  offset >>= 1;
  return (const char *)prel31 + offset;
}

/* binary search to find right func index address */
static const struct unwind_index *
search_unwind_index(const struct unwind_index *start,
                    const struct unwind_index *end, const void *ip) {
  const struct unwind_index *middle;
  uint32_t addr_prel31;

  addr_prel31 = ((unsigned long)ip - (unsigned long)start) & 0x7fffffff;

  while (start < end - 1) {
    middle = start + ((end - start) / 2);
    if (addr_prel31 - ((unsigned long)middle - (unsigned long)start) <
        middle->addr_offset) {
      end = middle;
    } else {
      addr_prel31 -= ((unsigned long)middle - (unsigned long)start);
      start = middle;
    }
  }

  if (start->addr_offset <= addr_prel31) {
    return start;
  } else {
    return NULL;
  }
}

/* https://gcc.gnu.org/onlinedocs/gcc-14.1.0/gcc/ARM-Options.html */
static const char *get_func_name(const void *addr) {
  uint32_t flag_word = *((const uint32_t *)((const char *)addr - 4));
  if (0xff000000 == (flag_word & 0xff000000)) {
    return (const char *)addr - 4 - (flag_word & 0x00ffffff);
  }
  return "<unknown>";
}

static int context_init(struct backtrace_context *ctx, const uint32_t *insn,
                        const struct backtrace_frame *frame) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->current = insn;

  /* ehabi32.pdf, 7.3, 10.2 */
  if (0x80000000 == (*insn & 0xff000000)) {
    /* short format, Su16 */
    ctx->remaining = 3;
    ctx->byte = 2;
  } else if (0x81000000 == (*insn & 0xff000000) ||
             0x82000000 == (*insn & 0xff000000)) {
    /* Lu16 or Lu32 */
    ctx->remaining = 2 + ((*insn & 0xff0000) >> 14);
    ctx->byte = 1;
  } else {
    return -1;
  }

  ctx->vrs[VRS_FP] = frame->fp;
  ctx->vrs[VRS_SP] = frame->sp;
  ctx->vrs[VRS_LR] = frame->lr;
  ctx->vrs[VRS_PC] = 0;

  return 0;
}

static int unwind_get_next_byte(struct backtrace_context *ctx) {
  int insn;

  /* no more instructions */
  if (!ctx->remaining) {
    return -1;
  }
  /* extract the next instruction */
  insn = (int)(((*ctx->current) >> (ctx->byte << 3)) & 0xff);
  /* move to next byte */
  ctx->byte--;
  if (ctx->byte < 0) {
    /* to next world */
    ctx->current++;
    ctx->byte = 3;
  }
  ctx->remaining--;

  return insn;
}

static int unwind_execute_instruction(struct backtrace_context *ctx) {
  int insn;
  uint32_t mask;
  int reg;
  uint32_t *vsp;

  while (-1 != (insn = unwind_get_next_byte(ctx))) {
    if (0x00 == (insn & 0xc0)) {
      /* vsp = vsp + (xxxxxx << 2) + 4 */
      ctx->vrs[VRS_SP] += (((uint32_t)insn & 0x3f) << 2) + 4;
    } else if (0x40 == (insn & 0xc0)) {
      /* vsp = vsp - (xxxxxx << 2) - 4 */
      ctx->vrs[VRS_SP] -= (((uint32_t)insn & 0x3f) << 2) + 4;
    } else if (0x80 == (insn & 0xf0)) {
      /* pop under mask {r15-r12},{r11-r4} or refuse to unwind */
      insn = insn << 8 | unwind_get_next_byte(ctx);
      /* Check for refuse to unwind */
      if (0x8000 == insn) {
        return 0;
      }
      /* Pop registers using mask */
      memcpy(&vsp, &ctx->vrs[VRS_SP], sizeof(vsp));
      mask = insn & 0xfff;
      /* Loop through the mask */
      reg = 4;
      while (mask) {
        if (mask & 0x01) {
          ctx->vrs[reg] = *vsp++;
        }
        mask >>= 1;
        reg++;
      }
      /* update sp if sp(r13) not in the mask */
      if (!(mask & 1 << (13 - 4))) {
        ctx->vrs[13] = (uint32_t)vsp;
      }
    } else if (0x90 == (insn & 0xf0) && insn != 0x9d && insn != 0x9f) {
      /* vsp = r[nnnn] */
      ctx->vrs[VRS_SP] = ctx->vrs[insn & 0x0f];
    } else if (0xa0 == (insn & 0xf0)) {
      /* pop r4-r[4+nnn] or pop r4-r[4+nnn], r14*/
      memcpy(&vsp, &ctx->vrs[VRS_SP], sizeof(vsp));

      for (reg = 4; reg <= (insn & 0x07) + 4; reg++) {
        ctx->vrs[reg] = *vsp++;
      }
      if (insn & 0x08) {
        ctx->vrs[14] = *vsp++;
      }
      ctx->vrs[VRS_SP] = (uint32_t)vsp;
    } else if (0xb0 == insn) {
      /* finished */
      if (!ctx->vrs[VRS_PC]) {
        ctx->vrs[VRS_PC] = ctx->vrs[VRS_LR];
      }
      /* All done unwinding */
      return 0;
    } else if (0xb1 == insn) {
      /* pop register under mask {r3,r2,r1,r0} */
      memcpy(&vsp, &ctx->vrs[VRS_SP], sizeof(vsp));
      mask = (uint32_t)unwind_get_next_byte(ctx);
      reg = 0;
      while (mask) {
        if (mask & 0x01) {
          ctx->vrs[reg] = *vsp++;
        }
        mask >>= 1;
        reg++;
      }
      ctx->vrs[VRS_SP] = (uint32_t)vsp;
    } else if (0xb2 == insn) {
      /* vps = vsp + 0x204 + (uleb128 << 2) */
      ctx->vrs[VRS_SP] += 0x204 + (uint32_t)(unwind_get_next_byte(ctx) << 2);
    } else if (0xb3 == insn || 0xc8 == insn || 0xc9 == insn) {
      /* pop VFP double-precision registers */
      memcpy(&vsp, &ctx->vrs[VRS_SP], sizeof(vsp));
      /* D[ssss]-D[ssss+cccc] or D[16+sssss]-D[16+ssss+cccc] as pushed by VPUSH
       * or FSTMFDX */
      vsp += 2 * ((unwind_get_next_byte(ctx) & 0x0f) + 1);
      if (0xb3 == insn) {
        /* as pushed by FSTMFDX */
        vsp++;
      }
      ctx->vrs[VRS_SP] = (uint32_t)vsp;
    } else if (0xb8 == (insn & 0xf8) || 0xd0 == (insn & 0xf8)) {
      /* pop VFP double-precision registers */
      memcpy(&vsp, &ctx->vrs[VRS_SP], sizeof(vsp));
      /* D[8]-D[8+nnn] as pushed by VPUSH or FSTMFDX */
      vsp += 2 * ((insn & 0x07) + 1);
      if (0xb8 == (insn & 0xf8)) {
        /* as pushed by FSTMFDX */
        vsp++;
      }
      ctx->vrs[VRS_SP] = (uint32_t)vsp;
    } else {
      return -1;
    }
  }

  return insn != -1;
}

static int unwind_stack_frame(struct backtrace_frame *frame,
                              const struct unwind_index *idx) {
  struct backtrace_context ctx;
  const struct unwind_index *index;
  const uint32_t *insn;
  int result;

  /* use idx if valid */
  if (idx) {
    index = idx;
  } else {
    const void *pc;
    memcpy(&pc, &frame->pc, sizeof(pc));
    index = search_unwind_index(__exidx_start, __exidx_end, pc);
    if (!index) {
      return -1;
    }
  }
  /* Make sure we can unwind this frame */
  if (0x1 == index->insn) {
    return 0;
  }
  /* Get the pointer to the first unwind instruction */
  if (index->insn & 0x80000000) {
    insn = &index->insn;
  } else {
    insn = prel31_to_addr(&index->insn);
  }

  if (context_init(&ctx, insn, frame) < 0) {
    return -1;
  }
  /* Execute the unwind instructions */
  do {
    result = unwind_execute_instruction(&ctx);
  } while (result > 0);
  if (-1 == result) {
    return -1;
  }
  /* Set the virtual pc to the virtual lr if this is the first unwind */
  if (!ctx.vrs[VRS_PC]) {
    ctx.vrs[VRS_PC] = ctx.vrs[VRS_LR];
  }
  /* Check for exception return */
  if (0xffffff00 == (ctx.vrs[VRS_PC] & 0xffffff00)) {
    /* Need manually exception return */
    uint32_t *stack;
    /* The unwind code is in Handler mode */
    /* From the cortex-m4 device generic user guide */
    /* If last 4 bit is 1 or 9, we shall get state from MSP */
    if (0x1 == (0xf & ctx.vrs[VRS_PC]) || 0x9 == (0xf & ctx.vrs[VRS_PC])) {
      memcpy(&stack, &ctx.vrs[VRS_SP], sizeof(stack));
    } else if (0xd == (0xf & ctx.vrs[VRS_PC])) {
      /* Get state from PSP */
      stack = arch_get_psp();
    } else {
      /* Invalid EXC_RETURN */
      return -1;
    }
    /* ARM ABI request all stack must be 8-bytes aligned */
    if (0xf0 == (0xf0 & ctx.vrs[VRS_PC])) {
      /* non-floating-point */
      ctx.vrs[VRS_SP] = (uint32_t)(stack + 8);
    } else if (0xe0 == (0xf0 & ctx.vrs[VRS_PC])) {
      /* floating-point */
      ctx.vrs[VRS_SP] = (uint32_t)(stack + 26);
    }
    ctx.vrs[VRS_LR] = *(stack + 5);
    ctx.vrs[VRS_PC] = *(stack + 6);
  }
  if (frame->pc == ctx.vrs[VRS_PC]) {
    /* We are done if current frame pc is equal to the virtual pc, prevent
     * infinite loop */
    return 0;
  }
  /* update stack frame */
  frame->fp = ctx.vrs[VRS_FP];
  frame->sp = ctx.vrs[VRS_SP];
  frame->lr = ctx.vrs[VRS_LR];
  frame->pc = ctx.vrs[VRS_PC];

  return 1;
}

int backtrace_unwind_stack(struct backtrace_frame *frame,
                           int (*cb)(const struct backtrace *)) {
  struct backtrace bt = {0};
  int unwind_result = 0;
  const struct unwind_index *idx;

  do {
    const void *ip;

    if (!frame->pc) {
      /* Reached __exidx_end. */
      bt.func_name = "<reached end of unwind table>";
      bt.func = NULL;
      bt.addr = NULL;
      cb(&bt);
      break;
    }
    if (0x01 == frame->pc) {
      bt.func_name = "<reached .cantunwind>";
      bt.func = NULL;
      bt.addr = NULL;
      cb(&bt);
      break;
    }

    /* Clear last bit (Thumb indicator) */
    frame->pc &= 0xfffffffe;
    memcpy(&ip, &frame->pc, sizeof(ip));
    idx = search_unwind_index(__exidx_start, __exidx_end, ip);
    bt.addr = ip;
    bt.func = prel31_to_addr(&idx->addr_offset);
    bt.func_name = get_func_name(bt.func);
    if (cb(&bt) < 0) {
      break;
    }
  } while (1 == (unwind_result = unwind_stack_frame(frame, idx)));

  return unwind_result < 0 ? -1 : 0;
}
