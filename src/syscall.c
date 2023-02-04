#include "SEGGER_RTT.h"
#include <errno.h>
#include <sys/types.h>

extern ssize_t _write(int fd, const char *ptr, size_t len)
    __attribute__((__used__));

ssize_t _write(int fd, const char *ptr, size_t len) {
  if (fd == 1 || fd == 2) {
    SEGGER_RTT_Write(0, ptr, len);
    return 0;
  } else {
    errno = ENOSYS;
    return -1;
  }
}
