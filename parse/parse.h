#ifndef HACKTICAL_PARSE_H
#define HACKTICAL_PARSE_H

#include "list/list.h"

struct hc_parser {
  struct hc_list parent;
  
  bool (*parse)(struct hc_parser *p,
		const char *in,
		size_t *i,
		struct hc_list *out);

  void (*free)(struct hc_parser *);
};

struct hc_parsed {
  struct hc_list parent;
  struct hc_list children;
  int id;
  size_t start;
  size_t end;
};

struct hc_parser *hc_parse_ws();
struct hc_parser *hc_parse_if(int id, bool (*predicate)(char));

struct hc_parser *hc_parse_alpha(int id);
struct hc_parser *hc_parse_digit(int id);

#define hc_parse_any(...)					\
  _hc_parse_any((struct hc_parser *[]){__VA_ARGS__, NULL})

struct hc_parser *_hc_parse_any(struct hc_parser *alts[]);

size_t hc_parse(struct hc_parser *p,
		const char *in,
		struct hc_list *out);

void hc_parser_free(struct hc_parser *p);
void hc_parsed_free(struct hc_list *prs);

#endif
