#include "rbuf.h"

#define rbuf_impl_val_t uint16_t
#define rbuf_impl_suffix u16

#include "./rbuf_impl.h"

#undef rbuf_impl_val_t
#undef rbuf_impl_suffix

#define rbuf_impl_val_t uint32_t
#define rbuf_impl_suffix u32

#include "./rbuf_impl.h"

#undef rbuf_impl_val_t
#undef rbuf_impl_suffix
