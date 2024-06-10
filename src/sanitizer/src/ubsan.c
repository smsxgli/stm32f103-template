/*	$NetBSD: ubsan.c,v 1.12 2023/12/07 07:10:44 andvar Exp $	*/

/*-
 * Copyright (c) 2018 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "./ubsan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>

#define UBSAN_REPORTED (1u << 31)
#define UBSAN_SIGNED 1u
#define UBSAN_NUMBER_MAX 64
#define UBSAN_PATH_MAX 128 /* hope file path not too long */
#define UBSAN_LOCATION_MAX (UBSAN_PATH_MAX + 32 /* ':LINE:COLUMN' */)

static void report(bool fatal, const char *fmt, ...)
    __attribute__((__format__(printf, 2, 3)));
static bool is_reported(struct source_location *);
static void handle_overflow(bool fatal, struct overflow_data *data, void *lhs,
                            void *rhs, const char *opt);
static void handle_negate_overflow(bool fatal, struct overflow_data *data,
                                   void *old_data);
static void handle_alignment_assumption(bool fatal,
                                        struct alignment_assumption_data *data,
                                        unsigned long ptr, unsigned long align,
                                        unsigned long offset);
static void handle_type_mismatch(bool fatal, struct source_location *loc,
                                 struct type_descriptor *type,
                                 unsigned long align, uint8_t chk_kind,
                                 void *ptr);
static void handle_out_of_bounds(bool fatal, struct out_of_bounds_data *data,
                                 void *index);
static void handle_shift_out_of_bounds(bool fatal,
                                       struct shift_out_of_bounds_data *data,
                                       void *lhs, void *rhs);
static void handle_builtin_unreachable(bool fatal,
                                       struct unreachable_data *data);
static void handle_load_invalid_value(bool fatal,
                                      struct invalid_value_data *data,
                                      void *val);
static void handle_pointer_overflow(bool fatal,
                                    struct pointer_overflow_data *data,
                                    void *base, void *result);
static void handle_nonnull_arg(bool fatal, struct nonnull_arg_data *data);
static void handle_float_cast_overflow(bool fatal,
                                       struct float_cast_overflow_data *data,
                                       void *from);
static void handle_function_type_mismatch(
    bool fatal, struct function_type_mismatch_data *data, void *func);
static void handle_implicit_conversion(bool fatal,
                                       struct implicit_conversion_data *data,
                                       void *from, void *to);
static void handle_invalid_builtin(bool fatal,
                                   struct invalid_builtin_data *data);
static void handle_missing_return(bool fatal, struct unreachable_data *data);
static void handle_nonnull_return(bool fatal, struct nonnull_return_data *data,
                                  struct source_location *loc);
static void handle_vla_bound_not_positive(
    bool fatal, struct vla_bound_not_positive_data *data, void *bound);
static void deserialize_location(char *buf, size_t len,
                                 struct source_location *location);
static void deserialize_number(char *location, char *buf, size_t len,
                               struct type_descriptor *type, void *num);
static void deserialize_number_signed(char *buf, size_t len,
                                      struct type_descriptor *type, s_max num);
static void deserialize_number_unsigned(char *buf, size_t len,
                                        struct type_descriptor *type,
                                        u_max num);
#ifdef __SIZEOF_INT128__
static void deserialize_number_uint128(char *buf, size_t len,
                                       struct type_descriptor *type,
                                       __uint128_t num);
#endif
static void deserialize_number_float(char *location, char *buf, size_t len,
                                     struct type_descriptor *type, void *num);
static void deserialize_float_over_pointer(char *buf, size_t len,
                                           struct type_descriptor *type,
                                           void *num);
static void deserialize_float_inlined(char *buf, size_t len,
                                      struct type_descriptor *type, void *num);
static u_max parse_num_umax(char *location, struct type_descriptor *type,
                            void *num);
static s_max parse_num_smax(char *location, struct type_descriptor *type,
                            void *num);
static size_t deserialize_type_width(struct type_descriptor *type);
static const char *deserialize_implicit_conversion_check_kind(uint8_t kind);
static const char *deserialize_type_check_kind(uint8_t chk_kind);
static const char *deserialize_builtin_check_kind(uint8_t chk_kind);
static bool is_negative_number(char *location, struct type_descriptor *type,
                               void *num);
static bool is_shift_exponent_too_large(char *location,
                                        struct type_descriptor *type, void *num,
                                        size_t width);

