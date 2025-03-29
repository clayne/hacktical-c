#include <assert.h>
#include <string.h>
#include "stream.h"

void stream_tests() {
  struct hc_memory_stream s;
  hc_memory_stream_init(&s);
  hc_defer(hc_stream_deinit(&s));
  hc_stream_printf(&s, "%s%d", "foo", 42);
  assert(strcmp((const char *)hc_memory_stream_string(&s), "foo42") == 0);
}
