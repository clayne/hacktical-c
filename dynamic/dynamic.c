#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "dynamic.h"
#include "error/error.h"
#include "malloc1/malloc1.h"
#include "vector/vector.h"

static struct hc_vector parse_args(const char *in) {
  struct hc_vector out;
  hc_vector_init(&out, sizeof(char *));
  const char *p, *i;
  
  for (p = in; p;) {
    i = strchr(p, ' ');
    if (!i) { break; }
    *(char **)hc_vector_push(&out) = strndup(p, i - p);
    p = i+1;
  }

  if (i == NULL && *p) {
    *(char **)hc_vector_push(&out) = strdup(p);
  }

  return out;
}

struct hc_proc hc_exec(const char *cmd, ...) {
  va_list args;
  va_start(args, cmd);
  struct hc_malloc *ma = hc_malloc;
  char *c = hc_vsprintf(cmd, args);
  va_end(args);

  char *i = strchr(c, ' ');
  if (!i) { i = c + strlen(c); }
  char *cp = strndup(c, i - c);
  struct hc_vector cas = parse_args(i+1);
  int fds[2];
  pipe(fds);
  pid_t child_pid = fork();

  switch (child_pid) {
  case 0: {
    close(fds[1]);
    dup2(fds[0], 0);
    char **as = calloc(cas.length+2, sizeof(char *));
    as[0] = cp;
    
    for (int i = 0; i < cas.length; i++) {
      as[i+1] = *(char **)hc_vector_get(&cas, i);
    }

    as[cas.length+1] = NULL;
    char *const env[] = {"PATH=/bin:/sbin", NULL};
      
    if (execve(cp, as, env) == -1) {
      hc_throw(0, "Failed to exec '%s': %d", c, errno);
    }
    
    break;
  }
  case -1:
    hc_throw(0, "Failed to fork process: %d", errno);
  default:
    close(fds[0]);
    free(cp);
    
    hc_vector_do(&cas, ca) {
      free(*(char **)ca);
    }
    
    hc_vector_deinit(&cas);
    _hc_malloc_do(ma) { hc_release(c); }
    return (struct hc_proc){.pid = child_pid, .stdin = fds[1]};
  }
}

void hc_compile(const char *code, const char *out) {
  const char *cmd = "/usr/bin/gcc -shared -fpic -o %s -xc -";
  struct hc_proc child = hc_exec(cmd, out, out);
  FILE *stdin = fdopen(child.stdin, "w");
  fputs(code, stdin);
  fclose(stdin);
  close(child.stdin);
  int status;
  waitpid(child.pid, &status, 0);
}

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

struct hc_dlib *hc_dlib_init(struct hc_dlib *lib, const char *path) {
  lib->handle = dlopen(path, RTLD_NOW);

  if (lib->handle == NULL) {
    hc_throw(0, "Error opening dynamic library '%s': %s", path, dlerror());
  }
  
  return lib;
}

struct hc_dlib *hc_dlib_deinit(struct hc_dlib *lib) {
  if (dlclose(lib->handle) != 0) {
    hc_throw(0, "Failed closing dynamic library: ", dlerror());
  }

  return lib;
}

void *hc_dlib_find(const struct hc_dlib *lib, const char *s) {
  dlerror();
  void *v = dlsym(lib->handle, s);
  char *e = dlerror();

  if (e != NULL) {
    hc_throw(0, "Symbol '%s' not found: %s", e);
  }

  return v;
}
