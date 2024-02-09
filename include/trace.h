#ifndef TRACE_H_DEFINED
#define TRACE_H_DEFINED

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int trace_init(void);
extern int trace_write(const char *, size_t) __attribute__((__nonnull__));
/* block until all output is flushed */
extern void trace_flush(void);
extern int trace_read(char *, size_t) __attribute__((__nonnull__));

#ifdef __cplusplus
}
#endif
#endif /* TRACE_H_DEFINED */
