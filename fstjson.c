/*
 * @Author: HanwGeek
 * @Github: https://github.com/HanwGeek
 * @Description: FstJson module
 * @Date: 2020-01-01 21:45:56
 * @Last Modified: 2020-01-03 21:31:23
 */
#include "fstjson.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h> 

#ifndef FST_PARSE_STACK_INIT_SIZE
#define FST_PARSE_STACK_INIT_SIZE 256
#endif

#define EXPECT(c, ch) do {assert(*c->json == (ch)); c->json++;} while(0)
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch) do {*(char*)fst_context_push(c, sizeof(char)) = (ch);} while(0)
#define STRING_ERR(ret) do {c->top = head; return ret;} while(0)

typedef struct {
  const char* json;
  char* stack;
  size_t size, top;
}fst_context;

/* Allocate `size` memory to stack, return top of stack */
static void* fst_context_push(fst_context* c, size_t size) {
  assert(size > 0);
  if (c->top + size >= c->size) {
    if (c->size == 0)
      c->size = FST_PARSE_STACK_INIT_SIZE;
    while (c->top + size >= c->size)
      c->size += c->size >> 1;
    c->stack = (char*)realloc(c->stack, c->size);
  }
  void* ret = c->stack + c->top;
  c->top += size;
  return ret;
}

/* Pop `size` elem, return top of stack after */
static void* fst_context_pop(fst_context* c, size_t size) {
  assert(c->top >= size);
  return c->stack + (c->top -= size);
}

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
  v->u.n = strtod(c->json, NULL);
  c->json = p;
  v->type = FST_NUMBER;
  return FST_PARSE_OK;
}

static const char* fst_parse_hex4(const char* p, unsigned* u) {
  *u = 0;
  for (int i = 0; i < 4; i++) {
    char ch = *p++;
    *u <<= 4;
    if (ch >= '0' && ch <= '9') *u |= ch - '0';
    else if (ch >= 'A' && ch <= 'F') *u |= ch - ('A' - 10);
    else if (ch >= 'a' && ch <= 'f') *u |= ch - ('a' - 10);
    else return NULL;
  }
  return p;
}

static void fst_encode_utf8(fst_context* c, unsigned u) {
  if (u <= 0x7F) 
    PUTC(c, u & 0xFF);
  else if (u <= 0x7FF) {
    PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
    PUTC(c, 0x80 | ( u       & 0x3F));
  }
  else if (u <= 0xFFFF) {
    PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
    PUTC(c, 0x80 | ((u >>  6) & 0x3F));
    PUTC(c, 0x80 | ( u        & 0x3F));
  }
  else {
    assert(u <= 0x10FFFF);
    PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
    PUTC(c, 0x80 | ((u >> 12) & 0x3F));
    PUTC(c, 0x80 | ((u >>  6) & 0x3F));
    PUTC(c, 0x80 | ( u        & 0x3F));
  }  
}

