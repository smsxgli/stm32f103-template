#include "SEGGER_RTT.h"
#include "stm32f10x.h"
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#undef errno

extern int errno;
extern void _exit(int code) __attribute__((__noreturn__, __weak__, __used__));
extern int _close(int fd) __attribute__((__weak__, __used__));
extern int _execve(char *name, char **argv, char **env)
    __attribute__((__weak__, __used__));
extern int _fork(void) __attribute__((__weak__, __used__));
extern int _fstat(int fd, struct stat *st) __attribute__((__weak__, __used__));
extern int _getpid(void) __attribute__((__weak__, __used__));
extern int _isatty(int fd) __attribute__((__weak__, __used__));
extern int _kill(int pid, int sig) __attribute__((__weak__, __used__));
extern int _link(char *old, char *new) __attribute__((__weak__, __used__));
extern int _lseek(int fd, int ptr, int dir) __attribute__((__weak__, __used__));
extern int _open(char *file, int flags, int mode)
    __attribute__((__weak__, __used__));
extern int _read(int fd, char *ptr, size_t len)
    __attribute__((__weak__, __used__));
extern caddr_t _sbrk(int incr) __attribute__((__weak__, __used__));
extern int _stat(const char *file, struct stat *st)
    __attribute__((__weak__, __used__));
extern clock_t _times(struct tms *buf) __attribute__((__weak__, __used__));
extern int _unlink(char *name) __attribute__((__weak__, __used__));
extern int _wait(int *status) __attribute__((__weak__, __used__));
extern int _write(int fd, const char *ptr, size_t len)
    __attribute__((__weak__, __used__));

__attribute__((__used__)) int errno;

char *__env[1] = {0};
char **environ = __env;

void _exit(__attribute__((__unused__)) int code) {
  /* In devel, we shall be noticed app is exited */
  for (;;) {
#ifndef NDEBUG
    __asm__("bkpt");
#else
    NVIC_SystemReset();
#endif
  }
}

int _close(__attribute__((__unused__)) int fd) {
  errno = ENOSYS;
  return -1;
}

int _execve(__attribute__((__unused__)) char *name,
            __attribute__((__unused__)) char **argv,
            __attribute__((__unused__)) char **env) {
  errno = ENOSYS;
  return -1;
}

int _fork(void) {
  errno = ENOSYS;
  return -1;
}

int _fstat(__attribute__((__unused__)) int fd,
           __attribute__((__unused__)) struct stat *st) {
  errno = ENOSYS;
  return -1;
}

int _getpid(void) {
  errno = ENOSYS;
  return -1;
}

int _isatty(__attribute__((__unused__)) int fd) {
  errno = ENOSYS;
  return 0;
}

int _kill(__attribute__((__unused__)) int pid,
          __attribute__((__unused__)) int sig) {
  errno = ENOSYS;
  return -1;
}

int _link(__attribute__((__unused__)) char *old,
          __attribute__((__unused__)) char *new) {
  errno = ENOSYS;
  return -1;
}

int _lseek(__attribute__((__unused__)) int fd,
           __attribute__((__unused__)) int ptr,
           __attribute__((__unused__)) int dir) {
  errno = ENOSYS;
  return -1;
}

int _open(__attribute__((__unused__)) char *file,
          __attribute__((__unused__)) int flags,
          __attribute__((__unused__)) int mode) {
  errno = ENOSYS;
  return -1;
}

int _read(int fd, char *ptr, size_t len) {
  if (0 == fd) {
    unsigned bytes = SEGGER_RTT_Read(0, ptr, len);
    return (int)bytes;
  }
  errno = EBADF;
  return -1;
}

caddr_t _sbrk(int incr) {
  extern char _end; /* Defined by the linker */
  static char *head_end;
  char *prev_head_end;
  register char *stack_ptr __asm__("sp");

  if (0 == head_end) {
    head_end = &_end;
  }
  prev_head_end = head_end;
  if (head_end + incr > stack_ptr) {
    const char *msg = "_sbrk: Heap and stack collision!\n";
    _write(2, msg, strlen(msg));
    abort();
  }
  head_end += incr;
  return (caddr_t)prev_head_end;
}

int _stat(__attribute__((__unused__)) const char *file,
          __attribute__((__unused__)) struct stat *st) {
  errno = ENOSYS;
  return -1;
}

clock_t _times(__attribute__((__unused__)) struct tms *buf) {
  errno = ENOSYS;
  return (clock_t)-1;
}

int _unlink(__attribute__((__unused__)) char *name) {
  errno = ENOSYS;
  return -1;
}

int _wait(__attribute__((__unused__)) int *status) {
  errno = ENOSYS;
  return -1;
}

int _write(int fd, const char *ptr, size_t len) {
  if (fd == 1 || fd == 2) {
    unsigned bytes = SEGGER_RTT_Write(0, ptr, len);
    return (int)bytes;
  }
  errno = EBADF;
  return -1;
}