void __ubsan_handle_add_overflow(void *data, void *lhs, void *rhs) {
  assert(data);
  handle_overflow(false, data, lhs, rhs, "+");
}
void __ubsan_handle_add_overflow_abort(void *data, void *lhs, void *rhs) {
  assert(data);
  handle_overflow(true, data, lhs, rhs, "+");
  __builtin_unreachable();
}

void __ubsan_handle_sub_overflow(void *data, void *lhs, void *rhs) {
  assert(data);
  handle_overflow(false, data, lhs, rhs, "-");
}
void __ubsan_handle_sub_overflow_abort(void *data, void *lhs, void *rhs) {
  assert(data);
  handle_overflow(true, data, lhs, rhs, "-");
  __builtin_unreachable();
}

void __ubsan_handle_mul_overflow(void *data, void *lhs, void *rhs) {
  assert(data);
  handle_overflow(false, data, lhs, rhs, "*");
}
void __ubsan_handle_mul_overflow_abort(void *data, void *lhs, void *rhs) {
  assert(data);
  handle_overflow(true, data, lhs, rhs, "*");
  __builtin_unreachable();
}

void __ubsan_handle_divrem_overflow(void *data, void *lhs, void *rhs) {
  assert(data);
  handle_overflow(false, data, lhs, rhs, "divrem");
}
void __ubsan_handle_divrem_overflow_abort(void *data, void *lhs, void *rhs) {
  assert(data);
  handle_overflow(true, data, lhs, rhs, "divrem");
  __builtin_unreachable();
}

void __ubsan_handle_negate_overflow(void *data, void *old_val) {
  assert(data);
  handle_negate_overflow(false, data, old_val);
}
void __ubsan_handle_negate_overflow_abort(void *data, void *old_val) {
  assert(data);
  handle_negate_overflow(true, data, old_val);
  __builtin_unreachable();
}

void __ubsan_handle_alignment_assumption(void *data, unsigned long ptr,
                                         unsigned long align,
                                         unsigned long offset) {
  assert(data);
  handle_alignment_assumption(false, data, ptr, align, offset);
}
void __ubsan_handle_alignment_assumption_abort(void *data, unsigned long ptr,
                                               unsigned long align,
                                               unsigned long offset) {
  assert(data);
  handle_alignment_assumption(true, data, ptr, align, offset);
  __builtin_unreachable();
}

void __ubsan_handle_type_mismatch(struct type_mismatch_data *data, void *ptr) {
  assert(data);
  handle_type_mismatch(false, &data->location, data->type, data->alignment,
                       data->type_check_kind, ptr);
}
void __ubsan_handle_type_mismatch_abort(struct type_mismatch_data *data,
                                        void *ptr) {
  assert(data);
  handle_type_mismatch(true, &data->location, data->type, data->alignment,
                       data->type_check_kind, ptr);
  __builtin_unreachable();
}

void __ubsan_handle_type_mismatch_v1(void *data, void *ptr) {
  struct type_mismatch_data_v1 *rdata = data;
  assert(data);
  handle_type_mismatch(false, &rdata->location, rdata->type,
                       1UL << rdata->log_alignment, rdata->type_check_kind,
                       ptr);
}
void __ubsan_handle_type_mismatch_v1_abort(void *data, void *ptr) {
  struct type_mismatch_data_v1 *rdata = data;
  assert(data);
  handle_type_mismatch(true, &rdata->location, rdata->type,
                       1UL << rdata->log_alignment, rdata->type_check_kind,
                       ptr);
  __builtin_unreachable();
}

void __ubsan_handle_out_of_bounds(void *data, void *index) {
  assert(data);
  handle_out_of_bounds(false, data, index);
}
void __ubsan_handle_out_of_bounds_abort(void *data, void *index) {
  assert(data);
  handle_out_of_bounds(true, data, index);
  __builtin_unreachable();
}

void __ubsan_handle_shift_out_of_bounds(void *data, void *lhs, void *rhs) {
  assert(data);
  handle_shift_out_of_bounds(false, data, lhs, rhs);
}
void __ubsan_handle_shift_out_of_bounds_abort(void *data, void *lhs,
                                              void *rhs) {
  assert(data);
  handle_shift_out_of_bounds(true, data, lhs, rhs);
  __builtin_unreachable();
}

void __ubsan_handle_builtin_unreachable(void *data) {
  assert(data);
  handle_builtin_unreachable(true, data);
  __builtin_unreachable();
}