static int fst_parse_string_raw(fst_context* c, char** str, size_t* len) {
  size_t head = c->top;
  EXPECT(c, '\"');  
  const char* p = c->json;
  unsigned u, u2;
  for (;;) {
    char ch = *p++;
    switch (ch) {
      case '\"': 
        *len = c->top - head;
        *str = fst_context_pop(c, *len);
        c->json = p;
        return FST_PARSE_OK;
      case '\\': 
        switch (*p++) {
          case '\"': PUTC(c, '\"'); break;
          case '\\': PUTC(c, '\\'); break;
          case '/':  PUTC(c, '/' ); break;
          case 'b':  PUTC(c, '\b'); break;
          case 'f':  PUTC(c, '\f'); break;
          case 'n':  PUTC(c, '\n'); break;
          case 'r':  PUTC(c, '\r'); break;
          case 't':  PUTC(c, '\t'); break;
          case 'u':  
            if (!(p = fst_parse_hex4(p, &u)))
              STRING_ERR(FST_PARSE_INVALID_UNICODE_HEX);
            if (u >= 0xD800 && u <= 0xDBFF) {
              if (*p++ != '\\')
                STRING_ERR(FST_PARSE_INVALID_UNICODE_SURROGATE);
              if (*p++ != 'u')
                STRING_ERR(FST_PARSE_INVALID_UNICODE_SURROGATE);
              if (!(p = fst_parse_hex4(p, &u2)));
                STRING_ERR(FST_PARSE_INVALID_UNICODE_HEX);
              if (u2 < 0xDC00 || u2 > 0xDFFF)
                STRING_ERR(FST_PARSE_INVALID_UNICODE_SURROGATE);
              u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
            }
            fst_encode_utf8(c, u);
            break;           
          default:
            STRING_ERR(FST_PARSE_INVALID_STRING_ESCAPE);
        } break;
      case '\0': 
        STRING_ERR(FST_PARSE_MISS_QUOTATION_MARK);
      default: if ((unsigned char)ch < 0x20) 
                  STRING_ERR(FST_PARSE_INVALID_STRING_CHAR);            
                PUTC(c, ch);
    }
  }  
}

static int fst_parse_string(fst_context* c, fst_value* v) {
  int ret;
  char* s;
  size_t len;
  if ((ret = fst_parse_string_raw(c, &s, &len)) == FST_PARSE_OK)
    fst_set_string(v, s, len);
  return ret;
}

static int fst_parse_value(fst_context* c, fst_value* v);
static int fst_parse_array(fst_context* c, fst_value* v) {
  size_t size = 0;
  int ret;
  EXPECT(c, '[');
  fst_parse_whitespace(c);
  if (*c->json == ']') {
    c->json++;
    v->type = FST_ARRAY;
    v->u.a.size = 0;
    v->u.a.e = NULL;
    return FST_PARSE_OK;
  }
  for (;;) {
    fst_value t;
    fst_init(&t);
    if ((ret = fst_parse_value(c, &t)) != FST_PARSE_OK)
      break;
    memcpy(fst_context_push(c, sizeof(fst_value)), &t, sizeof(fst_value));
    size++;
    fst_parse_whitespace(c);
    if (*c->json == ',') {
      c->json++;
      fst_parse_whitespace(c);
    } else if (*c->json == ']') {
      c->json++;
      v->type = FST_ARRAY;
      v->u.a.size = size;
      size *= sizeof(fst_value);
      memcpy(v->u.a.e = (fst_value*)malloc(size), fst_context_pop(c, size), size);
      return FST_PARSE_OK;
    } else {
      ret = FST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
      break;
    }
  }
  for (size_t i = 0; i < size; i++)
    fst_free((fst_value*)fst_context_pop(c, sizeof(fst_value)));
  return ret;
}

static int fst_parse_object(fst_context* c, fst_value* v) {
  EXPECT(c, '{');
  fst_member m;
  size_t size;
  int ret;
  fst_parse_whitespace(c);
  if (*c->json == '}') {
    c->json++;
    v->type = FST_OBJ;
    v->u.o.m = NULL;
    v->u.o.size = 0;
    return FST_PARSE_OK;
  }

  m.k = NULL;
  size = 0;
  for (;;) {
    fst_init(&m.v);
    if (*c->json != '"') {
      ret = FST_PARSE_MISS_KEY;
      break;
    }
    char* str;
    if ((ret = fst_parse_string_raw(c, &str, &m.klen)) != FST_PARSE_OK)
      break;
    memcpy(m.k = (char*)malloc(m.klen + 1), str, m.klen);
    m.k[m.klen] = '\0';

    fst_parse_whitespace(c);
    if (*c->json != ':') {
      ret = FST_PARSE_MISS_COLON;
      break;
    }

    c->json++;
    fst_parse_whitespace(c);

    if ((ret = fst_parse_value(c, &m.v)) != FST_PARSE_OK)
      break;
    memcpy(fst_context_push(c, sizeof(fst_member)), &m, sizeof(fst_member));
    size++;
    m.k = NULL;

    fst_parse_whitespace(c);
    if (*c->json == ',') {
      c->json++;
      fst_parse_whitespace(c);
    } else if (*c->json == '}') {
      c->json++;
      v->u.o.size = size;
      size_t s = sizeof(fst_member) * size;
      memcpy(v->u.o.m = (fst_member*)malloc(s), fst_context_pop(c, s), s);
      v->type = FST_OBJ;
      return FST_PARSE_OK;
    } else {
      ret = FST_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
      break;
    }
  }
  free(m.k);
  for (size_t i = 0; i < size; i++) {
    fst_member* m = (fst_member*)fst_context_pop(c, sizeof(fst_member));
    free(m->k);
    fst_free(&m->v);
  }
  v->type = FST_NULL;
  return ret;
}

