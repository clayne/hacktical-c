#include <assert.h>
#include <string.h>
#include "stream1.h"

void stream1_tests() {
  struct hc_memory_stream s;
  hc_memory_stream_init(&s, &hc_malloc_default);
  hc_defer(hc_stream_deinit(&s.stream));
  hc_printf(&s.stream, "%s%d", "foo", 42);
  assert(strcmp("foo42", hc_memory_stream_string(&s)) == 0);
}
