#include "mpmc.h"

#define mpmc_impl_val_t uint16_t
#define mpmc_impl_raw_t uint32_t
#define mpmc_impl_suffix u32
#define MPMC_IMPL_RTS_MAX_DIST (mpmc_impl_val_t)(128)

#include "./mpmc_impl.h"

#undef mpmc_impl_val_t
#undef mpmc_impl_raw_t
#undef mpmc_impl_suffix
#undef MPMC_IMPL_RTS_MAX_DIST

#define mpmc_impl_val_t uint32_t
#define mpmc_impl_raw_t uint64_t
#define mpmc_impl_suffix u64
#define MPMC_IMPL_RTS_MAX_DIST (mpmc_impl_val_t)(512)

#include "./mpmc_impl.h"

#undef mpmc_impl_val_t
#undef mpmc_impl_raw_t
#undef mpmc_impl_suffix
#undef MPMC_IMPL_RTS_MAX_DIST