static int fst_parse_value(fst_context* c, fst_value* v) {
  switch (*c->json) {
    case 'n': return fst_parse_literal(c, v, "null", FST_NULL);
    case 't': return fst_parse_literal(c, v, "true", FST_TRUE);
    case 'f': return fst_parse_literal(c, v, "false", FST_FALSE);
    case '\"': return fst_parse_string(c, v);
    case '[': return fst_parse_array(c, v);
    case '{': return fst_parse_object(c, v);
    default: return fst_parse_number(c, v);
    case '\0': return FST_PARSE_EXPECT_VALUE;
  }
}

void fst_free(fst_value* v) {
  assert(v != NULL);
  switch (v->type) {
    case FST_STRING: free(v->u.s.s); break;
    case FST_ARRAY:
      for (size_t i = 0; i < v->u.a.size; i++)
        fst_free(&v->u.a.e[i]);
      free(v->u.a.e);
      break;
    case FST_OBJ:
      for (size_t i = 0; i < v->u.o.size; i++) {
        free(v->u.o.m[i].k);
        fst_free(&v->u.o.m[i].v);
      }
      free(v->u.o.m);
      break;
    default: break;
  }
  v->type = FST_NULL;
}

int fst_get_boolean(const fst_value* v) {
  assert(v != NULL && (v->type == FST_TRUE || v->type == FST_FALSE));
  return v->type == FST_TRUE;
}

void fst_set_boolean(fst_value* v, int b) {
  fst_free(v);
  v->type = b ? FST_TRUE : FST_FALSE;
}

double fst_get_number(const fst_value* v) {
  assert(v != NULL && v->type == FST_NUMBER);
  return v->u.n;
}

void fst_set_number(fst_value* v, double n) {
  fst_free(v);
  v->type = FST_NUMBER;
  v->u.n = n;
}

const char* fst_get_string(const fst_value* v) {
  assert(v != NULL && v->type == FST_STRING);
  return v->u.s.s;
}

size_t fst_get_string_len(const fst_value* v) {
  assert(v != NULL && v->type == FST_STRING);
  return v->u.s.len;
}

void fst_set_string(fst_value* v, const char* s, size_t len) {
  assert(v != NULL && (s != NULL || len == 0));
  fst_free(v);
  v->u.s.s = (char*)malloc(len + 1);
  v->u.s.s[len] = '\0';
  memcpy(v->u.s.s, s, len);
  v->u.s.len = len;
  v->type = FST_STRING;
}

size_t fst_get_array_size(const fst_value* v) {
  assert (v != NULL && v->type == FST_ARRAY);
  return v->u.a.size;
}

fst_value* fst_get_array_elem(const fst_value* v, size_t index) {
  assert(v != NULL && v->type == FST_ARRAY);
  assert(index < v->u.a.size);
  return &v->u.a.e[index];
}

int fst_parse(fst_value* v, const char* json) {
  fst_context c;
  assert(v != NULL);
  c.json = json;
  c.stack = NULL;
  c.size = c.top = 0;
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
  assert(c.top == 0);
  free(c.stack);
  return ret;
}

fst_type fst_get_type(const fst_value* v) {
  assert(v != NULL);
  return v->type;
}