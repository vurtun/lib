/*
    Copyright (c) 2016
    vurtun <polygone@gmx.net>
    zlib license
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MM_JSON_STATIC
#define MM_JSON_IMPLEMENTATION
#include "../mm_json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define test_section(desc) \
    do { \
        printf("--------------- {%s} ---------------\n", desc);\
    } while (0);

#define test_assert(cond) \
    do { \
        int pass = cond; \
        printf("[%s] %s:%d: ", pass ? "PASS" : "FAIL", __FILE__, __LINE__);\
        printf((strlen(#cond) > 60 ? "%.47s...\n" : "%s\n"), #cond);\
        if (pass) pass_count++; else fail_count++; \
    } while (0)

#define test_token(t, content, typ, child, s)\
    {test_assert(!mm_json_cmp(t, content));\
    test_assert((t)->type == (typ));\
    test_assert((t)->children == (child));\
    test_assert((t)->sub == (s));}

#define test_result()\
    do { \
        printf("======================================================\n"); \
        printf("== Result:  %3d Total   %3d Passed      %3d Failed  ==\n", \
                pass_count  + fail_count, pass_count, fail_count); \
        printf("======================================================\n"); \
    } while (0)

static int run_test(void)
{
    int pass_count = 0;
    int fail_count = 0;


    test_section("str")
    {
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        mm_json_char buffer[8];

        const mm_json_char buf[] = "{\"name\":\"value\"}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "name"));
        test_assert(!mm_json_cmp(&pair.value, "value"));
        test_assert(pair.value.type == MM_JSON_STRING);
        test_assert(pair.value.children == 0);
        test_assert(pair.value.sub == 0);
        test_assert(mm_json_cpy(buffer, sizeof buffer, &pair.value) == 5);
        test_assert(!strcmp(&buffer[0], "value"));
    }

    test_section("num")
    {
        mm_json_number num = 0;
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] = "\n{\n\"test\":13\n}\n";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "test"));
        test_assert(!mm_json_cmp(&pair.value, "13"));
        test_assert(pair.value.type == MM_JSON_NUMBER);
        test_assert(mm_json_convert(&num, &pair.value) == MM_JSON_NUMBER);
        test_assert(num == 13.0);
    }

    test_section("negnum")
    {
        mm_json_number num = 0;
        struct mm_json_pair pair;
        struct mm_json_iter iter;
        const mm_json_char buf[] = "{\"name\":-1234}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "name"));
        test_assert(!mm_json_cmp(&pair.value, "-1234"));
        test_assert(pair.value.type == MM_JSON_NUMBER);
        test_assert(mm_json_convert(&num, &pair.value) == MM_JSON_NUMBER);
        test_assert(num == -1234.0);
    }

    test_section("fracnum")
    {
        mm_json_number num = 0;
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] = "{\"name\":1234.5678}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "name"));
        test_assert(!mm_json_cmp(&pair.value, "1234.5678"));
        test_assert(pair.value.type == MM_JSON_NUMBER);
        test_assert(mm_json_convert(&num, &pair.value) == MM_JSON_NUMBER);
        test_assert(num == 1234.5678);
    }

    test_section("negfracnum")
    {
        mm_json_number num = 0;
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] = "{\"name\":-1234.5678}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "name"));
        test_assert(!mm_json_cmp(&pair.value, "-1234.5678"));
        test_assert(pair.value.type == MM_JSON_NUMBER);
        test_assert(mm_json_convert(&num, &pair.value) == MM_JSON_NUMBER);
        test_assert(num == -1234.5678);
    }

    test_section("exponent")
    {
        mm_json_number num = 0;
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] = "{\"name\":2e+2}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "name"));
        test_assert(!mm_json_cmp(&pair.value, "2e+2"));
        test_assert(pair.value.type == MM_JSON_NUMBER);
        test_assert(mm_json_convert(&num, &pair.value) == MM_JSON_NUMBER);
        test_assert(num == 200.0);
    }

    test_section("negexponent")
    {
        mm_json_number num = 0;
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] = "{\"name\":-1234e-2}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "name"));
        test_assert(!mm_json_cmp(&pair.value, "-1234e-2"));
        test_assert(pair.value.type == MM_JSON_NUMBER);
        test_assert(mm_json_convert(&num, &pair.value) == MM_JSON_NUMBER);
        test_assert(num == -12.34);
    }

    test_section("smallexp")
    {
        mm_json_number num = 0;
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] = "{\"name\":2.567e-4}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "name"));
        test_assert(!mm_json_cmp(&pair.value, "2.567e-4"));
        test_assert(pair.value.type == MM_JSON_NUMBER);
        test_assert(mm_json_convert(&num, &pair.value) == MM_JSON_NUMBER);
        test_assert(num >= 0.0002567 && num <= 0.0002568);
    }

    test_section("utf8")
    {
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] = "{\"name\":\"$¢€𤪤\"}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "name"));
        test_assert(!mm_json_cmp(&pair.value, "$¢€𤪤"));
        test_assert(pair.value.type == MM_JSON_STRING);
    }

    test_section("map")
    {
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] = "{\"name\":\"test\", \"age\":42, \"utf8\":\"äöü\", \"alive\":true}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "name"));
        test_assert(!mm_json_cmp(&pair.value, "test"));
        test_assert(pair.value.type == MM_JSON_STRING);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "age"));
        test_assert(!mm_json_cmp(&pair.value, "42"));
        test_assert(pair.value.type == MM_JSON_NUMBER);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "utf8"));
        test_assert(!mm_json_cmp(&pair.value, "äöü"));
        test_assert(pair.value.type == MM_JSON_STRING);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "alive"));
        test_assert(!mm_json_cmp(&pair.value, "true"));
        test_assert(pair.value.type == MM_JSON_TRUE);
    }

    test_section("array_root")
    {
        int i = 1;
        mm_json_number num;
        struct mm_json_token tok;
        struct mm_json_iter iter;
        struct mm_json_pair pair;

        const mm_json_char buf[] = "[ 1.0, 2.0, 3.0, 4.0 ]";
        iter = mm_json_begin(buf, sizeof(buf));
        iter = mm_json_read(&tok, &iter);
        while (iter.src) {
            test_assert(mm_json_convert(&num, &tok) == MM_JSON_NUMBER);
            test_assert((mm_json_number)i == num);
            iter = mm_json_read(&tok, &iter);
            i++;
        }
    }

    test_section("array")
    {
        int i = 1;
        mm_json_number num;
        struct mm_json_token tok;
        struct mm_json_iter iter;
        struct mm_json_pair pair;

        const mm_json_char buf[] = "{\"list\":[ 1.0, 2.0, 3.0, 4.0 ]}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "list"));
        test_assert(!mm_json_cmp(&pair.value, "[ 1.0, 2.0, 3.0, 4.0 ]"));
        test_assert(pair.value.type == MM_JSON_ARRAY);
        test_assert(pair.value.children == 4);
        test_assert(pair.value.sub == 4);

        iter = mm_json_begin(pair.value.str, pair.value.len);
        iter = mm_json_read(&tok, &iter);
        while (iter.src) {
            test_assert(mm_json_convert(&num, &tok) == MM_JSON_NUMBER);
            test_assert((mm_json_number)i == num);
            iter = mm_json_read(&tok, &iter);
            i++;
        }
    }

    test_section("sub")
    {
        mm_json_number num = 0;
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] = "{\"sub\":{\"a\":1234.5678}}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "sub"));
        test_assert(!mm_json_cmp(&pair.value, "{\"a\":1234.5678}"));
        test_assert(pair.value.type == MM_JSON_OBJECT);
        test_assert(pair.value.children == 1);
        test_assert(pair.value.sub == 2);

        iter = mm_json_begin(pair.value.str, pair.value.len);
        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "a"));
        test_assert(!mm_json_cmp(&pair.value, "1234.5678"));
        test_assert(pair.value.type == MM_JSON_NUMBER);

        test_assert(mm_json_convert(&num, &pair.value) == MM_JSON_NUMBER);
        test_assert(num == 1234.5678);
    }

    test_section("subarray")
    {
        int i = 0;
        struct mm_json_token tok;
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char check[] = "1234";
        const mm_json_char buf[] = "{\"sub\":{\"a\":[1,2,3,4]}}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "sub"));
        test_assert(!mm_json_cmp(&pair.value, "{\"a\":[1,2,3,4]}"));
        test_assert(pair.value.type == MM_JSON_OBJECT);
        test_assert(pair.value.children == 1);
        test_assert(pair.value.sub == 6);

        iter = mm_json_begin(pair.value.str, pair.value.len);
        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "a"));
        test_assert(!mm_json_cmp(&pair.value, "[1,2,3,4]"));
        test_assert(pair.value.type == MM_JSON_ARRAY);
        test_assert(pair.value.children == 4);
        test_assert(pair.value.sub == 4);

        iter = mm_json_begin(pair.value.str, pair.value.len);
        iter = mm_json_read(&tok, &iter);
        while (iter.src) {
            test_assert(tok.str[0] == check[i++]);
            iter = mm_json_read(&tok, &iter);
        }
    }

    test_section("list")
    {
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] = "{\"sub\":{\"a\":\"b\"}, \"list\":{\"c\":\"d\"}}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "sub"));
        test_assert(!mm_json_cmp(&pair.value, "{\"a\":\"b\"}"));
        test_assert(pair.value.type == MM_JSON_OBJECT);
        test_assert(pair.value.children == 1);
        test_assert(pair.value.sub == 2);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "list"));
        test_assert(!mm_json_cmp(&pair.value, "{\"c\":\"d\"}"));
        test_assert(pair.value.type == MM_JSON_OBJECT);
        test_assert(pair.value.children == 1);
        test_assert(pair.value.sub == 2);
    }

    test_section("table")
    {
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] =
            "{\"sub\":{\"a\": \"b\"}, \"list\":[1,2,3,4], \"a\":true, \"b\": \"0a1b2\"}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "sub"));
        test_assert(!mm_json_cmp(&pair.value, "{\"a\": \"b\"}"));
        test_assert(pair.value.type == MM_JSON_OBJECT);
        test_assert(pair.value.children == 1);
        test_assert(pair.value.sub == 2);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "list"));
        test_assert(!mm_json_cmp(&pair.value, "[1,2,3,4]"));
        test_assert(pair.value.type == MM_JSON_ARRAY);
        test_assert(pair.value.children == 4);
        test_assert(pair.value.sub == 4);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "a"));
        test_assert(!mm_json_cmp(&pair.value, "true"));
        test_assert(pair.value.type == MM_JSON_TRUE);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "b"));
        test_assert(!mm_json_cmp(&pair.value, "0a1b2"));
        test_assert(pair.value.type == MM_JSON_STRING);
    }

    test_section("children")
    {
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        const mm_json_char buf[] = "{\"b\": {\"a\": {\"b\":5}, \"b\":[1,2,3,4],"
            "\"c\":\"test\", \"d\":true, \"e\":false, \"f\":null, \"g\":10},"
            "\"a\": [{\"b\":5}, [1,2,3,4], \"test\", true, false, null, 10]}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "b"));
        test_assert(pair.value.type == MM_JSON_OBJECT);
        test_assert(pair.value.children == 7);
        test_assert(pair.value.sub == 20);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "a"));
        test_assert(pair.value.type == MM_JSON_ARRAY);
        test_assert(pair.value.children == 7);
        test_assert(pair.value.sub == 13);
    }

    test_section("arrayofarray")
    {
        int i = 1;
        struct mm_json_iter iter;
        struct mm_json_pair pair;
        struct mm_json_token tok;
        const mm_json_char buf[] = "{\"coord\":[[[1,2], [3,4], [5,6]]]}";
        iter = mm_json_begin(buf, sizeof buf);

        iter = mm_json_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mm_json_cmp(&pair.name, "coord"));
        test_assert(pair.value.type == MM_JSON_ARRAY);
        test_assert(pair.value.children == 1);

        iter = mm_json_begin(pair.value.str, pair.value.len);
        iter = mm_json_read(&tok, &iter);
        test_assert(tok.type == MM_JSON_ARRAY);
        test_assert(tok.children == 3);

        iter = mm_json_begin(tok.str, tok.len);
        iter = mm_json_read(&tok, &iter);
        while (!iter.err && iter.src) {
            struct mm_json_iter it;
            test_assert(tok.type == MM_JSON_ARRAY);
            test_assert(tok.children == 2);
            it = mm_json_begin(tok.str, tok.len);
            it = mm_json_read(&tok, &it);
            while (!it.err && it.src) {
                mm_json_number n;
                test_assert(tok.type == MM_JSON_NUMBER);
                mm_json_convert(&n, &tok);
                test_assert(n == i++);
                it = mm_json_read(&tok, &it);
            }
            iter = mm_json_read(&tok, &iter);
        }
    }

    test_section("totalcount")
    {
        const mm_json_char buf[] =
            "{\"sub\":{\"a\": \"b\"}, \"list\":[1,2,3,4], \"a\":true, \"b\": \"0a1b2\"}";
        const mm_json_char buf2[] = "{\"coord\":[[[1,2], [3,4], [5,6]]]}";
        const mm_json_char buf3[] = "{\"list\":[ 1.0, 2.0, 3.0, 4.0 ]}";
        const mm_json_char buf4[] = "{\"name\":\"test\", \"age\":42, \"utf8\":\"äöü\", \"alive\":true}";
        test_assert(mm_json_num(buf, sizeof(buf)) == 14);
        test_assert(mm_json_num(buf2, sizeof(buf2)) == 12);
        test_assert(mm_json_num(buf3, sizeof(buf3)) == 6);
        test_assert(mm_json_num(buf4, sizeof(buf4)) == 8);
    }

    test_section("load")
    {
        mm_json_size count = 0;
        mm_json_size read = 0;
        enum mm_json_status status;
        struct mm_json_token toks[14];
        const mm_json_char buf[] =
            "{\"sub\":{\"a\": \"b\"}, \"list\":[1,2,3,4], \"a\":true, \"b\": \"0a1b2\"}";

        memset(toks, 0,  sizeof(toks));
        count = mm_json_num(buf, sizeof(buf));
        test_assert(count == 14);
        status = mm_json_load(toks, count, &read, buf, sizeof(buf));
        test_assert(status == MM_JSON_OK);
        test_token(&toks[0], "sub", MM_JSON_STRING, 0, 0);
        test_token(&toks[1], "{\"a\": \"b\"}", MM_JSON_OBJECT, 1, 2);
        test_token(&toks[2], "a", MM_JSON_STRING, 0, 0);
        test_token(&toks[3], "b", MM_JSON_STRING, 0, 0);
        test_token(&toks[4], "list", MM_JSON_STRING, 0, 0);
        test_token(&toks[5], "[1,2,3,4]", MM_JSON_ARRAY, 4, 4);
        test_token(&toks[6], "1", MM_JSON_NUMBER, 0, 0);
        test_token(&toks[7], "2", MM_JSON_NUMBER, 0, 0);
        test_token(&toks[8], "3", MM_JSON_NUMBER, 0, 0);
        test_token(&toks[9], "4", MM_JSON_NUMBER, 0, 0);
        test_token(&toks[10], "a", MM_JSON_STRING, 0, 0);
        test_token(&toks[11], "true", MM_JSON_TRUE, 0, 0);
        test_token(&toks[12], "b", MM_JSON_STRING, 0, 0);
        test_token(&toks[13], "0a1b2", MM_JSON_STRING, 0, 0);
        test_assert(read == 14);
    }

    test_section("load_array_root")
    {
        mm_json_size count = 0;
        mm_json_size read = 0;
        enum mm_json_status status;
        struct mm_json_token toks[14];
        const mm_json_char buf[] = "[ 1.0, 2.0, 3.0, 4.0 ]";

        memset(toks, 0,  sizeof(toks));
        count = mm_json_num(buf, sizeof(buf));
        status = mm_json_load(toks, count, &read, buf, sizeof(buf));
        test_assert(status == MM_JSON_OK);
        test_token(&toks[0], "1.0", MM_JSON_NUMBER, 0, 0);
        test_token(&toks[1], "2.0", MM_JSON_NUMBER, 0, 0);
        test_token(&toks[2], "3.0", MM_JSON_NUMBER, 0, 0);
        test_token(&toks[3], "4.0", MM_JSON_NUMBER, 0, 0);
        test_assert(read == 4);
    }

    test_section("load_array")
    {
        mm_json_size count = 0;
        mm_json_size read = 0;
        enum mm_json_status status;
        struct mm_json_token toks[14];
        const mm_json_char buf[] = "[\"Extra close\"]]";

        memset(toks, 0,  sizeof(toks));
        count = mm_json_num(buf, sizeof(buf));
        status = mm_json_load(toks, count, &read, buf, sizeof(buf));
        test_assert(status == MM_JSON_OK);
        test_token(&toks[0], "Extra close", MM_JSON_STRING, 0, 0);
        test_assert(read == 1);
    }


    test_section("empty")
    {
        mm_json_size count = 0;
        mm_json_size read = 0;
        enum mm_json_status status;
        struct mm_json_token toks[14];
        const mm_json_char buf[] =
            "{\"sub\":{\"a\": \"b\"}, \"list\":[1,2,3,4], \"a\":true, \"b\": \"0a1b2\"}";

        memset(toks, 0,  sizeof(toks));
        count = mm_json_num(buf, sizeof(buf));
        test_assert(count == 14);
        status = mm_json_load(toks, count, &read, buf, sizeof(buf));
        test_assert(status == MM_JSON_OK);
        test_token(&toks[0], "sub", MM_JSON_STRING, 0, 0);
        test_token(&toks[1], "{\"a\": \"b\"}", MM_JSON_OBJECT, 1, 2);
        test_token(&toks[2], "a", MM_JSON_STRING, 0, 0);
        test_token(&toks[3], "b", MM_JSON_STRING, 0, 0);
        test_token(&toks[4], "list", MM_JSON_STRING, 0, 0);
        test_token(&toks[5], "[1,2,3,4]", MM_JSON_ARRAY, 4, 4);
        test_token(&toks[6], "1", MM_JSON_NUMBER, 0, 0);
        test_token(&toks[7], "2", MM_JSON_NUMBER, 0, 0);
        test_token(&toks[8], "3", MM_JSON_NUMBER, 0, 0);
        test_token(&toks[9], "4", MM_JSON_NUMBER, 0, 0);
        test_token(&toks[10], "a", MM_JSON_STRING, 0, 0);
        test_token(&toks[11], "true", MM_JSON_TRUE, 0, 0);
        test_token(&toks[12], "b", MM_JSON_STRING, 0, 0);
        test_token(&toks[13], "0a1b2", MM_JSON_STRING, 0, 0);
        test_assert(read == 14);
    }

    test_section("query_simple")
    {
        mm_json_size count = 0;
        mm_json_size read = 0;
        enum mm_json_status status;
        struct mm_json_token toks[14];
        const mm_json_char buf[] =
            "{\"sub\":{\"a\": \"b\"}, \"list\":[1,2,3,4], \"a\":true, \"b\": \"0a1b2\"}";

        memset(toks, 0,  sizeof(toks));
        count = mm_json_num(buf, sizeof(buf));
        test_assert(count == 14);
        status = mm_json_load(toks, count, &read, buf, sizeof(buf));
        test_assert(read == 14);
        test_assert(status == MM_JSON_OK);
        test_assert(mm_json_query(toks, read, "list[0]") == &toks[6]);
        test_assert(mm_json_query(toks, read, "list[3]") == &toks[9]);
        test_assert(mm_json_query(toks, read, "sub.a") == &toks[3]);
        test_assert(mm_json_query(toks, read, "b") == &toks[13]);
    }

    test_section("query_complex")
    {
        mm_json_size read = 0;
        enum mm_json_status status;
        struct mm_json_token toks[128];
        const mm_json_char buf[] =
        "{\"map\":{"
            "\"entity\":["
                "{\"position\": {\"x\":1, \"y\":1}, \"size\":{\"w\":1,\"h\":1}},"
                "{\"position\": {\"x\":2, \"y\":2}, \"size\":{\"w\":2,\"h\":2}},"
                "{\"position\": {\"x\":3, \"y\":3}, \"size\":{\"w\":3,\"h\":3}},"
                "{\"position\": {\"x\":4, \"y\":4}, \"size\":{\"w\":4,\"h\":4}},"
                "{\"position\": {\"x\":5, \"y\":5}, \"size\":{\"w\":5,\"h\":5}}"
            "]"
        "}}";
        memset(toks, 0,  sizeof(toks));
        status = mm_json_load(toks, 128, &read, buf, sizeof(buf));
        test_assert(status == MM_JSON_OK);
        test_assert(mm_json_query(toks, read, "map") == &toks[1]);
        test_assert(mm_json_query(toks, read, "map.entity") == &toks[3]);
        test_assert(mm_json_query(toks, read, "map.entity[0]") == &toks[4]);
        test_assert(mm_json_query(toks, read, "map.entity[1]") == &toks[17]);
        test_assert(mm_json_query(toks, read, "map.entity[2]") == &toks[30]);
        test_assert(mm_json_query(toks, read, "map.entity[3]") == &toks[43]);
        test_assert(mm_json_query(toks, read, "map.entity[4]") == &toks[56]);
        test_assert(mm_json_query(toks, read, "map.entity[0].position.x") == &toks[8]);
        test_assert(mm_json_query(toks, read, "map.entity[2].position.y") == &toks[36]);
        test_assert(mm_json_query(toks, read, "map.entity[4].size.w") == &toks[66]);
        {
            struct mm_json_token *tok = mm_json_query(toks, read, "map.entity");
            test_assert(tok->children == 5);
        }
    }

    test_section("query_entangled")
    {
        mm_json_size read = 0;
        enum mm_json_status status;
        struct mm_json_token toks[128];
        const mm_json_char buf[] = "{\"b\": {\"a\": {\"b\":5}, \"b\":[1,2,3,4],"
            "\"c\":\"test\", \"d\":true, \"e\":false, \"f\":null, \"g\":10},"
            "\"a\": [{\"b\":5}, [1,2,3,4], \"test\", true, false, null, 10]}";

        memset(toks, 0,  sizeof(toks));
        status = mm_json_load(toks, 128 , &read, buf, sizeof(buf));
        test_assert(status == MM_JSON_OK);
        test_assert(mm_json_query(toks, read, "b") == &toks[1]);
        test_assert(mm_json_query(toks, read, "b.a") == &toks[3]);
        test_assert(mm_json_query(toks, read, "b.a.b") == &toks[5]);
        test_assert(mm_json_query(toks, read, "b.b") == &toks[7]);
        test_assert(mm_json_query(toks, read, "b.c") == &toks[13]);
        test_assert(mm_json_query(toks, read, "b.d") == &toks[15]);
        test_assert(mm_json_query(toks, read, "b.e") == &toks[17]);
        test_assert(mm_json_query(toks, read, "b.f") == &toks[19]);
        test_assert(mm_json_query(toks, read, "a[0]") == &toks[24]);
        test_assert(mm_json_query(toks, read, "a[0].b") == &toks[26]);
        test_assert(mm_json_query(toks, read, "a[1]") == &toks[27]);
        test_assert(mm_json_query(toks, read, "a[1][0]") == &toks[28]);
        test_assert(mm_json_query(toks, read, "a[1][1]") == &toks[29]);
        test_assert(mm_json_query(toks, read, "a[1][2]") == &toks[30]);
        test_assert(mm_json_query(toks, read, "a[1][3]") == &toks[31]);
        test_assert(mm_json_query(toks, read, "a[2]") == &toks[32]);
        test_assert(mm_json_query(toks, read, "a[3]") == &toks[33]);
        test_assert(mm_json_query(toks, read, "a[4]") == &toks[34]);
        test_assert(mm_json_query(toks, read, "a[5]") == &toks[35]);
        test_assert(mm_json_query(toks, read, "a[6]") == &toks[36]);
    }

    test_section("query_sub")
    {
        mm_json_size read = 0;
        enum mm_json_status status;
        struct mm_json_token toks[128];
        struct mm_json_token *entity;
        struct mm_json_token *position;
        struct mm_json_token *size;

        const mm_json_char buf[] =
        "{\"map\":{"
            "\"entity\":["
                "{\"position\": {\"x\":1, \"y\":1}, \"size\":{\"w\":1,\"h\":1}},"
                "{\"position\": {\"x\":2, \"y\":2}, \"size\":{\"w\":2,\"h\":2}},"
                "{\"position\": {\"x\":3, \"y\":3}, \"size\":{\"w\":3,\"h\":3}},"
                "{\"position\": {\"x\":4, \"y\":4}, \"size\":{\"w\":4,\"h\":4}},"
                "{\"position\": {\"x\":5, \"y\":5}, \"size\":{\"w\":5,\"h\":5}}"
            "]"
        "}}";

        memset(toks, 0,  sizeof(toks));
        status = mm_json_load(toks, 128, &read, buf, sizeof(buf));
        test_assert(status == MM_JSON_OK);
        entity = mm_json_query(toks, read, "map.entity[2]");
        test_assert(entity == &toks[30]);
        position = mm_json_query(entity, entity->sub, "position");
        test_assert(position == &toks[32]);
        size = mm_json_query(entity, entity->sub, "size");
        test_assert(size == &toks[38]);
        {
            struct mm_json_token *x, *y;
            x = mm_json_query(position, position->sub, "x");
            y = mm_json_query(position, position->sub, "y");
            test_assert(x == &toks[34]);
            test_assert(y == &toks[36]);
        }
        {
            struct mm_json_token *w, *h;
            w = mm_json_query(size, size->sub, "w");
            h = mm_json_query(size, size->sub, "h");
            test_assert(w == &toks[40]);
            test_assert(h == &toks[42]);
        }
    }

    test_section("query_number")
    {
        mm_json_size read = 0;
        enum mm_json_status status;
        struct mm_json_token toks[128];
        mm_json_number num;
        mm_json_int ret;
        mm_json_int i = 0;

        const mm_json_char buf[] =
        "{\"map\":{"
            "\"entity\":["
                "{\"position\": {\"x\":1, \"y\":1}, \"size\":{\"w\":1,\"h\":1}},"
                "{\"position\": {\"x\":2, \"y\":2}, \"size\":{\"w\":2,\"h\":2}},"
                "{\"position\": {\"x\":3, \"y\":3}, \"size\":{\"w\":3,\"h\":3}},"
                "{\"position\": {\"x\":4, \"y\":4}, \"size\":{\"w\":4,\"h\":4}},"
                "{\"position\": {\"x\":5, \"y\":5}, \"size\":{\"w\":5,\"h\":5}}"
            "]"
        "}}";

        memset(toks, 0,  sizeof(toks));
        status = mm_json_load(toks, 128, &read, buf, sizeof(buf));
        test_assert(status == MM_JSON_OK);
        for (i = 0; i < 5; ++i) {
            mm_json_char path[32];
            sprintf(path, "map.entity[%d].position.x", i);
            ret = mm_json_query_number(&num, toks, read, path);
            test_assert(ret == MM_JSON_NUMBER);
            test_assert(num == (mm_json_number)(i + 1));

            sprintf(path, "map.entity[%d].position.y", i);
            ret = mm_json_query_number(&num, toks, read, path);
            test_assert(ret == MM_JSON_NUMBER);
            test_assert(num == (mm_json_number)(i + 1));

            sprintf(path, "map.entity[%d].size.w", i);
            ret = mm_json_query_number(&num, toks, read, path);
            test_assert(ret == MM_JSON_NUMBER);
            test_assert(num == (mm_json_number)(i + 1));

            sprintf(path, "map.entity[%d].size.h", i);
            ret = mm_json_query_number(&num, toks, read, path);
            test_assert(ret == MM_JSON_NUMBER);
            test_assert(num == (mm_json_number)(i + 1));
        }
        test_assert(mm_json_query_number(&num, toks, read, "map.test") == MM_JSON_NONE);
        test_assert(mm_json_query_number(&num, toks, read, "map.entity") == MM_JSON_ARRAY);
    }

    test_section("query_string")
    {
        mm_json_size read = 0;
        enum mm_json_status status;
        struct mm_json_token toks[128];
        mm_json_char buffer[256];
        mm_json_size size;

        const mm_json_char buf[] = "{\"b\": {\"a\": {\"b\":5}, \"b\":[1,2,3,4],"
            "\"c\":\"test\", \"d\":true, \"e\":false, \"f\":null, \"g\":10},"
            "\"a\": [{\"b\":5}, [1,2,3,4], \"test\", true, false, null, 10]}";

        memset(toks, 0,  sizeof(toks));
        status = mm_json_load(toks, 128, &read, buf, sizeof(buf));
        test_assert(status == MM_JSON_OK);
        test_assert(mm_json_query_string(buffer, 256, &size, toks, read, "b.c")==MM_JSON_STRING);
        test_assert(!strcmp(buffer, "test"));
        test_assert(mm_json_query_string(buffer, 256, &size, toks, read, "a[2]")==MM_JSON_STRING);
        test_assert(!strcmp(buffer, "test"));
        test_assert(mm_json_query_string(buffer, 256, &size, toks, read, "a[0]")==MM_JSON_OBJECT);
        test_assert(mm_json_query_string(buffer, 256, &size, toks, read, "b.d")==MM_JSON_TRUE);
        test_assert(mm_json_query_string(buffer, 256, &size, toks, read, "b.h")==MM_JSON_NONE);
    }

    test_section("query_type")
    {
        mm_json_size read = 0;
        enum mm_json_status status;
        struct mm_json_token toks[128];
        const mm_json_char buf[] = "{\"b\": {\"a\": {\"b\":5}, \"b\":[1,2,3,4],"
            "\"c\":\"test\", \"d\":true, \"e\":false, \"f\":null, \"g\":10},"
            "\"a\": [{\"b\":5}, [1,2,3,4], \"test\", true, false, null, 10]}";

        memset(toks, 0,  sizeof(toks));
        status = mm_json_load(toks, 128, &read, buf, sizeof(buf));
        test_assert(status == MM_JSON_OK);
        test_assert(mm_json_query_type(toks, read, "b") == MM_JSON_OBJECT);
        test_assert(mm_json_query_type(toks, read, "b.b") == MM_JSON_ARRAY);
        test_assert(mm_json_query_type(toks, read, "b.b[0]") == MM_JSON_NUMBER);
        test_assert(mm_json_query_type(toks, read, "b.c") == MM_JSON_STRING);
        test_assert(mm_json_query_type(toks, read, "b.d") == MM_JSON_TRUE);
        test_assert(mm_json_query_type(toks, read, "b.e") == MM_JSON_FALSE);
        test_assert(mm_json_query_type(toks, read, "b.f") == MM_JSON_NULL);
    }
    test_result();
    return fail_count;
}

int main(void)
{
    return run_test();
}

