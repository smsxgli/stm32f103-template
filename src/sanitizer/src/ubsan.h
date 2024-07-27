#ifndef LIB_UBSAN_H_DEFINED
#define LIB_UBSAN_H_DEFINED

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*
 * ABI defined by Clang's UBSAN enum SanitizerHandler:
 * https://github.com/llvm/llvm-project/blob/release/16.x/clang/lib/CodeGen/CodeGenFunction.h#L113
 */

enum ubsan_checks {
  ubsan_add_overflow,
  ubsan_builtin_unreachable,
  ubsan_divrem_overflow,
  ubsan_float_cast_overflow,
  ubsan_function_type_mismatch,
  ubsan_implicit_conversion,
  ubsan_invalid_builtin,
  ubsan_load_invalid_value,
  ubsan_missing_return,
  ubsan_mul_overflow,
  ubsan_negate_overflow,
  ubsan_nullability_arg,
  ubsan_nullability_return,
  ubsan_nonnull_arg,
  ubsan_nonnull_return,
  ubsan_out_of_bounds,
  ubsan_pointer_overflow,
  ubsan_shift_out_of_bounds,
  ubsan_sub_overflow,
  ubsan_type_mismatch,
  ubsan_alignment_assumption,
  ubsan_vla_bound_not_positive,
};

enum { type_kind_int = 0, type_kind_float = 1, type_unknown = 0xffff };

struct type_descriptor {
  uint16_t type_kind;
  uint16_t type_info;
  char type_name[1];
};

struct source_location {
  const char *file_name;
  union {
    unsigned long reported;
    struct {
      uint32_t line;
      uint32_t column;
    };
  };
};

struct overflow_data {
  struct source_location location;
  struct type_descriptor *type;
};

struct type_mismatch_data {
  struct source_location location;
  struct type_descriptor *type;
  unsigned long alignment;
  unsigned char type_check_kind;
};

struct type_mismatch_data_v1 {
  struct source_location location;
  struct type_descriptor *type;
  unsigned char log_alignment;
  unsigned char type_check_kind;
};

struct type_mismatch_data_common {
  struct source_location *location;
  struct type_descriptor *type;
  unsigned long alignment;
  unsigned char type_check_kind;
};

struct nonnull_arg_data {
  struct source_location location;
  struct source_location attr_location;
  int arg_index;
};

struct out_of_bounds_data {
  struct source_location location;
  struct type_descriptor *array_type;
  struct type_descriptor *index_type;
};

struct shift_out_of_bounds_data {
  struct source_location location;
  struct type_descriptor *lhs_type;
  struct type_descriptor *rhs_type;
};

struct unreachable_data {
  struct source_location location;
};

struct invalid_value_data {
  struct source_location location;
  struct type_descriptor *type;
};

struct alignment_assumption_data {
  struct source_location location;
  struct source_location assumption_location;
  struct type_descriptor *type;
};

struct pointer_overflow_data {
  struct source_location location;
};

struct function_type_mismatch_data {
  struct source_location location;
  struct type_descriptor *type;
};

struct implicit_conversion_data {
  struct source_location location;
  struct type_descriptor *from_type;
  struct type_descriptor *to_type;
  unsigned char kind;
};

struct invalid_builtin_data {
  struct source_location location;
  unsigned char kind;
};

struct vla_bound_not_positive_data {
  struct source_location location;
  struct type_descriptor *type;
};

struct float_cast_overflow_data {
  struct source_location location;
  struct type_descriptor *from_type;
  struct type_descriptor *to_type;
};

struct nonnull_return_data {
  struct source_location attr_location;
};

#ifdef __SIZEOF_INT128__
typedef __int128_t s_max;
typedef unsigned __int128_t u_max;
#else
typedef int64_t s_max;
typedef uint64_t u_max;
#endif

