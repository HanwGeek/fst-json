/*
 * @Author: HanwGeek
 * @Github: https://github.com/HanwGeek
 * @Description: FstJson module
 * @Date: 2020-01-01 21:45:56
 * @Last Modified: 2020-01-02 15:04:56
 */
#include "fstjson.h"
#include <assert.h>
#include <stdlib.h>

#define EXPECT(c, ch) do{assert(*c->json == (ch)); c->json++;} while(0)

typedef struct {
  const char* json;
}fst_context;

/* ws = *(%x20 / %x09 / %x0A / %x0D) * */
static void fst_parse_whitespace(fst_context* c) {
  const char *p = c->json;
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
    p++;
  c->json = p;
}


/* null == "null" 
   true == "true" 
   false == "false" */
static int fst_parse_literal(fst_context* c, fst_value* v, const char* literal, fst_type type) {
  size_t i;
  EXPECT(c, literal[0]);
  for (i = 0; literal[i + 1]; i++)
    if (c->json[i] != literal[i + 1]) 
      return FST_PARSE_INVALID_VALUE;
  c->json += i;
  v->type = type;
  return FST_PARSE_OK;
}

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

static int fst_parse_number(fst_context* c, fst_value* v) {
  const char* p = c->json;
  if (*p == '-') p++;
  if (*p == '0') p++;
  else {
    if (!ISDIGIT1TO9(*p)) return FST_PARSE_INVALID_VALUE;
    for (p++; ISDIGIT(*p); p++);
  }
  if (*p == '.') {
    p++;
    if(!ISDIGIT(*p)) return FST_PARSE_INVALID_VALUE;
    for (p++; ISDIGIT(*p); p++);
  }
  if (*p == 'e' || *p == 'E') {
    p++;
    if (*p == '+' || *p == '-') p++;
    if (!ISDIGIT(*p)) return FST_PARSE_INVALID_VALUE;
    for (p++; ISDIGIT(*p); p++);
  }
  v->n = strtod(c->json, NULL);
  c->json = p;
  v->type = FST_NUMBER;
  return FST_PARSE_OK;
}

static int fst_parse_value(fst_context* c, fst_value* v) {
  switch (*c->json) {
    case 'n': return fst_parse_literal(c, v, "null", FST_NULL);
    case 't': return fst_parse_literal(c, v, "true", FST_TRUE);
    case 'f': return fst_parse_literal(c, v, "false", FST_FALSE);
    default: return fst_parse_number(c, v);
    case '\0': return FST_PARSE_EXPECT_VALUE;
  }
}

int fst_parse(fst_value* v, const char* json) {
  fst_context c;
  assert(v != NULL);
  c.json = json;
  v->type = FST_NULL;
  fst_parse_whitespace(&c);
  int ret =  fst_parse_value(&c, v);
  if (ret == FST_PARSE_OK) {
    fst_parse_whitespace(&c);
    if (*c.json != '\0') {
      v->type = FST_NULL;
      ret = FST_PARSE_ROOT_NOT_SINGULAR;
    }
  }
  return ret;
}

fst_type fst_get_type(const fst_value* v) {
  assert(v != NULL);
  return v->type;
}

double fst_get_number(const fst_value* v) {
  assert(v != NULL && v->type == FST_NUMBER);
  return v->n;
}