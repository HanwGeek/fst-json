/*
 * @Author: HanwGeek
 * @Github: https://github.com/HanwGeek
 * @Description: The header file for fstjson.c
 * @Date: 2020-01-01 21:27:19
 * @Last Modified: 2020-01-01 22:24:25
 */
#ifndef FSTJSON_H_
#define FSTJSON_H_

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
  fst_type type;
} fst_value;

enum {
  FST_PARSE_OK = 0,
  FST_PARSE_EXPECT_VALUE,
  FST_PARSE_INVALID_VALUE,
  FST_PARSE_ROOT_NOT_SINGULAR
};

int fst_parse(fst_value* v, const char* json);

fst_type fst_get_type(const fst_value* v);

#endif