extern void __ubsan_handle_add_overflow(void *data, void *lhs, void *rhs);
extern void __ubsan_handle_add_overflow_abort(void *data, void *lhs, void *rhs);
extern void __ubsan_handle_sub_overflow(void *data, void *lhs, void *rhs);
extern void __ubsan_handle_sub_overflow_abort(void *data, void *lhs, void *rhs);
extern void __ubsan_handle_mul_overflow(void *data, void *lhs, void *rhs);
extern void __ubsan_handle_mul_overflow_abort(void *data, void *lhs, void *rhs);
extern void __ubsan_handle_negate_overflow(void *data, void *old_val);
extern void __ubsan_handle_negate_overflow_abort(void *data, void *old_val);
extern void __ubsan_handle_divrem_overflow(void *data, void *lhs, void *rhs);
extern void __ubsan_handle_divrem_overflow_abort(void *data, void *lhs,
                                                 void *rhs);
extern void __ubsan_handle_type_mismatch(struct type_mismatch_data *data,
                                         void *ptr);
extern void __ubsan_handle_type_mismatch_abort(struct type_mismatch_data *data,
                                               void *ptr);
extern void __ubsan_handle_type_mismatch_v1(void *data, void *ptr);
extern void __ubsan_handle_type_mismatch_v1_abort(void *data, void *ptr);
extern void __ubsan_handle_out_of_bounds(void *data, void *index);
extern void __ubsan_handle_out_of_bounds_abort(void *data, void *index);
extern void __ubsan_handle_shift_out_of_bounds(void *data, void *lhs,
                                               void *rhs);
extern void __ubsan_handle_shift_out_of_bounds_abort(void *data, void *lhs,
                                                     void *rhs);
extern void __ubsan_handle_builtin_unreachable(void *data);
extern void __ubsan_handle_load_invalid_value(void *data, void *val);
extern void __ubsan_handle_load_invalid_value_abort(void *data, void *val);
extern void __ubsan_handle_alignment_assumption(void *data, unsigned long ptr,
                                                unsigned long align,
                                                unsigned long offset);
extern void __ubsan_handle_alignment_assumption_abort(void *data,
                                                      unsigned long ptr,
                                                      unsigned long align,
                                                      unsigned long offset);
extern void __ubsan_handle_pointer_overflow(void *data, void *base,
                                            void *result);
extern void __ubsan_handle_pointer_overflow_abort(void *data, void *base,
                                                  void *result);
extern void __ubsan_handle_nonnull_arg(void *data);
extern void __ubsan_handle_nonnull_arg_abort(void *data);
extern void __ubsan_handle_nullability_arg(void *data);
extern void __ubsan_handle_nullability_arg_abort(void *data);
extern void __ubsan_handle_float_cast_overflow(void *data, void *from);
extern void __ubsan_handle_float_cast_overflow_abort(void *data, void *from);
extern void __ubsan_handle_function_type_mismatch(void *data, void *func);
extern void __ubsan_handle_function_type_mismatch_abort(void *data, void *func);
extern void __ubsan_handle_implicit_conversion(void *data, void *from,
                                               void *to);
extern void __ubsan_handle_implicit_conversion_abort(void *data, void *from,
                                                     void *to);
extern void __ubsan_handle_invalid_builtin(void *data);
extern void __ubsan_handle_invalid_builtin_abort(void *data);
extern void __ubsan_handle_missing_return(void *data);
extern void __ubsan_handle_nullability_return_v1(void *data,
                                                 struct source_location *loc);
extern void
__ubsan_handle_nullability_return_v1_abort(void *data,
                                           struct source_location *loc);
extern void __ubsan_handle_vla_bound_not_positive(void *data, void *bound);
extern void __ubsan_handle_vla_bound_not_positive_abort(void *data,
                                                        void *bound);
/* internal use, so declared weak here */
extern void __libubsan_report(bool fatal, const char *fmt, ...)
    __attribute__((__weak__, __format__(printf, 2, 3)));
extern void __libubsan_abort(void) __attribute__((__weak__, __noreturn__));

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* LIB_UBSAN_H_DEFINED */
