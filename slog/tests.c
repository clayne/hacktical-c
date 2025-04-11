#include <assert.h>
#include <string.h>

#include "slog.h"

void slog_tests() {
  struct hc_memory_stream out;
  hc_memory_stream_init(&out);
  
  struct hc_slog_stream s;
  hc_slog_stream_init(&s, &out.stream, true);
  
  hc_slog_do(&s) {
    hc_slog_context_do(hc_slog_string("foo", "bar")) {
      hc_slog_write(hc_slog_int("baz", 42));
    }
  }

  assert(strcmp("foo=\"bar\", baz=42\n", hc_memory_stream_string(&out)) == 0);
  hc_slog_deinit(&s);  
}
