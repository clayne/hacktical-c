#ifndef HACKTICAL_DCGEN_H
#define HACKTICAL_DCGEN_H

#include <stdarg.h>

char *hc_vsprintf(const char *format, va_list args);
void hc_exec(const char *path, ...);

#endif
