#ifndef HACKTICAL_PARSE_H
#define HACKTICAL_PARSE_H

#include "list/list.h"

struct hc_parser {
  struct hc_list parent;
  
  struct hc_parser *(*parse)(struct hc_parser *p,
			     struct hc_parser *pn,
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

struct hc_parser *hc_parse_any();
struct hc_parser *hc_parse_space();
struct hc_parser *hc_parse_char(int id, char ch);

struct hc_parser *hc_parse_if(int id, bool (*predicate)(char));
struct hc_parser *hc_parse_alpha(int id);
struct hc_parser *hc_parse_digit(int id);

#define hc_parse_or(...)					\
  _hc_parse_or((struct hc_parser *[]){__VA_ARGS__, NULL})

struct hc_parse_and {
  struct hc_parser parser;
  int id;
  struct hc_list parts;
};

struct hc_parser *_hc_parse_or(struct hc_parser *alts[]);

#define hc_parse_and(id, ...)					\
  _hc_parse_and(id, (struct hc_parser *[]){__VA_ARGS__, NULL})

struct hc_parser *_hc_parse_and(int id, struct hc_parser *parts[]);

struct hc_parser *hc_parse_many(int id, struct hc_parser *part);

size_t hc_parse(struct hc_parser *p,
		const char *in,
		struct hc_list *out);

void hc_parser_free(struct hc_parser *p);
void hc_parsed_free(struct hc_list *prs);

#endif