void __ubsan_handle_load_invalid_value(void *data, void *val) {
  assert(data);
  handle_load_invalid_value(false, data, val);
}
void __ubsan_handle_load_invalid_value_abort(void *data, void *val) {
  assert(data);
  handle_load_invalid_value(true, data, val);
  __builtin_unreachable();
}

void __ubsan_handle_pointer_overflow(void *data, void *base, void *result) {
  assert(data);
  handle_pointer_overflow(false, data, base, result);
}
void __ubsan_handle_pointer_overflow_abort(void *data, void *base,
                                           void *result) {
  assert(data);
  handle_pointer_overflow(true, data, base, result);
  __builtin_unreachable();
}

void __ubsan_handle_nonnull_arg(void *data) {
  assert(data);
  handle_nonnull_arg(false, data);
}
void __ubsan_handle_nonnull_arg_abort(void *data) {
  assert(data);
  handle_nonnull_arg(true, data);
  __builtin_unreachable();
}

void __ubsan_handle_nullability_arg(void *data) {
  assert(data);
  handle_nonnull_arg(false, data);
}
void __ubsan_handle_nullability_arg_abort(void *data) {
  assert(data);
  handle_nonnull_arg(true, data);
  __builtin_unreachable();
}

void __ubsan_handle_float_cast_overflow(void *data, void *from) {
  assert(data);
  handle_float_cast_overflow(false, data, from);
}
void __ubsan_handle_float_cast_overflow_abort(void *data, void *from) {
  assert(data);
  handle_float_cast_overflow(true, data, from);
  __builtin_unreachable();
}

void __ubsan_handle_function_type_mismatch(void *data, void *func) {
  assert(data);
  handle_function_type_mismatch(false, data, func);
}
void __ubsan_handle_function_type_mismatch_abort(void *data, void *func) {
  assert(data);
  handle_function_type_mismatch(true, data, func);
  __builtin_unreachable();
}

void __ubsan_handle_implicit_conversion(void *data, void *from, void *to) {
  assert(data);
  handle_implicit_conversion(false, data, from, to);
}
void __ubsan_handle_implicit_conversion_abort(void *data, void *from,
                                              void *to) {
  assert(data);
  handle_implicit_conversion(true, data, from, to);
  __builtin_unreachable();
}

void __ubsan_handle_invalid_builtin(void *data) {
  assert(data);
  handle_invalid_builtin(true, data);
  __builtin_unreachable();
}
void __ubsan_handle_invalid_builtin_abort(void *data) {
  assert(data);
  handle_invalid_builtin(true, data);
  __builtin_unreachable();
}

void __ubsan_handle_missing_return(void *data) {
  assert(data);
  handle_missing_return(true, data);
  __builtin_unreachable();
}

void __ubsan_handle_nullability_return_v1(void *data,
                                          struct source_location *loc) {
  assert(data && loc);
  handle_nonnull_return(false, data, loc);
}
void __ubsan_handle_nullability_return_v1_abort(void *data,
                                                struct source_location *loc) {
  assert(data && loc);
  handle_nonnull_return(true, data, loc);
  __builtin_unreachable();
}

void __ubsan_handle_vla_bound_not_positive(void *data, void *bound) {
  assert(data);
  handle_vla_bound_not_positive(false, data, bound);
}
void __ubsan_handle_vla_bound_not_positive_abort(void *data, void *bound) {
  assert(data);
  handle_vla_bound_not_positive(true, data, bound);
  __builtin_unreachable();
}

static bool is_reported(struct source_location *location) {
  uint32_t old;
  uint32_t new;
  uint32_t *line;

  line = &location->line;
  old = __atomic_load_n(line, __ATOMIC_ACQUIRE);
  do {
    new = old | UBSAN_REPORTED;
  } while (!__atomic_compare_exchange_n(line, &old, new, 0, __ATOMIC_RELEASE,
                                        __ATOMIC_ACQUIRE));
  return (old & UBSAN_REPORTED);
}

static void deserialize_location(char *buf, size_t len,
                                 struct source_location *location) {
  (void)snprintf(buf, len, "%s:%" PRIu32 ":%" PRIu32, location->file_name,
                 location->line & (uint32_t)~UBSAN_REPORTED, location->column);
}

