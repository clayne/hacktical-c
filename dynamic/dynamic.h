#ifndef HACKTICAL_DYNAMIC_H
#define HACKTICAL_DYNAMIC_H

#include <stdarg.h>
#include <stdio.h>

struct hc_proc {
  int pid;
  int stdin;
};

struct hc_proc hc_exec(const char *path, ...);
void hc_compile(const char *code, const char *out);
char *hc_vsprintf(const char *format, va_list args);

struct hc_dlib {
  void *handle;
};

struct hc_dlib *hc_dlib_init(struct hc_dlib *lib, const char *path);
struct hc_dlib *hc_dlib_deinit(struct hc_dlib *lib);
void *hc_dlib_find(const struct hc_dlib *lib, const char *s);

#endif
