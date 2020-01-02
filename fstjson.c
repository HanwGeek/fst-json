/*
 * @Author: HanwGeek
 * @Github: https://github.com/HanwGeek
 * @Description: FstJson module
 * @Date: 2020-01-01 21:45:56
 * @Last Modified: 2020-01-02 13:57:04
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

/* null == "null" */
static int fst_parse_null(fst_context* c, fst_value* v) {
  EXPECT(c, 'n');
  if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
    return FST_PARSE_INVALID_VALUE;
  c->json += 3;
  v->type = FST_NULL;
  return FST_PARSE_OK;
}

/* true == "true" */
static int fst_parse_true(fst_context* c, fst_value* v) {
  EXPECT(c, 't');
  if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
    return FST_PARSE_INVALID_VALUE;
  c->json += 3;
  v->type = FST_TRUE;
  return FST_PARSE_OK;
}

/* false == "false" */
static int fst_parse_false(fst_context* c, fst_value* v) {
  EXPECT(c, 'f');
  if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
    return FST_PARSE_INVALID_VALUE;
  c->json += 4;
  v->type = FST_FALSE;
  return FST_PARSE_OK;
}

static int fst_parse_value(fst_context* c, fst_value* v) {
  switch (*c->json) {
    case 'n': return fst_parse_null(c, v);
    case 't': return fst_parse_true(c, v);
    case 'f': return fst_parse_false(c, v);
    case '\0': return FST_PARSE_EXPECT_VALUE;
    default: return FST_PARSE_INVALID_VALUE;
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
    if (*c.json != '\0') ret = FST_PARSE_ROOT_NOT_SINGULAR;
  }
  return ret;
}

fst_type fst_get_type(const fst_value* v) {
  assert(v != NULL);
  return v->type;
}