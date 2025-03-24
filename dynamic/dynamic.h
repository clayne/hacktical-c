#ifndef HACKTICAL_DYNAMIC_H
#define HACKTICAL_DYNAMIC_H

#include <stdarg.h>
#include <stdio.h>

char *hc_vsprintf(const char *format, va_list args);

struct hc_proc {
  int pid;
  FILE *stdin;
};

struct hc_proc hc_exec(const char *path, ...);
void hc_compile(const char *code, const char *out);

#endif
