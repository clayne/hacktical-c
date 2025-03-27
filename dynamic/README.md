## Dynamic Compilation

*Never put off till run-time what you can do at compile-time.*

~ D. Gries

To implement dynamic compilation in C, we'll have to cast a few non-trivial Unix spells and sacrifice some portability in the process. It's not that these features aren't available on other platforms; rather that they're implemented in slightly different ways, using different names.

Example:
```C
  const char *out = "/var/tmp/libtest.so";
  
  hc_compile("#include <stdio.h>\n"
	     "int test() { return 42; }",
	     out);

  struct hc_dlib lib;
  hc_dlib_init(&lib, out);
  hc_defer(hc_dlib_deinit(&lib));
  int (*fn)() = hc_dlib_find(&lib, "test");
  assert(fn() == 42);
```

The star of the show is `hc_compile()` which allows dynamically creating shared libraries from source code.

```C
void hc_compile(const char *code, const char *out) {
  const char *cmd = "/usr/bin/gcc -shared -fpic -o %s -xc -";
  struct hc_proc child;
  hc_proc_init(&child, cmd, out);
  hc_defer(hc_proc_deinit(&child));
  FILE *stdin = fdopen(child.stdin, "w");
  fputs(code, stdin);
  fclose(stdin);
}
```

Starting an external process with a pipe attached to `stdin` gets a tiny bit involved, and non-trivial to refactor into smaller pieces; but goes someting like this.

First we create a pipe using `pipe()`. Next we `fork()` a new process, attach one of the pipe ends to `stdin` using `dup2()` and execute the specified command using `execve()`. In the parent process we clean up the mess we made and initialize the struct. 

```C
struct hc_proc {
  int pid;
  int stdin;
};

struct hc_proc *hc_proc_init(struct hc_proc *proc, const char *cmd, ...) {
  va_list args;
  va_start(args, cmd);
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
    hc_throw(0, "Failed forking process: %d", errno);
  default:
    close(fds[0]);
    free(cp);
    
    hc_vector_do(&cas, ca) {
      free(*(char **)ca);
    }
    
    hc_vector_deinit(&cas);
    hc_release(c);
    return (struct hc_proc){.pid = child_pid, .stdin = fds[1]};
  }
}
```

`struct hc_dlib` handles loading shared libraries and looking up symbols.

```C
struct hc_dlib {
  void *handle;
};

struct hc_dlib *hc_dlib_init(struct hc_dlib *lib, const char *path) {
  lib->handle = dlopen(path, RTLD_NOW);

  if (!lib->handle) {
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

  if (e) {
    hc_throw(0, "Symbol '%s' not found: %s", e);
  }

  return v;
}
```