#include "trace.h"

int trace_init(void) { return 0; }

int trace_write(const char *str, size_t len) {
  (void)str;
  (void)len;
  return 0;
}

void trace_flush(void) {}

int trace_read(char *buf, size_t len) {
  (void)buf;
  (void)len;
  return 0;
}
