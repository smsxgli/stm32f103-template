#ifndef BITOPS_H_DEFINED
#define BITOPS_H_DEFINED

#include "utils.h"
#include <limits.h>

#define clz(_x) no_side_effect_1(__clz, _x)
#define __clz(_x)                                                              \
  __builtin_choose_expr(                                                       \
      __builtin_types_compatible_p(__typeof__(_x), unsigned int),              \
      __builtin_clz((unsigned int)(_x)),                                       \
      __builtin_choose_expr(                                                   \
          __builtin_types_compatible_p(__typeof__(_x), unsigned long),         \
          __builtin_clzl((unsigned long)(_x)),                                 \
          __builtin_choose_expr(__builtin_types_compatible_p(                  \
                                    __typeof__(_x), unsigned long long),       \
                                __builtin_clzll((unsigned long long)(_x)),     \
                                (void)0)))

#define is_power_of_two(_x) no_side_effect_1(__is_power_of_two, _x)
#define __is_power_of_two(_x) (((_x) != 0) && (0 == ((_x) & ((_x)-1))))

#define roundup_power_of_two(_x) no_side_effect_1(__roundup_power_of_two, _x)
#define __roundup_power_of_two(_x)                                             \
  (0 == (_x) ? 1 : (__typeof__(_x))1 << (CHAR_BIT * sizeof(_x) - __clz(_x)))

#endif /* BITOPS_H_DEFINED */
