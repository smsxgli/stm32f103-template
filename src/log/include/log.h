#ifndef LOG_H_DEFINED
#define LOG_H_DEFINED

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int log_write(const char *, size_t) __attribute__((__nonnull__));
/* block until all output is flushed */
extern void log_flush(void);
extern int log_read(char *, size_t) __attribute__((__nonnull__));

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* LOG_H_DEFINED */
