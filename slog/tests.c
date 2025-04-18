#include <assert.h>
#include <string.h>

#include "slog.h"

void slog_tests() {
  struct hc_memory_stream out;
  hc_memory_stream_init(&out);
  
  struct hc_slog_stream s;
  hc_slog_stream_init(&s, &out.stream, .close_out=true);
  
  hc_slog_do(&s) {
    hc_slog_context_do(hc_slog_string("string", "abc")) {
      struct hc_time t = hc_time(2025, 4, 13, 1, 40, 0);
      
      hc_slog_write(hc_slog_bool("bool", true),
		    hc_slog_int("int", 42),
		    hc_slog_time("time", t));
    }
  }

  assert(strcmp("string=\"abc\", "
		"bool=true, "
		"int=42, "
		"time=2025-04-13T01:40:00\n",
		hc_memory_stream_string(&out)) == 0);
  
  hc_slog_deinit(&s);  
}
