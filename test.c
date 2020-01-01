/*
 * @Author: HanwGeek
 * @Github: https://github.com/HanwGeek
 * @Description: Test module
 * @Date: 2020-01-01 21:32:35
 * @Last Modified: 2020-01-01 22:18:57
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fstjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)\
  do {\
    test_count++;\
    if (equality)\
      test_pass++;\
    else {\
      fprintf(stderr, "%s:%d: expect " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
      main_ret = 1;\
    }\
  } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

static void test_parse_null() {
  fst_value v;
  v.type = FST_FALSE;
  EXPECT_EQ_INT(FST_PARSE_OK, fst_parse(&v, "null"));
  EXPECT_EQ_INT(FST_NULL, fst_get_type(&v));
}

static void test_parse_true() {
  fst_value v;
  v.type = FST_FALSE;
  EXPECT_EQ_INT(FST_PARSE_OK, fst_parse(&v, "true"));
  EXPECT_EQ_INT(FST_TRUE, fst_get_type(&v));
}

static void test_parse_false() {
  fst_value v;
  v.type = FST_TRUE;
  EXPECT_EQ_INT(FST_PARSE_OK, fst_parse(&v, "false"));
  EXPECT_EQ_INT(FST_FALSE, fst_get_type(&v));
}

static void test_parse_expect_value() {
  fst_value v;
  v.type = FST_FALSE;
  EXPECT_EQ_INT(FST_PARSE_EXPECT_VALUE, fst_parse(&v, ""));
  EXPECT_EQ_INT(FST_NULL, fst_get_type(&v));

  v.type = FST_FALSE;
  EXPECT_EQ_INT(FST_PARSE_EXPECT_VALUE, fst_parse(&v, " "));
  EXPECT_EQ_INT(FST_NULL, fst_get_type(&v));
}

static void test_parse_invalid_value() {
  fst_value v;
  v.type = FST_FALSE;
  EXPECT_EQ_INT(FST_PARSE_INVALID_VALUE, fst_parse(v, "tur"));
  EXPECT_EQ_INT(FST_NULL, fst_get_type(&v));

  v.type = FST_FALSE;
  EXPECT_EQ_INT(FST_PARSE_INVALID_VALUE, fst_parse(v, "'"));
  EXPECT_EQ_INT(FST_NULL, fst_get_type(&v));
}

static void test_parse_root_not_singular() {
  fst_type v;
  v.type = FST_FALSE;
  EXPECT_EQ_INT(FST_PARSE_ROOT_NOT_SINGULAR, fst_parse(v, "null x"));
  EXPECT_EQ_INT(FST_NULL, fst_get_type(&v));
}

static void test_parse() {
  test_parse_null();
  test_parse_true();
  test_parse_false();
  test_parse_expect_value();
  test_parse_invalid_value();
  test_parse_root_not_singular();
}

int main() {
  test_parse();
  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100 / test_count);
  return main_ret;
}