static void deserialize_number(char *location, char *buf, size_t len,
                               struct type_descriptor *type, void *num) {
  switch (type->type_kind) {
  case type_kind_int:
    if (type->type_info & UBSAN_SIGNED) {
      s_max num_smax = parse_num_smax(location, type, num);
      deserialize_number_signed(buf, len, type, num_smax);
    } else {
      u_max num_umax = parse_num_umax(location, type, num);
      deserialize_number_unsigned(buf, len, type, num_umax);
    }
    break;
  case type_kind_float:
    deserialize_number_float(location, buf, len, type, num);
    break;
  case type_unknown:
    report(true, "ubsan: Unknown type in %s\n", location);
    __builtin_unreachable();
  default:
    abort();
  }
}

static void deserialize_number_signed(char *buf, size_t len,
                                      struct type_descriptor *type, s_max num) {
  size_t width = deserialize_type_width(type);
  switch (width) {
#ifdef __SIZEOF_INT128__
  case 128:
    deserialize_number_uint128(buf, len, type, (__uint128_t)num);
    break;
#endif
  case 64:
    /* FIXME: newlib seem not support %lld */
    (void)snprintf(buf, len, "%" PRId64, (int64_t)num);
    break;
  case 32:
    /* FALLTHROUGH */
  case 16:
    /* FALLTHROUGH */
  case 8:
    (void)snprintf(buf, len, "%" PRId32, (int32_t)((int64_t)num));
    break;
  default:
    /* invalid code path */
    abort();
  }
}

static void deserialize_number_unsigned(char *buf, size_t len,
                                        struct type_descriptor *type,
                                        u_max num) {
  size_t width = deserialize_type_width(type);
  switch (width) {
#ifdef __SIZEOF_INT128__
  case 128:
    deserialize_number_uint128(buf, len, type, (__uint128_t)num);
    break;
#endif
  case 64:
    /* FIXME: newlib seem not support %lld */
    (void)snprintf(buf, len, "%" PRIu64, (uint64_t)num);
    break;
  case 32:
    /* FALLTHROUGH */
  case 16:
    /* FALLTHROUGH */
  case 8:
    (void)snprintf(buf, len, "%" PRIu32, (uint32_t)((uint64_t)num));
    break;
  default:
    /* invalid code path */
    abort();
  }
}

static void deserialize_number_float(char *location, char *buf, size_t len,
                                     struct type_descriptor *type, void *num) {
  size_t width = deserialize_type_width(type);
  switch (width) {
#ifdef __HAVE_LONG_DOUBLE
  case 128:
    /* FALLTHROUGH */
  case 96:
    /* FALLTHROUGH */
  case 80:
    deserialize_float_over_pointer(buf, len, type, num);
    break;
#endif
  case 64:
    if (sizeof(num) * CHAR_BIT < 64) {
      deserialize_float_over_pointer(buf, len, type, num);
      break;
    }
    /* FALLTHROUGH */
  case 32:
    /* FALLTHROUGH */
  case 16:
    deserialize_float_inlined(buf, len, type, num);
    break;
  default:
    /* invalid code path */
    report(true, "ubsan: Unexpected %zu-bit type in %s\n", width, location);
  }
}

static void deserialize_float_over_pointer(char *buf, size_t len,
                                           struct type_descriptor *type,
                                           void *num) {
  size_t width;
  double d;
#ifdef __HAVE_LONG_DOUBLE
  long double ld;
#endif
  assert(sizeof(unsigned long) * CHAR_BIT < 64 ||
         deserialize_type_width(type) >= 64);
  static_assert(sizeof(d) == sizeof(uint64_t), "");
#ifdef __HAVE_LONG_DOUBLE
  static_assert(sizeof(ld) > sizeof(uint64_t), "");
#endif
  width = deserialize_type_width(type);
  switch (width) {
#ifdef __HAVE_LONG_DOUBLE
  case 128:
    /* FALLTHROUGH */
  case 96:
    /* FALLTHROUGH */
  case 80:
    memcpy(&ld, num, sizeof(long double));
    (void)snprintf(buf, len, "%Lg", ld);
    break;
#endif
  case 64:
    memcpy(&d, num, sizeof(double));
    (void)snprintf(buf, len, "%g", d);
    break;
  default:
    __builtin_unreachable();
  }
}

