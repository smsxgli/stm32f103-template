#include "log.h"
#ifdef NDEBUG
#include "firmware.h"
#endif
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#undef errno

#define unlikely(_x) __builtin_expect((_x), 0)

extern int errno;

extern void __initialize_args(int *, char ***) __attribute__((__weak__));

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
  char buff[64];
  int status;

  status = snprintf(buff, sizeof(buff), "_exit: code: %d\n", code);
  if (status < 0) {
    strncpy(buff, "_exit: code: fmt failed!\n", sizeof(buff));
  }
  _write(2, buff, strlen(buff));
  /* TODO: make sure all log is flushed */
  log_flush();
  /* In devel, we shall be noticed the app is exited */
  for (;;) {
#ifndef NDEBUG
    __asm__ volatile("bkpt");
#else
    fw_reset();
#endif
  }
}

int _close(__attribute__((__unused__)) int fd) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

int _execve(__attribute__((__unused__)) char *name,
            __attribute__((__unused__)) char **argv,
            __attribute__((__unused__)) char **env) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

int _fork(void) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

int _fstat(__attribute__((__unused__)) int fd,
           __attribute__((__unused__)) struct stat *st) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

int _getpid(void) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

int _isatty(__attribute__((__unused__)) int fd) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return 0;
}

int _kill(__attribute__((__unused__)) int pid,
          __attribute__((__unused__)) int sig) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

int _link(__attribute__((__unused__)) char *old,
          __attribute__((__unused__)) char *new) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

int _lseek(__attribute__((__unused__)) int fd,
           __attribute__((__unused__)) int ptr,
           __attribute__((__unused__)) int dir) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

int _open(__attribute__((__unused__)) char *file,
          __attribute__((__unused__)) int flags,
          __attribute__((__unused__)) int mode) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

int _read(int fd, char *ptr, size_t len) {
  if (0 == fd) {
    return log_read(ptr, len);
  }
  __atomic_store_n(&errno, EBADF, __ATOMIC_RELAXED);
  return -1;
}

caddr_t _sbrk(int incr) {
  extern char _end; /* Defined by the linker */
  static char *head_end;
  char *prev_head_end;
  register char *stack_ptr __asm__("sp");
  char *expected = 0;

  __atomic_compare_exchange_n(&head_end, &expected, &_end, 0, __ATOMIC_RELEASE,
                              __ATOMIC_ACQUIRE);
  prev_head_end = __atomic_fetch_add(&head_end, incr, __ATOMIC_ACQ_REL);
  if (unlikely(prev_head_end + incr > stack_ptr)) {
    const char *msg = "_sbrk: Heap and stack collision!\n";
    _write(2, msg, strlen(msg));
    abort();
  }
  /* write done */
  return (caddr_t)prev_head_end;
}

int _stat(__attribute__((__unused__)) const char *file,
          __attribute__((__unused__)) struct stat *st) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

clock_t _times(__attribute__((__unused__)) struct tms *buf) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return (clock_t)-1;
}

int _unlink(__attribute__((__unused__)) char *name) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

int _wait(__attribute__((__unused__)) int *status) {
  __atomic_store_n(&errno, ENOSYS, __ATOMIC_RELAXED);
  return -1;
}

int _write(int fd, const char *ptr, size_t len) {
  if (1 == fd || 2 == fd) {
    return log_write(ptr, len);
  }
  __atomic_store_n(&errno, EBADF, __ATOMIC_RELAXED);
  return -1;
}

void __initialize_args(int *p_argc, char ***p_argv) {
  static char name[] = "";
  static char *argv[2] = {name, NULL};
  *p_argc = 1;
  *p_argv = &argv[0];
}
