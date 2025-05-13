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

struct hc_proc *_hc_proc_init(struct hc_proc *p, char *cmd[]) {
  int fds[2];

  if (pipe(fds) == -1) {
    hc_throw("Failed creating pipe: %d", errno);
  }
  
  pid_t child_pid = fork();

  switch (child_pid) {
  case 0: {
    if (close(fds[1]) == -1) {
      hc_throw("Failed closing pipe writer: %d", errno);
    }
    
    if (dup2(fds[0], 0) == -1) {
      hc_throw("Failed rebinding stdin: %d", errno);
    }
    
    char *const env[] = {"PATH=/bin:/sbin", NULL};

    if (execve(cmd[0], cmd, env) == -1) {
      hc_throw("Failed to execve '%s': %d", cmd[0], errno);
    }
  }
  case -1:
    hc_throw("Failed forking process: %d", errno);
  default:
    if (close(fds[0]) == -1) {
      hc_throw("Failed closing pipe reader: %d", errno);
    }
    
    p->pid = child_pid;
    p->in = fds[1];
    break;
  }

  return p;
}

static void close_in(struct hc_proc *p) {
  if (p->in != -1 && close(p->in) == -1) {
    hc_throw("Failed closing stdin: %d", errno);
  }
}

void hc_proc_wait(struct hc_proc *p) {
  close_in(p);

  if (waitpid(p->pid, NULL, 0) == -1) {
    hc_throw("Failed waiting for child process to exit: %d", errno);
  }
}

void hc_proc_deinit(struct hc_proc *p) {
  close_in(p);
}

static void free_cmd(char **in) {
  for (char **s = in; *s; s++) {
    free(*s);
  }
}

void _hc_compile(const char *code,
		 const char *out,
		 const struct hc_compile_opts opts) {
  hc_array(const char *, pre, 
	   opts.cc, "-shared", "-fpic", "-o", out, "-xc");
  
  int n = pre_n + 2;
  for (int i = 0; opts.cflags[i]; i++, n++);  
  char *cmd[n];
  int i = 0;

  for (; i < pre_n; i++) {
    cmd[i] = strdup(pre_a[i]);
  }
  
  for (; i <  n - 2; i++) {
    cmd[i] = strdup(opts.cflags[i - pre_n]);
  }

  cmd[i++] = strdup("-");
  cmd[i] = NULL;
  hc_defer(free_cmd(cmd));
  
  struct hc_proc child;
  _hc_proc_init(&child, cmd);
  hc_defer(hc_proc_deinit(&child));
  FILE *in = fdopen(child.in, "w");

  if (!in) {
    hc_throw("Failed opening stdin stream: %d", errno);
  }
  
  child.in = -1;
  hc_defer(hc_proc_wait(&child));
  hc_defer(fclose(in));

  if (fputs(code, in) == EOF) {
    hc_throw("Failed writing code: %d", errno);
  }
}

struct hc_dlib *hc_dlib_init(struct hc_dlib *lib, const char *path) {
  lib->handle = dlopen(path, RTLD_NOW);

  if (!lib->handle) {
    hc_throw("Error opening dynamic library '%s': %s", path, dlerror());
  }
  
  return lib;
}

struct hc_dlib *hc_dlib_deinit(struct hc_dlib *lib) {
  if (dlclose(lib->handle) != 0) {
    hc_throw("Failed closing dynamic library: ", dlerror());
  }

  return lib;
}

void *hc_dlib_find(const struct hc_dlib *lib, const char *s) {
  dlerror();
  void *v = dlsym(lib->handle, s);
  char *e = dlerror();

  if (e) {
    hc_throw("Symbol '%s' not found: %s", e);
  }

  return v;
}

char *hc_vsprintf(const char *format, va_list args) {
  va_list tmp_args;
  va_copy(tmp_args, args);
  int len = vsnprintf(NULL, 0, format, tmp_args);
  va_end(tmp_args);

  if (len < 0) {
    hc_throw("Formatting '%s' failed: %d", format, errno);
  }

  len++;
  char *out = malloc(len);
  vsnprintf(out, len, format, args);
  return out;
} 