static void deserialize_float_inlined(char *buf, size_t len,
                                      struct type_descriptor *type, void *num) {
  float f;
  double d;
  uint32_t u32;
  size_t width;

  static_assert(sizeof(f) == sizeof(uint32_t) && sizeof(d) == sizeof(uint64_t),
                "");
  width = deserialize_type_width(type);
  switch (width) {
  case 64:
    memcpy(&d, &num, sizeof(num));
    (void)snprintf(buf, len, "%g", d);
    break;
  case 32:
    /*
     * On supported platforms sizeof(float)==sizeof(uint32_t)
     * unsigned long is either 32 or 64-bit, cast it to 32-bit
     * value in order to call memcpy(3) in an Endian-aware way.
     */
    u32 = (uint32_t)num;
    memcpy(&f, &u32, sizeof(float));
    (void)snprintf(buf, len, "%g", (double)f);
    break;
  case 16:
    (void)snprintf(buf, len, "Undecoded-16-bit-Floating-Type (%#04" PRIx16 ")",
                   (uint16_t)((unsigned long)num));
    break;
  }
}

static size_t deserialize_type_width(struct type_descriptor *type) {
  size_t width;

  switch (type->type_kind) {
  case type_kind_int:
    width = (size_t)1 << (type->type_info >> 1);
    break;
  case type_kind_float:
    width = type->type_info;
    break;
  default:
    report(true, "ubsan: Unknown variable type %#04" PRIx16 "\n",
           type->type_kind);
    __builtin_unreachable();
  }
  assert(width > 0);

  return width;
}

static const char *deserialize_implicit_conversion_check_kind(uint8_t kind) {
  static const char *CONVERSION_CHECK_KIND[] = {
      "integer truncation", /* Not used since 2018 October 11th */
      "unsigned integer truncation",
      "signed integer truncation",
      "integer sign change",
      "signed integer trunctation or sign change",
  };
  assert(kind <
         sizeof(CONVERSION_CHECK_KIND) / sizeof(CONVERSION_CHECK_KIND[0]));
  return CONVERSION_CHECK_KIND[kind];
}

static u_max parse_num_umax(char *location, struct type_descriptor *type,
                            void *num) {
  size_t width;
  u_max num_umax = 0;

  width = deserialize_type_width(type);
  switch (width) {
  case 128:
#ifdef __SIZEOF_INT128__
    memcpy(&num_umax, num, sizeof(u_max));
    break;
#else
    report(true, "ubsan: Unexpected 128-bit type in %s\n", location);
    __builtin_unreachable();
#endif
  case 64:
    if (sizeof(num) * CHAR_BIT < 64) {
      num_umax = *(uint64_t *)num;
      break;
    }
  /* FALLTHROUGH */
  case 32:
  /* FALLTHROUGH */
  case 16:
  /* FALLTHROUGH */
  case 8: {
    num_umax = (unsigned long)num;
    break;
  }
  default:
    report(true, "ubsan: Unexpected %zu-bit type in %s\n", width, location);
    __builtin_unreachable();
  }
  return num_umax;
}

static s_max parse_num_smax(char *location, struct type_descriptor *type,
                            void *num) {
  size_t width;
  s_max num_smax;

  width = deserialize_type_width(type);
  switch (width) {
  case 128:
#ifdef __SIZEOF_INT128__
    memcpy(&num_smax, num, sizeof(s_max));
    break;
#else
    report(true, "ubsan: Unexpected 128-bit type in %s\n", location);
    __builtin_unreachable();
#endif
  case 64:
    if (sizeof(num) * CHAR_BIT < 64) {
      num_smax = *(int64_t *)num;
    } else {
      num_smax = (int64_t)((uint64_t)((unsigned long)num));
    }
    break;
  case 32:
    num_smax = (int32_t)((uint32_t)((unsigned long)num));
    break;
  case 16:
    num_smax = (int16_t)((uint16_t)((unsigned long)num));
    break;
  case 8:
    num_smax = (int8_t)((uint8_t)((unsigned long)num));
    break;
  default:
    report(true, "ubsan: Unexpected %zu-bit type in %s\n", width, location);
    __builtin_unreachable();
  }
  return num_smax;
}

static const char *deserialize_type_check_kind(uint8_t chk_kind) {
  static const char *TYPE_CHECK_KIND[] = {"load of",
                                          "store to",
                                          "reference binding to",
                                          "member access within",
                                          "member call on",
                                          "constructor call on",
                                          "downcast of",
                                          "downcast of",
                                          "upcast of",
                                          "cast to virtual base of",
                                          "_Nonnull binding to",
                                          "dynamic operation on"};
  assert(chk_kind < sizeof(TYPE_CHECK_KIND) / sizeof(TYPE_CHECK_KIND[0]));

  return TYPE_CHECK_KIND[chk_kind];
}

