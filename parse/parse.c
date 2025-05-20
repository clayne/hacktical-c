#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "parse.h"

static bool parse_once(struct hc_parser *p,
		       const char *in,
		       size_t *i,
		       struct hc_list *out) {
  assert(p->parse);
  return p->parse(p, in, i, out);
}

static struct hc_parsed *push_result(struct hc_list *parent,
				     const int id,
				     const size_t start,
				     const size_t end) {
  struct hc_parsed *r = malloc(sizeof(struct hc_parsed));
  hc_list_push_back(parent, &r->parent);
  hc_list_init(&r->children);
  r->id = id;
  r->start = start;
  r->end = end;
  return r;
}

struct hc_parse_space {
  struct hc_parser parser;
  int id;
};

static bool space_parse(struct hc_parser *_p,
			const char *in,
			size_t *i,
			struct hc_list *out) {
  struct hc_parse_space *p = hc_baseof(_p, struct hc_parse_space, parser);
  size_t start = *i;
  
  while (isspace(*(in + *i))) {
    (*i)++;
  }

  if (p->id && *i != start) {
    push_result(out, p->id, start, *i);
  }
  
  return true;
}

static void space_free(struct hc_parser *p) {
  free(p);
}

struct hc_parser *hc_parse_space(const int id) {
  struct hc_parse_space *p = malloc(sizeof(struct hc_parse_space));

  p->parser = (struct hc_parser) {
    .parse = space_parse,
    .free = space_free
  };

  p->id = id;
  
  return &p->parser;
}

struct hc_parse_char {
  struct hc_parser parser;
  int id;
  char ch;
};

static bool char_parse(struct hc_parser *_p,
		       const char *in,
		       size_t *i,
		       struct hc_list *out) {
  struct hc_parse_char *p = hc_baseof(_p, struct hc_parse_char, parser);

  if (*(in + *i) == p->ch) {
    if (p->id) {
      push_result(out, p->id, *i, *i+1);
    }

    (*i)++;
    return true;
  }

  return false;
}

static void char_free(struct hc_parser *p) {
  free(p);
}

struct hc_parser *hc_parse_char(const int id, const char ch) {
  struct hc_parse_char *p = malloc(sizeof(struct hc_parse_char));

  p->parser = (struct hc_parser){
    .parse = char_parse,
    .free = char_free
  };

  p->id = id;
  p->ch = ch;
  return &p->parser;
}

struct hc_parse_if {
  struct hc_parser parser;
  int id;
  bool (*predicate)(char);
};

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

struct hc_parser *hc_parse_if(const int id, bool (*predicate)(char)) {
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

struct hc_parser *hc_parse_alpha(const int id) {
  return hc_parse_if(id, alpha_p);
}

static bool digit_p(char c) {
  return isdigit(c);
}

struct hc_parser *hc_parse_digit(const int id) {
  return hc_parse_if(id, digit_p);
}

struct hc_parse_or {
  struct hc_parser parser;
  struct hc_list parts;
};

static bool or_parse(struct hc_parser *_p,
		      const char *in,
		      size_t *i,
		      struct hc_list *out) {
  struct hc_parse_or *p = hc_baseof(_p, struct hc_parse_or, parser);

  hc_list_do(&p->parts, pp) {
    if (parse_once(hc_baseof(pp, struct hc_parser, parent), in, i, out)) {
      return true;
    }
  }

  return false;
}

static void or_free(struct hc_parser *_p) {
  struct hc_parse_or *p = hc_baseof(_p, struct hc_parse_or, parser);

  hc_list_do(&p->parts, pp) {
    hc_parser_free(hc_baseof(pp, struct hc_parser, parent));
  }
  
  free(p);
}

struct hc_parser *_hc_parse_or(struct hc_parser *parts[]) {
  struct hc_parse_or *p = malloc(sizeof(struct hc_parse_or));

  p->parser = (struct hc_parser){
    .parse = or_parse,
    .free = or_free
  };

  hc_list_init(&p->parts);

  for (struct hc_parser **pp = parts; *pp; pp++) {
    hc_list_push_back(&p->parts, &(*pp)->parent);
  }

  return &p->parser;
  
}								   

struct hc_parse_and {
  struct hc_parser parser;
  struct hc_list parts;
};

static bool and_parse(struct hc_parser *_p,
		      const char *in,
		      size_t *i,
		      struct hc_list *out) {
  struct hc_parse_and *p = hc_baseof(_p, struct hc_parse_and, parser);

  size_t start = *i;
  struct hc_list ps;
  hc_list_init(&ps);
  
  hc_list_do(&p->parts, pp) {
    if (!parse_once(hc_baseof(pp, struct hc_parser, parent), in, i, &ps)) {
      *i = start;

      hc_list_do(&ps, pp) {
	hc_parser_free(hc_baseof(pp, struct hc_parser, parent));
      }
      
      return false;
    }
  }

  hc_list_do(&ps, pp) {
    hc_list_push_back(out, pp);
  }

  return true;
}

static void and_free(struct hc_parser *_p) {
  struct hc_parse_and *p = hc_baseof(_p, struct hc_parse_and, parser);

  hc_list_do(&p->parts, pp) {
    hc_parser_free(hc_baseof(pp, struct hc_parser, parent));
  }
  
  free(p);
}

struct hc_parser *_hc_parse_and(const int id, struct hc_parser *parts[]) {
  struct hc_parse_and *p = malloc(sizeof(struct hc_parse_and));

  p->parser = (struct hc_parser){
    .parse = and_parse,
    .free = and_free
  };

  hc_list_init(&p->parts);

  for (struct hc_parser **pp = parts; *pp; pp++) {
    hc_list_push_back(&p->parts, &(*pp)->parent);
  }

  return &p->parser;
}								   

struct hc_parse_many {
  struct hc_parser parser;
  struct hc_parser *part;
};

static bool many_parse(struct hc_parser *_p,
		       const char *in,
		       size_t *i,
		       struct hc_list *out) {
  struct hc_parse_many *p = hc_baseof(_p, struct hc_parse_many, parser);
  while (parse_once(p->part, in, i, out));
  return true;
}

static void many_free(struct hc_parser *_p) {
  struct hc_parse_many *p = hc_baseof(_p, struct hc_parse_many, parser);
  hc_parser_free(p->part);
  free(p);
}

struct hc_parser *hc_parse_many(struct hc_parser *part) {
  struct hc_parse_many *p = malloc(sizeof (struct hc_parse_many));

  p->parser = (struct hc_parser){
    .parse = many_parse,
    .free = many_free
  };

  p->part = part;
  return &p->parser;
}

size_t hc_parse(struct hc_parser *p,
		const char *in,
		struct hc_list *out) {
  size_t i = 0;
  while (parse_once(p, in, &i, out));
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
