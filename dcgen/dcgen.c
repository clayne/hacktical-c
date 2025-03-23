#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dcgen.h"
#include "error/error.h"
#include "malloc/malloc.h"

char *hc_vsprintf(const char *format, va_list args) {
  va_list tmp_args;
  va_copy(tmp_args, args);
  int len = vsnprintf(NULL, 0, format, tmp_args);
  va_end(tmp_args);

  if (len < 0) {
    hc_throw(0, "Formatting '%s' failed: %d", format, errno);
  }

  len++;
  char *out = hc_acquire(len);
  vsnprintf(out, len, format, args);
  return out;
}

void hc_exec(const char *cmd, ...) {
  va_list args;
  va_start(args, cmd);
  char *c = hc_vsprintf(cmd, args);
  va_end(args);
  const int ec = system(c);
  
  if (ec != 0) {
    hc_throw(0, "Command '%s' failed with exit code %d", ec);
  }
}

void hc_compile(const char *code, const char *out) {
  hc_exec("gcc -shared %s -o -xc - | %s", out, code);  
}
