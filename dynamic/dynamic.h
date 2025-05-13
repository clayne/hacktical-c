#ifndef HACKTICAL_DYNAMIC_H
#define HACKTICAL_DYNAMIC_H

#include <stdarg.h>
#include <stdio.h>

struct hc_proc {
  int pid;
  int in;
};

#define hc_proc_init(p, ...)			\
  _hc_proc_init(p, {__VA_ARGS__, NULL})

struct hc_proc *_hc_proc_init(struct hc_proc *p, char *cmd[]);
void hc_proc_wait(struct hc_proc *p);
void hc_proc_deinit(struct hc_proc *p);

struct hc_compile_opts {
  const char *cc;
  const char **cflags;
};

#define hc_compile(code, out, ...)				\
  _hc_compile(code, out, (struct hc_compile_opts){		\
      .cc = "/usr/bin/gcc",					\
      .cflags = (const char *[]){NULL},				\
      ##__VA_ARGS__						\
    })

void _hc_compile(const char *code,
		 const char *out,
		 struct hc_compile_opts opts);

struct hc_dlib {
  void *handle;
};

struct hc_dlib *hc_dlib_init(struct hc_dlib *lib, const char *path);
struct hc_dlib *hc_dlib_deinit(struct hc_dlib *lib);
void *hc_dlib_find(const struct hc_dlib *lib, const char *s);

char *hc_vsprintf(const char *format, va_list args);

#endif
