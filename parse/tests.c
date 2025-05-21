#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "parse.h"

static void alpha_test() {
  struct hc_parser *p = hc_parse_alpha(42);
  const char *in = "abc";
  struct hc_list out;
  hc_list_init(&out);
  assert(hc_parse(p, in, &out) == 3);

  struct hc_parsed *pr = hc_baseof(out.next, struct hc_parsed, parent);
  assert(pr->id == 42);
  assert(pr->start == 0);
  assert(pr->end == 1);

  pr = hc_baseof(pr->parent.next, struct hc_parsed, parent);
  assert(pr->id == 42);
  assert(pr->start == 1);
  assert(pr->end == 2);

  pr = hc_baseof(pr->parent.next, struct hc_parsed, parent);
  assert(pr->id == 42);
  assert(pr->start == 2);
  assert(pr->end == 3);

  hc_parser_free(p);
  hc_parsed_free(&out);
}

static void and_test() {
  struct hc_parser *p = hc_parse_and(0,
				     hc_parse_space(0),
				     hc_parse_alpha(42));
  const char *in = " a bc";
  struct hc_list out;
  hc_list_init(&out);
  assert(hc_parse(p, in, &out) == 5);

  struct hc_parsed *pr = hc_baseof(out.next, struct hc_parsed, parent);
  assert(pr->id == 42);
  assert(pr->start == 1);
  assert(pr->end == 2);

  pr = hc_baseof(pr->parent.next, struct hc_parsed, parent);
  assert(pr->id == 42);
  assert(pr->start == 3);
  assert(pr->end == 4);

  pr = hc_baseof(pr->parent.next, struct hc_parsed, parent);
  assert(pr->id == 42);
  assert(pr->start == 4);
  assert(pr->end == 5);

  hc_parser_free(p);
  hc_parsed_free(&out);
}

static void delim_test() {
  struct hc_parser *p = hc_parse_and(0,
				     hc_parse_char(0, '('),
				     hc_parse_many(42, hc_parse_any()),
				     hc_parse_char(0, ')'));
  const char *in = "(abc)";
  struct hc_list out;
  hc_list_init(&out);
  assert(hc_parse(p, in, &out) == 5);

  struct hc_parsed *pr = hc_baseof(out.next, struct hc_parsed, parent);
  assert(pr->id == 42);
  assert(pr->start == 1);
  assert(pr->end == 4);

  hc_parser_free(p);
  hc_parsed_free(&out);
}

static void not_char_test() {
  struct hc_parser *p =
    hc_parse_and(0, hc_parse_many(42, hc_parse_char(0, -'!')));
  
  const char *in = "abc!";
  struct hc_list out;
  hc_list_init(&out);
  assert(hc_parse(p, in, &out) == 3);

  struct hc_parsed *pr = hc_baseof(out.next, struct hc_parsed, parent);
  assert(pr->id == 42);
  assert(pr->start == 0);
  assert(pr->end == 3);

  hc_parser_free(p);
  hc_parsed_free(&out);
}

void parse_tests() {
  alpha_test();
  and_test();
  delim_test();
  not_char_test();
}
