#include "./ubsan.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void __libubsan_report(bool fatal, const char *fmt, ...) {
  va_list va;

  va_start(va, fmt);
  (void)vfprintf(stderr, fmt, va);
  va_end(va);

  if (fatal) {
    abort();
  }
}

void __libubsan_abort(void) { abort(); }
