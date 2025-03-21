#ifndef HACKTICAL_FAIL_H
#define HACKTICAL_FAIL_H

#include <stdio.h>
#include <stdlib.h>

#define hc_fail(spec, ...) do {					\
    fprintf(stderr, "Fatal error in %s, line %d:\n" spec,	\
	    __FILE__, __LINE__, ##__VA_ARGS__);			\
    abort();							\
  } while (0);

#endif