static const char *deserialize_builtin_check_kind(uint8_t chk_kind) {
  static const char *BUILTIN_CHECK_KIND[] = {"ctz()", "clz()"};
  assert(chk_kind < sizeof(BUILTIN_CHECK_KIND) / sizeof(BUILTIN_CHECK_KIND[0]));

  return BUILTIN_CHECK_KIND[chk_kind];
}

static bool is_negative_number(char *location, struct type_descriptor *type,
                               void *num) {
  assert(type_kind_int == type->type_kind);

  if (!(type->type_info & UBSAN_SIGNED)) {
    return false;
  }
  return parse_num_smax(location, type, num) < 0;
}

static bool is_shift_exponent_too_large(char *location,
                                        struct type_descriptor *type, void *num,
                                        size_t width) {
  assert(type_kind_int == type->type_kind);

  return parse_num_umax(location, type, num) >= width;
}

static void handle_overflow(bool fatal, struct overflow_data *data, void *lhs,
                            void *rhs, const char *opt) {
  char location_buf[UBSAN_LOCATION_MAX];
  char lhs_buf[UBSAN_NUMBER_MAX];
  char rhs_buf[UBSAN_NUMBER_MAX];

  if (is_reported(&data->location)) {
    return;
  }
  deserialize_location(location_buf, sizeof(location_buf), &data->location);
  deserialize_number(location_buf, lhs_buf, sizeof(lhs_buf), data->type, lhs);
  deserialize_number(location_buf, rhs_buf, sizeof(rhs_buf), data->type, rhs);

  report(fatal,
         "ubsan: Undefined behavior in %s, %s integer overflow: %s %s %s "
         "cannot be represented in type %s\n",
         location_buf,
         data->type->type_info & UBSAN_SIGNED ? "signed" : "unsigned", lhs_buf,
         opt, rhs_buf, data->type->type_name);
}

static void handle_negate_overflow(bool fatal, struct overflow_data *data,
                                   void *old_data) {
  char location_buf[UBSAN_LOCATION_MAX];
  char old_data_buf[UBSAN_NUMBER_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);
  deserialize_number(location_buf, old_data_buf, sizeof(old_data_buf),
                     data->type, old_data);
  report(fatal,
         "ubsan: Undefined behavior in %s, negation of %s cannot be "
         "represented in type %s\n",
         location_buf, old_data_buf, data->type->type_name);
}

static void handle_alignment_assumption(bool fatal,
                                        struct alignment_assumption_data *data,
                                        unsigned long ptr, unsigned long align,
                                        unsigned long offset) {
  char location_buf[UBSAN_LOCATION_MAX];
  char assumption_location[UBSAN_LOCATION_MAX];
  unsigned long real_ptr;

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);
  real_ptr = ptr - offset;

  if (data->assumption_location.file_name != NULL) {
    deserialize_location(assumption_location, sizeof(assumption_location),
                         &data->assumption_location);
    report(fatal,
           "ubsan: Undefined behavior in %s, alignment assumption of %#lx for "
           "pointer %#lx (offset %#lx), assumption made in %s\n",
           location_buf, align, real_ptr, offset, assumption_location);
  } else {
    report(fatal,
           "ubsan: Undefined behavior in %s, alignment assumption of %#lx for "
           "pointer %#lx (offset %#lx)\n",
           location_buf, align, real_ptr, offset);
  }
}

static void handle_type_mismatch(bool fatal, struct source_location *loc,
                                 struct type_descriptor *type,
                                 unsigned long align, uint8_t chk_kind,
                                 void *ptr) {
  char location_buf[UBSAN_LOCATION_MAX];

  if (is_reported(loc)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), loc);

  if (!ptr) {
    report(
        fatal, "ubsan: Undefined behavior in %s, %s null pointer of type %s\n",
        location_buf, deserialize_type_check_kind(chk_kind), type->type_name);
  } else if ((unsigned long)ptr & (align - 1)) {
    report(fatal,
           "ubsan: Undefined behavior in %s, %s misaligned address %p for type "
           "%s which requires %ld byte alignment\n",
           location_buf, deserialize_type_check_kind(chk_kind), ptr,
           type->type_name, align);
  } else {
    report(fatal,
           "ubsan: Undefined behavior in %s, %s address %p with insufficient "
           "space for an object of type %s\n",
           location_buf, deserialize_type_check_kind(chk_kind), ptr,
           type->type_name);
  }
}

