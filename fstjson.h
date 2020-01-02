/*
 * @Author: HanwGeek
 * @Github: https://github.com/HanwGeek
 * @Description: The header file for fstjson.c
 * @Date: 2020-01-01 21:27:19
 * @Last Modified: 2020-01-02 21:08:44
 */
#ifndef FSTJSON_H_
#define FSTJSON_H_

#include <stddef.h>

/* JSON file format */
typedef enum {
  FST_NULL, 
  FST_FALSE, 
  FST_TRUE, 
  FST_NUMBER,
  FST_STRING,
  FST_ARRAY,
  FST_OBJ} fst_type;

typedef struct {
  union {
    struct {char* s; size_t len;} s;
    double n;
  } u;
  fst_type type;
} fst_value;

enum {
  FST_PARSE_OK = 0,
  FST_PARSE_EXPECT_VALUE,
  FST_PARSE_INVALID_VALUE,
  FST_PARSE_ROOT_NOT_SINGULAR,
  FST_PARSE_NUMBER_TOO_BIG,
  FST_PARSE_MISS_QUOTATION_MARK,
  FST_PARSE_INVALID_STRING_ESCAPE,
  FST_PARSE_INVALID_STRING_CHAR
};

#define fst_init(v) do {(v)->type = FST_NULL;} while(0)

void fst_free(fst_value* v);

#define fst_set_null(v) fst_free(v)

int fst_parse(fst_value* v, const char* json);

fst_type fst_get_type(const fst_value* v);

int fst_get_boolean(const fst_value* v);
void fst_set_boolean(fst_value* v, int b);

double fst_get_number(const fst_value* v);
void fst_set_number(fst_value* v, double n);

const char* fst_get_string(const fst_value* v);
size_t fst_get_string_len(const fst_value* v);
void fst_set_string(fst_value* v, const char* s, size_t len);

#endif