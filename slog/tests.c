#include <assert.h>
#include <string.h>

#include "slog.h"

struct my_slog {
  struct hc_slog slog;
};
  
void slog_tests() {
  struct hc_memory_stream out;
  hc_memory_stream_init(&out);
  
  struct my_slog s;
  hc_slog_init(&s, &out.stream, true);
  
  hc_slog_do(&s) {
    hc_slog_write(hc_slog_string("foo", "bar"), hc_slog_int("baz", 42));
  }

  assert(strcmp("foo=\"bar\", baz=42\n", hc_memory_stream_string(&out)) == 0);
  hc_slog_deinit(&s);  
}