static void handle_out_of_bounds(bool fatal, struct out_of_bounds_data *data,
                                 void *index) {
  char location_buf[UBSAN_LOCATION_MAX];
  char index_buf[UBSAN_NUMBER_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);
  deserialize_number(location_buf, index_buf, sizeof(index_buf),
                     data->index_type, index);

  report(
      fatal,
      "ubsan: Undefined behavior in %s, index %s is out of range for type %s\n",
      location_buf, index_buf, data->array_type->type_name);
}

static void handle_shift_out_of_bounds(bool fatal,
                                       struct shift_out_of_bounds_data *data,
                                       void *lhs, void *rhs) {
  char location_buf[UBSAN_LOCATION_MAX];
  char lhs_buf[UBSAN_NUMBER_MAX];
  char rhs_buf[UBSAN_NUMBER_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);
  deserialize_number(location_buf, lhs_buf, sizeof(lhs_buf), data->lhs_type,
                     lhs);
  deserialize_number(location_buf, rhs_buf, sizeof(rhs_buf), data->rhs_type,
                     rhs);

  if (is_negative_number(location_buf, data->rhs_type, rhs)) {
    report(fatal,
           "ubsan: Undefined behavior in %s, shift exponent %s is negative\n",
           location_buf, rhs_buf);
  } else if (is_shift_exponent_too_large(
                 location_buf, data->rhs_type, rhs,
                 deserialize_type_width(data->lhs_type))) {
    report(fatal,
           "ubsan: Undefined behavior in %s, shift exponent %s is too large "
           "for %zu-bit type %s\n",
           location_buf, rhs_buf, deserialize_type_width(data->lhs_type),
           data->lhs_type->type_name);
  } else if (is_negative_number(location_buf, data->lhs_type, lhs)) {
    report(fatal,
           "ubsan: Undefined behavior in %s, left shift of negative value %s\n",
           location_buf, lhs_buf);
  } else {
    report(fatal,
           "ubsan: Undefined behavior in %s, left shift of %s by %s places "
           "cannot be represented in type %s\n",
           location_buf, lhs_buf, rhs_buf, data->lhs_type->type_name);
  }
}

static void handle_builtin_unreachable(bool fatal,
                                       struct unreachable_data *data) {
  char location_buf[UBSAN_LOCATION_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);

  report(fatal,
         "ubsan: Undefined behavior in %s, calling __builtin_unreachable()\n",
         location_buf);
}

static void handle_load_invalid_value(bool fatal,
                                      struct invalid_value_data *data,
                                      void *val) {
  char location_buf[UBSAN_LOCATION_MAX];
  char value_buf[UBSAN_NUMBER_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);
  deserialize_number(location_buf, value_buf, sizeof(value_buf), data->type,
                     val);

  report(fatal,
         "ubsan: Undefined behavior in %s, load of value %s is not a valid "
         "value for type %s\n",
         location_buf, value_buf, data->type->type_name);
}

static void handle_pointer_overflow(bool fatal,
                                    struct pointer_overflow_data *data,
                                    void *base, void *result) {
  char location_buf[UBSAN_LOCATION_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);

  report(fatal,
         "ubsan: Undefined behavior in %s, pointer expression with base %#lx "
         "overflowed to %#lx\n",
         location_buf, (unsigned long)base, (unsigned long)result);
}

static void handle_nonnull_arg(bool fatal, struct nonnull_arg_data *data) {
  char location_buf[UBSAN_LOCATION_MAX];
  char attr_location[UBSAN_LOCATION_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);
  if (data->attr_location.file_name) {
    deserialize_location(attr_location, sizeof(attr_location),
                         &data->attr_location);
  } else {
    attr_location[0] = '\0';
  }

  report(fatal,
         "ubsan: Undefined behavior in %s, null pointer passed as argument %d, "
         "which is declared to never be null%s%s\n",
         location_buf, data->arg_index,
         data->attr_location.file_name ? ", nonnull/_Nonnull specified in "
                                       : "",
         attr_location);
}

