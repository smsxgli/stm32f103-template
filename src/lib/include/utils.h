#ifndef UTILS_H_DEFINED
#define UTILS_H_DEFINED

#include <stddef.h>
#include <assert.h>

#define token_to_str(_x) __token_to_str(_x)
#define __token_to_str(_x) #_x

#define token_cat(_x, _y) __token_cat(_x, _y)
#define __token_cat(_x, _y) _x##_y

#define likely(_x) __builtin_expect(!!(_x), 1)
#define unlikely(_x) __builtin_expect(!!(_x), 0)

#define no_side_effect_1(_macro, _x, ...)                                      \
  __no_side_effect_1(_macro, _x, __COUNTER__, ##__VA_ARGS__)
#define __no_side_effect_1(_macro, _x, _uniq, ...)                             \
  __builtin_choose_expr(__builtin_constant_p(_x), _macro(_x, ##__VA_ARGS__),   \
                        __extension__({                                        \
                          __auto_type token_cat(__x, _uniq) = (_x);            \
                          _macro(token_cat(__x, _uniq), ##__VA_ARGS__);        \
                        }))

#define no_side_effect_2(_macro, _x, _y, ...)                                  \
  __no_side_effect_2(_macro, _x, _y, __COUNTER__, ##__VA_ARGS__)
#define __no_side_effect_2(_macro, _x, _y, _uniq, ...)                         \
  __builtin_choose_expr(__builtin_constant_p(_x) && __builtin_constant_p(_y),  \
                        _macro(_x, _y, ##__VA_ARGS__), __extension__({         \
                          __auto_type token_cat(__x, _uniq) = (_x);            \
                          __auto_type token_cat(__y, _uniq) = (_y);            \
                          _macro(token_cat(__x, _uniq), token_cat(__y, _uniq), \
                                 ##__VA_ARGS__);                               \
                        }))

#define is_pointer_or_array(_x) (5 == __builtin_classify_type(_x))

#define is_pointer(_p) no_side_effect_1(__is_pointer, _p)
#define __is_pointer(_p)                                                       \
  __builtin_types_compatible_p(__typeof__(_p), __pointer_decay(_p))
#define __pointer_decay(_p)                                                    \
  (&*__builtin_choose_expr(is_pointer_or_array(_p), (_p), NULL))

#define is_array(_x) no_side_effect_1(__is_array, _x)
#define __is_array(_x) (is_pointer_or_array(_x) && !is_pointer(_x))

#define array_size(_x)                                                         \
  __extension__({                                                              \
    static_assert(is_array(_x), "'array_size' invoked with non-array param!"); \
    no_side_effect_1(__array_size, _x);                                        \
  })
#define __array_size(_x) (sizeof(_x) / sizeof(_x)[0])

#define container_of(_ptr, _type, _member)                                     \
  __extension__({                                                              \
    static_assert(__container_of_type_chk(_ptr, _type, _member),               \
                  "'container_of' invoked with invalid param!");               \
    no_side_effect_1(__container_of, _ptr, _type, _member);                    \
  })
#define __container_of_type_chk(_ptr, _type, _member)                          \
  (__builtin_types_compatible_p(__typeof__(*(_ptr)),                           \
                                __typeof__((_type){}._member)) ||              \
   __builtin_types_compatible_p(__typeof__(_ptr), __typeof__(NULL)))
#define __container_of(_ptr, _type, _member)                                   \
  ((_ptr)                                                                      \
       ? (_type *)__container_of_ptr_wrapper(_ptr, offsetof(_type, _member))   \
       : NULL)

#define min(_x, _y) no_side_effect_2(__min, _x, _y)
// #define __min(_x, _y) ((_x) < (_y) ? (_x) : (_y))
#define __min(_x, _y)                                                          \
  __extension__({                                                              \
    static_assert(                                                             \
        __builtin_types_compatible_p(__typeof__(_x), __typeof__(_y)), "");     \
    (__typeof__(_x))((_x) < (_y) ? (_x) : (_y));                               \
  })

#define max(_x, _y) no_side_effect_2(__max, _x, _y)
// #define __max(_x, _y) ((_x) < (_y) ? (_y) : (_x))
#define __max(_x, _y)                                                          \
  __extension__({                                                              \
    static_assert(                                                             \
        __builtin_types_compatible_p(__typeof__(_x), __typeof__(_y)), "");     \
    (__typeof__(_x))((_x) < (_y) ? (_y) : (_x));                               \
  })

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static inline void *__container_of_ptr_wrapper(const void *__ptr,
                                               size_t __offset) {
  /*
   * Arithmetic on NULL is UB, even if in dead-code. Hide it in a proper
   * C function, so the macro never emits it as code.
   */
  return (char *)__ptr - __offset;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* UTILS_H_DEFINED */
