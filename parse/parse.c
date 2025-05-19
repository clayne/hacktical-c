#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "parse.h"

static struct hc_parsed *push_result(struct hc_list *parent,
				     int id,
				     size_t start,
				     size_t end) {
      struct hc_parsed *r = malloc(sizeof(struct hc_parsed));
      hc_list_push_back(parent, &r->parent);
      hc_list_init(&r->children);
      r->id = id;
      r->start = start;
      r->end = end;
      return r;
}

static bool if_parse(struct hc_parser *_p,
		     const char *in,
		     size_t *i,
		     struct hc_list *out) {
  struct hc_parse_if *p = hc_baseof(_p, struct hc_parse_if, parser);
  char c = *(in + *i);
  
  if (c && p->predicate(c)) {
    if (p->id) {
      push_result(out, p->id, *i, *i+1);
    }
    
    (*i)++;
    return true;
  }

  return false;
}

static void if_free(struct hc_parser *p) {
  free(hc_baseof(p, struct hc_parse_if, parser));
}

struct hc_parser *hc_parse_if(int id, bool (*predicate)(char)) {
  struct hc_parse_if *p = malloc(sizeof(struct hc_parse_if));

  p->parser = (struct hc_parser){
    .parse = if_parse,
    .free = if_free
  };

  p->id = id;
  p->predicate = predicate;
  return &p->parser;
}

static bool alpha_p(char c) {
  return isalpha(c);
}

struct hc_parser *hc_parse_alpha(int id) {
  return hc_parse_if(id, alpha_p);
}

static bool digit_p(char c) {
  return isdigit(c);
}

struct hc_parser *hc_parse_digit(int id) {
  return hc_parse_if(id, digit_p);
}

static bool any_parse(struct hc_parser *_p,
		     const char *in,
		     size_t *i,
		     struct hc_list *out) {
  struct hc_parse_any *p = hc_baseof(_p, struct hc_parse_any, parser);

  hc_list_do(&p->alts, a) {
    if (_hc_parse(hc_baseof(a, struct hc_parser, parent), in, i, out)) {
      return true;
    }
  }

  return false;
}

static void any_free(struct hc_parser *_p) {
  struct hc_parse_any *p = hc_baseof(_p, struct hc_parse_any, parser);

  hc_list_do(&p->alts, a) {
    hc_parser_free(hc_baseof(a, struct hc_parser, parent));
  }
  
  free(p);
}

struct hc_parser *_hc_parse_any(struct hc_parser *alts[]) {
  struct hc_parse_any *p = malloc(sizeof(struct hc_parse_any));

  p->parser = (struct hc_parser){
    .parse = any_parse,
    .free = any_free
  };

  hc_list_init(&p->alts);

  for (struct hc_parser **a = alts; *a; a++) {
    hc_list_push_back(&p->alts, &(*a)->parent);
  }

  return &p->parser;
  
}								   

size_t hc_parse(struct hc_parser *p,
		const char *in,
		struct hc_list *out) {
  size_t i = 0;
  while (_hc_parse(p, in, &i, out));
  return i;
}

bool _hc_parse(struct hc_parser *p,
	       const char *in,
	       size_t *i,
	       struct hc_list *out) {
  assert(p->parse);
  return p->parse(p, in, i, out);
}

void hc_parser_free(struct hc_parser *p) {
  assert(p->free);
  p->free(p);
}

void hc_parsed_free(struct hc_list *prs) {
  hc_list_do(prs, pr) {
    free(hc_baseof(pr, struct hc_parsed, parent));
  }
}
