#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "parse.h"

static bool if_parse(struct hc_parser *_p,
		     const char *in,
		     size_t *i,
		     struct hc_list *out) {
  struct hc_parse_if *p = hc_baseof(_p, struct hc_parse_if, parser);
  char c = *(in + *i);
  
  if (c && p->predicate(c)) {
    if (_p->id) {
      struct hc_parsed *pr = malloc(sizeof(struct hc_parsed));
      hc_list_push_back(out, &pr->parent);
      hc_list_init(&pr->children);
      pr->id = _p->id;
      pr->start = *i;
      pr->end = *i+1;
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
    .id = id,
    .parse = if_parse,
    .free = if_free
  };

  p->predicate = predicate;
  return &p->parser;
}

static bool alpha_p(char c) {
  return isalpha(c);
}

struct hc_parser *hc_parse_alpha(int id) {
  return hc_parse_if(id, alpha_p);
}

size_t hc_parse(struct hc_parser *p,
	      const char *in,
	      struct hc_list *out) {
  assert(p->parse);
  size_t i = 0;
  while (p->parse(p, in, &i, out));
  return i;
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
