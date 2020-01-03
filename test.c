/*
 * @Author: HanwGeek
 * @Github: https://github.com/HanwGeek
 * @Description: Test module
 * @Date: 2020-01-01 21:32:35
 * @Last Modified: 2020-01-03 20:00:58
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
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, length) EXPECT_EQ_BASE((sizeof(expect) - 1 == length && memcmp(expect, actual, length) == 0), expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")
#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

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

#define TEST_NUMBER(expect, json)\
  do {\
    fst_value v;\
    EXPECT_EQ_INT(FST_PARSE_OK, fst_parse(&v, json));\
    EXPECT_EQ_INT(FST_NUMBER, fst_get_type(&v));\
    EXPECT_EQ_DOUBLE(expect, fst_get_number(&v));\
  } while(0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json) \
  do {\
    fst_value v;\
    fst_init(&v);\
    EXPECT_EQ_INT(FST_PARSE_OK, fst_parse(&v, json));\
    EXPECT_EQ_INT(FST_STRING, fst_get_type(&v));\
    EXPECT_EQ_STRING(expect, fst_get_string(&v), fst_get_string_len(&v));\
    fst_free(&v);\
  } while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

#define TEST_ERROR(err, json) \
  do {\
    fst_value v;\
    v.type = FST_FALSE;\
    EXPECT_EQ_INT(err, fst_parse(&v, json));\
    EXPECT_EQ_INT(FST_NULL, fst_get_type(&v));\
  } while(0)

static void test_parse_expect_value() {
  TEST_ERROR(FST_PARSE_EXPECT_VALUE, "");
  TEST_ERROR(FST_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
  TEST_ERROR(FST_PARSE_INVALID_VALUE, "nul");
  TEST_ERROR(FST_PARSE_INVALID_VALUE, "$");

  /* Invalid number cases */
  TEST_ERROR(FST_PARSE_INVALID_VALUE, "+0");
  TEST_ERROR(FST_PARSE_INVALID_VALUE, "+1");
  TEST_ERROR(FST_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
  TEST_ERROR(FST_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
  TEST_ERROR(FST_PARSE_INVALID_VALUE, "INF");
  TEST_ERROR(FST_PARSE_INVALID_VALUE, "inf");
  TEST_ERROR(FST_PARSE_INVALID_VALUE, "NAN");
  TEST_ERROR(FST_PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_singular() {
  TEST_ERROR(FST_PARSE_ROOT_NOT_SINGULAR, "null x");

  /* Invalid number cases */
  TEST_ERROR(FST_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
  TEST_ERROR(FST_PARSE_ROOT_NOT_SINGULAR, "0x0");
  TEST_ERROR(FST_PARSE_ROOT_NOT_SINGULAR, "0x123"); 
}

static void test_parse_missing_quotation_mark() {
    TEST_ERROR(FST_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(FST_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(FST_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(FST_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(FST_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(FST_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(FST_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(FST_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_array() {
  fst_value v;
  
  fst_init(&v);
  EXPECT_EQ_INT(FST_PARSE_OK, fst_parse(&v, "[ ]"));
  EXPECT_EQ_INT(FST_ARRAY, fst_get_type(&v));
  EXPECT_EQ_SIZE_T(0, fst_get_array_size(&v));
  fst_free(&v);

  fst_init(&v);
  EXPECT_EQ_INT(FST_PARSE_OK, fst_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
  EXPECT_EQ_INT(FST_ARRAY, fst_get_type(&v));
  EXPECT_EQ_SIZE_T(5, fst_get_array_size(&v));
  EXPECT_EQ_INT(FST_NULL,   fst_get_type(fst_get_array_elem(&v, 0)));
  EXPECT_EQ_INT(FST_FALSE,  fst_get_type(fst_get_array_elem(&v, 1)));
  EXPECT_EQ_INT(FST_TRUE,   fst_get_type(fst_get_array_elem(&v, 2)));
  EXPECT_EQ_INT(FST_NUMBER, fst_get_type(fst_get_array_elem(&v, 3)));
  EXPECT_EQ_INT(FST_STRING, fst_get_type(fst_get_array_elem(&v, 4)));
  EXPECT_EQ_DOUBLE(123.0, fst_get_number(fst_get_array_elem(&v, 3)));
  EXPECT_EQ_STRING("abc", fst_get_string(fst_get_array_elem(&v, 4)), fst_get_string_len(fst_get_array_elem(&v, 4)));
  fst_free(&v);

  fst_init(&v);
  EXPECT_EQ_INT(FST_PARSE_OK, fst_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
  EXPECT_EQ_INT(FST_ARRAY, fst_get_type(&v));
  EXPECT_EQ_SIZE_T(4, fst_get_array_size(&v));
  for (size_t i = 0; i < 4; i++) {
    fst_value* a = fst_get_array_elem(&v, i);
    EXPECT_EQ_INT(FST_ARRAY, fst_get_type(a));
    EXPECT_EQ_SIZE_T(i, fst_get_array_size(a));
    for (size_t j = 0; j < i; j++) {
      fst_value* e = fst_get_array_elem(a, j);
      EXPECT_EQ_INT(FST_NUMBER, fst_get_type(e));
      EXPECT_EQ_DOUBLE((double)j, fst_get_number(e));
    }
  }
  fst_free(&v);  
}

static void test_parse_miss_key() {
    TEST_ERROR(FST_PARSE_MISS_KEY, "{:1,");
    TEST_ERROR(FST_PARSE_MISS_KEY, "{1:1,");
    TEST_ERROR(FST_PARSE_MISS_KEY, "{true:1,");
    TEST_ERROR(FST_PARSE_MISS_KEY, "{false:1,");
    TEST_ERROR(FST_PARSE_MISS_KEY, "{null:1,");
    TEST_ERROR(FST_PARSE_MISS_KEY, "{[]:1,");
    TEST_ERROR(FST_PARSE_MISS_KEY, "{{}:1,");
    TEST_ERROR(FST_PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
    TEST_ERROR(FST_PARSE_MISS_COLON, "{\"a\"}");
    TEST_ERROR(FST_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
    TEST_ERROR(FST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(FST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(FST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(FST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_access_null() {
  fst_value v;
  fst_init(&v);
  fst_set_string(&v, "a", 1);
  fst_set_null(&v);
  EXPECT_EQ_INT(FST_NULL, fst_get_type(&v));
  fst_free(&v);
}

static void test_access_boolean() {
    fst_value v;
    fst_init(&v);
    fst_set_string(&v, "a", 1);
    fst_set_boolean(&v, 1);
    EXPECT_TRUE(fst_get_boolean(&v));
    fst_set_boolean(&v, 0);
    EXPECT_FALSE(fst_get_boolean(&v));
    fst_free(&v);
}

static void test_access_number() {
    fst_value v;
    fst_init(&v);
    fst_set_string(&v, "a", 1);
    fst_set_number(&v, 1234.5);
    EXPECT_EQ_DOUBLE(1234.5, fst_get_number(&v));
    fst_free(&v);
}

static void test_access_string() {
  fst_value v;
  fst_init(&v);
  fst_set_string(&v, "", 0);
  EXPECT_EQ_STRING("", fst_get_string(&v), fst_get_string_len(&v));
  fst_set_string(&v, "Hello", 5);
  EXPECT_EQ_STRING("Hello", fst_get_string(&v), fst_get_string_len(&v));
  fst_free(&v);
}

static void test_parse() {
  test_parse_null();
  test_parse_true();
  test_parse_false();
  test_parse_number();
  test_parse_string();
  test_parse_array();
  test_parse_expect_value();
  test_parse_invalid_value();
  test_parse_root_not_singular();
  test_parse_missing_quotation_mark();
  test_parse_invalid_string_escape();
  test_parse_invalid_string_char();
  test_parse_miss_key();
  test_parse_miss_colon();
  test_parse_miss_comma_or_curly_bracket();

  test_access_null();
  test_access_boolean();
  test_access_number();
  test_access_string();
}

int main() {
  test_parse();
  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
  return main_ret;
}