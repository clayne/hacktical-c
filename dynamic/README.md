## Dynamic Compilation
To implement dynamic compilation in C, we'll have to cast a few non-trivial Unix spells and sacrifice some portability. It's not that these features aren't available on other platforms; rather that they're implemented in slightly different ways, using different names.

Example:
```C
  const char *out = "/var/tmp/libtest.so";
  
  hc_compile("#include <stdio.h>\n"
	     "int test() { return 42; }",
	     out,
	     .cflags = (const char *[]){"-Wall",
					"-fsanitize=undefined",
					NULL});

  struct hc_dlib lib;
  hc_dlib_init(&lib, out);
  hc_defer(hc_dlib_deinit(&lib));
  int (*fn)() = hc_dlib_find(&lib, "test");
  assert(fn() == 42);
```

The star of the show is `hc_compile()` which allows dynamically creating shared libraries from source code.

```C
struct hc_compile_opts {
  const char *cc;
  const char **cflags;
};

#define hc_compile(code, out, ...)				
  _hc_compile(code, out, (struct hc_compile_opts){		
      .cc = "/usr/bin/gcc",					
      .cflags = (const char *[]){NULL},				
      ##__VA_ARGS__						
    })

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

#define hc_proc_init(p, ...)			\
  _hc_proc_init(p, {__VA_ARGS__, NULL})

struct hc_proc *_hc_proc_init(struct hc_proc *p, char *cmd[]) {
  int fds[2];
  pipe(fds);
  pid_t child_pid = fork();

  switch (child_pid) {
  case 0: {
    close(fds[1]);
    dup2(fds[0], 0);
    char *const env[] = {"PATH=/bin:/sbin", NULL};

    if (execve(cmd[0], cmd, env) == -1) {
      hc_throw("Failed to exec '%s': %d", cmd[0], errno);
    }
  }
  case -1:
    hc_throw("Failed forking process: %d", errno);
  default:
    close(fds[0]);
    p->pid = child_pid;
    p->stdin = fds[1];
    break;
  }

  return p;
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
```