void handle_float_cast_overflow(bool fatal,
                                struct float_cast_overflow_data *data,
                                void *from) {
  char location_buf[UBSAN_LOCATION_MAX];
  char from_buf[UBSAN_NUMBER_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);
  deserialize_number(location_buf, from_buf, sizeof(from_buf), data->from_type,
                     from);

  report(fatal,
         "ubsan: Undefined behavior in %s, %s (of type %s) is outside the "
         "range of representable values of type %s\n",
         location_buf, from_buf, data->from_type->type_name,
         data->to_type->type_name);
}

static void handle_function_type_mismatch(
    bool fatal, struct function_type_mismatch_data *data, void *func) {
  char location_buf[UBSAN_LOCATION_MAX];

  /*
   * There is no a portable C solution to translate an address of a
   * function to its name. On the cost of getting this routine simple
   * and portable without ifdefs between the userland and the kernel
   * just print the address of the function as-is.
   *
   * For better diagnostic messages in the userland, users shall use
   * the full upstream version shipped along with the compiler toolchain.
   */
  assert(data);

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);

  report(fatal,
         "ubsan: Undefined behavior in %s, call to function %#lx through "
         "pointer to incorrect function type %s\n",
         location_buf, (unsigned long)func, data->type->type_name);
}

static void handle_implicit_conversion(bool fatal,
                                       struct implicit_conversion_data *data,
                                       void *from, void *to) {
  char location_buf[UBSAN_LOCATION_MAX];
  char from_buf[UBSAN_NUMBER_MAX];
  char to_buf[UBSAN_NUMBER_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);
  deserialize_number(location_buf, from_buf, sizeof(from_buf), data->from_type,
                     from);
  deserialize_number(location_buf, to_buf, sizeof(to_buf), data->to_type, to);

  report(fatal,
         "ubsan: Undefined behavior in %s, %s from %s %zu-bit %s (%s) to %s "
         "changed the value to %s %zu-bit %s\n",
         location_buf, deserialize_implicit_conversion_check_kind(data->kind),
         from_buf, deserialize_type_width(data->from_type),
         data->from_type->type_info & UBSAN_SIGNED ? "signed" : "unsigned",
         data->from_type->type_name, data->to_type->type_name, to_buf,
         deserialize_type_width(data->to_type),
         data->to_type->type_info & UBSAN_SIGNED ? "signed" : "unsigned");
}

static void handle_invalid_builtin(bool fatal,
                                   struct invalid_builtin_data *data) {
  char location_buf[UBSAN_LOCATION_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);

  report(fatal,
         "ubsan: Undefined behavior in %s, passing zero to %s, which is not a "
         "valid argument\n",
         location_buf, deserialize_builtin_check_kind(data->kind));
}

static void handle_missing_return(bool fatal, struct unreachable_data *data) {
  char location_buf[UBSAN_LOCATION_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);

  report(fatal,
         "ubsan: Undefined behavior in %s, execution reached the end of a "
         "value-returning function without returning a value\n",
         location_buf);
}

static void handle_nonnull_return(bool fatal, struct nonnull_return_data *data,
                                  struct source_location *loc) {
  char location_buf[UBSAN_LOCATION_MAX];
  char attr_location[UBSAN_LOCATION_MAX];

  if (is_reported(loc)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), loc);
  if (data->attr_location.file_name) {
    deserialize_location(attr_location, sizeof(attr_location),
                         &data->attr_location);
  } else {
    attr_location[0] = '\0';
  }

  report(fatal,
         "ubsan: Undefined behavior in %s, null pointer returned from function "
         "declared to never return null%s%s\n",
         location_buf,
         data->attr_location.file_name ? ", nonnull/_Nonnull specified in "
                                       : "",
         attr_location);
}

static void handle_vla_bound_not_positive(
    bool fatal, struct vla_bound_not_positive_data *data, void *bound) {
  char location_buf[UBSAN_LOCATION_MAX];
  char bound_buf[UBSAN_NUMBER_MAX];

  if (is_reported(&data->location)) {
    return;
  }

  deserialize_location(location_buf, sizeof(location_buf), &data->location);
  deserialize_number(location_buf, bound_buf, sizeof(bound_buf), data->type,
                     bound);

  report(fatal,
         "ubsan: Undefined behavior in %s, variable length array bound value "
         "%s <= 0\n",
         location_buf, bound_buf);
}

static void report(bool fatal, const char *fmt, ...) {
  va_list va;

  va_start(va, fmt);
  (void)vfprintf(stderr, fmt, va);
  va_end(va);

  if (fatal) {
    abort();
  }
}
