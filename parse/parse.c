#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "parse.h"

static struct hc_parser *parse_once(struct hc_parser *p,
				    struct hc_parser *pn,
				    const char *in,
				    size_t *i,
				    struct hc_list *out) {
  assert(p->parse);
  return p->parse(p, pn, in, i, out);
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

static struct hc_parser *any_parse(struct hc_parser *_p,
				     struct hc_parser *pn,
				     const char *in,
				     size_t *i,
				     struct hc_list *out) {
  if (*(in + *i)) {
    (*i)++;
    return _p;
  }

  return NULL;
}

static void any_free(struct hc_parser *p) {
  free(p);
}

struct hc_parser *hc_parse_any() {
  struct hc_parser *p = malloc(sizeof(struct hc_parser));

  *p = (struct hc_parser) {
    .parse = any_parse,
    .free = any_free
  };

  return p;
}

struct hc_parse_space {
  struct hc_parser parser;
  int id;
};

static struct hc_parser *space_parse(struct hc_parser *_p,
				     struct hc_parser *pn,
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
  
  return _p;
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

static struct hc_parser *char_parse(struct hc_parser *_p,
				    struct hc_parser *pn,
				    const char *in,
				    size_t *i,
				    struct hc_list *out) {
  struct hc_parse_char *p = hc_baseof(_p, struct hc_parse_char, parser);
  const char c = *(in + *i);
  
  if ((p->ch >= 0 && c == p->ch) || (p->ch < 0 && c != -p->ch)) {
    if (p->id) {
      push_result(out, p->id, *i, *i+1);
    }

    (*i)++;
    return _p;
  }

  return NULL;
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

static struct hc_parser *if_parse(struct hc_parser *_p,
				  struct hc_parser *pn,
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
    return _p;
  }

  return NULL;
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

static struct hc_parser *or_parse(struct hc_parser *_p,
				  struct hc_parser *pn,
				  const char *in,
				  size_t *i,
				  struct hc_list *out) {
  struct hc_parse_or *p = hc_baseof(_p, struct hc_parse_or, parser);

  hc_list_do(&p->parts, _pp) {
    struct hc_parser *pp = hc_baseof(_pp, struct hc_parser, parent);
    
    if (parse_once(pp, NULL, in, i, out)) {
      return pp;
    }
  }

  return NULL;
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
  int id;
  struct hc_list parts;
};

static struct hc_parser *and_parse(struct hc_parser *_p,
				   struct hc_parser *pn,
				   const char *in,
				   size_t *i,
				   struct hc_list *out) {
  struct hc_parse_and *p = hc_baseof(_p, struct hc_parse_and, parser);

  size_t start = *i;
  struct hc_list ps;
  hc_list_init(&ps);
  
  for (struct hc_list *_pp = p->parts.next;
       _pp != &p->parts;
       _pp = _pp->next) {
    struct hc_parser *pp = hc_baseof(_pp, struct hc_parser, parent);
    
    struct hc_parser *pn = (_pp->next == &p->parts)
      ? NULL
      : hc_baseof(_pp->next, struct hc_parser, parent);

    struct hc_parser *pr = parse_once(pp, pn, in, i, &ps);

    if (!pr) {
      *i = start;
      return NULL;
    }

    if (pr == pn) {
      _pp = _pp->next;
    }
  }

  if (p->id) {
    out = &push_result(out, p->id, start, *i)->children;
  }
  
  hc_list_do(&ps, pp) {
    hc_list_push_back(out, pp);
  }

  return _p;
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

  p->id = id;
  hc_list_init(&p->parts);

  for (struct hc_parser **pp = parts; *pp; pp++) {
    hc_list_push_back(&p->parts, &(*pp)->parent);
  }

  return &p->parser;
}								   

struct hc_parse_many {
  struct hc_parser parser;
  int id;
  struct hc_parser *part;
};

static struct hc_parser *many_parse(struct hc_parser *_p,
				    struct hc_parser *pn,
				    const char *in,
				    size_t *i,
				    struct hc_list *out) {
  struct hc_parse_many *p = hc_baseof(_p, struct hc_parse_many, parser);

  struct hc_list ps;
  hc_list_init(&ps);
  size_t start = *i;
  size_t end = start;
  struct hc_parser *pr = _p;

  while (parse_once(p->part, NULL, in, i, &ps)) {
    end = *i;
    struct hc_list *po = out->prev;

    if (pn && parse_once(pn, NULL, in, i, out)) {
      pr = pn;
      out = po;
      break;
    }
  }

  if (p->id && start != end) {
    out = &push_result(out, p->id, start, end)->children;
  }

  hc_list_do(&ps, pp) {
    hc_list_push_back(out, pp);
  }

  return (start == end) ? NULL : pr;
}

static void many_free(struct hc_parser *_p) {
  struct hc_parse_many *p = hc_baseof(_p, struct hc_parse_many, parser);
  hc_parser_free(p->part);
  free(p);
}

struct hc_parser *hc_parse_many(int id, struct hc_parser *part) {
  struct hc_parse_many *p = malloc(sizeof (struct hc_parse_many));

  p->parser = (struct hc_parser){
    .parse = many_parse,
    .free = many_free
  };

  p->id = id;
  p->part = part;
  return &p->parser;
}

size_t hc_parse(struct hc_parser *p,
		const char *in,
		struct hc_list *out) {
  size_t i = 0;
  while (parse_once(p, NULL, in, &i, out));
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
