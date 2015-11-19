/*
    Copyright (c) 2015
    vurtun <polygone@gmx.net>
    MIT license
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MMJ_STATIC
#define MMJ_IMPLEMENTATION
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
    {test_assert(!mmj_cmp(t, content));\
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
        struct mmj_iter iter;
        struct mmj_pair pair;
        mmj_char buffer[8];

        const mmj_char buf[] = "{\"name\":\"value\"}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "name"));
        test_assert(!mmj_cmp(&pair.value, "value"));
        test_assert(pair.value.type == MMJ_STRING);
        test_assert(pair.value.children == 0);
        test_assert(pair.value.sub == 0);
        test_assert(mmj_cpy(buffer, sizeof buffer, &pair.value) == 5);
        test_assert(!strcmp(&buffer[0], "value"));
    }

    test_section("num")
    {
        mmj_number num = 0;
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] = "\n{\n\"test\":13\n}\n";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "test"));
        test_assert(!mmj_cmp(&pair.value, "13"));
        test_assert(pair.value.type == MMJ_NUMBER);
        test_assert(mmj_convert(&num, &pair.value) == MMJ_NUMBER);
        test_assert(num == 13.0);
    }

    test_section("negnum")
    {
        mmj_number num = 0;
        struct mmj_pair pair;
        struct mmj_iter iter;
        const mmj_char buf[] = "{\"name\":-1234}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "name"));
        test_assert(!mmj_cmp(&pair.value, "-1234"));
        test_assert(pair.value.type == MMJ_NUMBER);
        test_assert(mmj_convert(&num, &pair.value) == MMJ_NUMBER);
        test_assert(num == -1234.0);
    }

    test_section("fracnum")
    {
        mmj_number num = 0;
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] = "{\"name\":1234.5678}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "name"));
        test_assert(!mmj_cmp(&pair.value, "1234.5678"));
        test_assert(pair.value.type == MMJ_NUMBER);
        test_assert(mmj_convert(&num, &pair.value) == MMJ_NUMBER);
        test_assert(num == 1234.5678);
    }

    test_section("negfracnum")
    {
        mmj_number num = 0;
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] = "{\"name\":-1234.5678}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "name"));
        test_assert(!mmj_cmp(&pair.value, "-1234.5678"));
        test_assert(pair.value.type == MMJ_NUMBER);
        test_assert(mmj_convert(&num, &pair.value) == MMJ_NUMBER);
        test_assert(num == -1234.5678);
    }

    test_section("exponent")
    {
        mmj_number num = 0;
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] = "{\"name\":2e+2}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "name"));
        test_assert(!mmj_cmp(&pair.value, "2e+2"));
        test_assert(pair.value.type == MMJ_NUMBER);
        test_assert(mmj_convert(&num, &pair.value) == MMJ_NUMBER);
        test_assert(num == 200.0);
    }

    test_section("negexponent")
    {
        mmj_number num = 0;
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] = "{\"name\":-1234e-2}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "name"));
        test_assert(!mmj_cmp(&pair.value, "-1234e-2"));
        test_assert(pair.value.type == MMJ_NUMBER);
        test_assert(mmj_convert(&num, &pair.value) == MMJ_NUMBER);
        test_assert(num == -12.34);
    }

    test_section("smallexp")
    {
        mmj_number num = 0;
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] = "{\"name\":2.567e-4}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "name"));
        test_assert(!mmj_cmp(&pair.value, "2.567e-4"));
        test_assert(pair.value.type == MMJ_NUMBER);
        test_assert(mmj_convert(&num, &pair.value) == MMJ_NUMBER);
        test_assert(num >= 0.0002567 && num <= 0.0002568);
    }

    test_section("utf8")
    {
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] = "{\"name\":\"$¢€𤪤\"}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "name"));
        test_assert(!mmj_cmp(&pair.value, "$¢€𤪤"));
        test_assert(pair.value.type == MMJ_STRING);
    }

    test_section("map")
    {
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] = "{\"name\":\"test\", \"age\":42, \"utf8\":\"äöü\", \"alive\":true}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "name"));
        test_assert(!mmj_cmp(&pair.value, "test"));
        test_assert(pair.value.type == MMJ_STRING);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "age"));
        test_assert(!mmj_cmp(&pair.value, "42"));
        test_assert(pair.value.type == MMJ_NUMBER);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "utf8"));
        test_assert(!mmj_cmp(&pair.value, "äöü"));
        test_assert(pair.value.type == MMJ_STRING);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "alive"));
        test_assert(!mmj_cmp(&pair.value, "true"));
        test_assert(pair.value.type == MMJ_TRUE);
    }

    test_section("array")
    {
        int i = 1;
        mmj_number num;
        struct mmj_token tok;
        struct mmj_iter iter;
        struct mmj_pair pair;

        const mmj_char buf[] = "{\"list\":[ 1.0, 2.0, 3.0, 4.0 ]}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "list"));
        test_assert(!mmj_cmp(&pair.value, "[ 1.0, 2.0, 3.0, 4.0 ]"));
        test_assert(pair.value.type == MMJ_ARRAY);
        test_assert(pair.value.children == 4);
        test_assert(pair.value.sub == 4);

        iter = mmj_begin(pair.value.str, pair.value.len);
        iter = mmj_read(&tok, &iter);
        while (iter.src) {
            test_assert(mmj_convert(&num, &tok) == MMJ_NUMBER);
            test_assert((mmj_number)i == num);
            iter = mmj_read(&tok, &iter);
            i++;
        }
    }

    test_section("sub")
    {
        mmj_number num = 0;
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] = "{\"sub\":{\"a\":1234.5678}}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "sub"));
        test_assert(!mmj_cmp(&pair.value, "{\"a\":1234.5678}"));
        test_assert(pair.value.type == MMJ_OBJECT);
        test_assert(pair.value.children == 1);
        test_assert(pair.value.sub == 2);

        iter = mmj_begin(pair.value.str, pair.value.len);
        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "a"));
        test_assert(!mmj_cmp(&pair.value, "1234.5678"));
        test_assert(pair.value.type == MMJ_NUMBER);

        test_assert(mmj_convert(&num, &pair.value) == MMJ_NUMBER);
        test_assert(num == 1234.5678);
    }

    test_section("subarray")
    {
        int i = 0;
        struct mmj_token tok;
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char check[] = "1234";
        const mmj_char buf[] = "{\"sub\":{\"a\":[1,2,3,4]}}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "sub"));
        test_assert(!mmj_cmp(&pair.value, "{\"a\":[1,2,3,4]}"));
        test_assert(pair.value.type == MMJ_OBJECT);
        test_assert(pair.value.children == 1);
        test_assert(pair.value.sub == 6);

        iter = mmj_begin(pair.value.str, pair.value.len);
        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "a"));
        test_assert(!mmj_cmp(&pair.value, "[1,2,3,4]"));
        test_assert(pair.value.type == MMJ_ARRAY);
        test_assert(pair.value.children == 4);
        test_assert(pair.value.sub == 4);

        iter = mmj_begin(pair.value.str, pair.value.len);
        iter = mmj_read(&tok, &iter);
        while (iter.src) {
            test_assert(tok.str[0] == check[i++]);
            iter = mmj_read(&tok, &iter);
        }
    }

    test_section("list")
    {
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] = "{\"sub\":{\"a\":\"b\"}, \"list\":{\"c\":\"d\"}}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "sub"));
        test_assert(!mmj_cmp(&pair.value, "{\"a\":\"b\"}"));
        test_assert(pair.value.type == MMJ_OBJECT);
        test_assert(pair.value.children == 1);
        test_assert(pair.value.sub == 2);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "list"));
        test_assert(!mmj_cmp(&pair.value, "{\"c\":\"d\"}"));
        test_assert(pair.value.type == MMJ_OBJECT);
        test_assert(pair.value.children == 1);
        test_assert(pair.value.sub == 2);
    }

    test_section("table")
    {
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] =
            "{\"sub\":{\"a\": \"b\"}, \"list\":[1,2,3,4], \"a\":true, \"b\": \"0a1b2\"}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "sub"));
        test_assert(!mmj_cmp(&pair.value, "{\"a\": \"b\"}"));
        test_assert(pair.value.type == MMJ_OBJECT);
        test_assert(pair.value.children == 1);
        test_assert(pair.value.sub == 2);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "list"));
        test_assert(!mmj_cmp(&pair.value, "[1,2,3,4]"));
        test_assert(pair.value.type == MMJ_ARRAY);
        test_assert(pair.value.children == 4);
        test_assert(pair.value.sub == 4);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "a"));
        test_assert(!mmj_cmp(&pair.value, "true"));
        test_assert(pair.value.type == MMJ_TRUE);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "b"));
        test_assert(!mmj_cmp(&pair.value, "0a1b2"));
        test_assert(pair.value.type == MMJ_STRING);
    }

    test_section("children")
    {
        struct mmj_iter iter;
        struct mmj_pair pair;
        const mmj_char buf[] = "{\"b\": {\"a\": {\"b\":5}, \"b\":[1,2,3,4],"
            "\"c\":\"test\", \"d\":true, \"e\":false, \"f\":null, \"g\":10},"
            "\"a\": [{\"b\":5}, [1,2,3,4], \"test\", true, false, null, 10]}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "b"));
        test_assert(pair.value.type == MMJ_OBJECT);
        test_assert(pair.value.children == 7);
        test_assert(pair.value.sub == 20);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "a"));
        test_assert(pair.value.type == MMJ_ARRAY);
        test_assert(pair.value.children == 7);
        test_assert(pair.value.sub == 13);
    }

    test_section("arrayofarray")
    {
        int i = 1;
        struct mmj_iter iter;
        struct mmj_pair pair;
        struct mmj_token tok;
        const mmj_char buf[] = "{\"coord\":[[[1,2], [3,4], [5,6]]]}";
        iter = mmj_begin(buf, sizeof buf);

        iter = mmj_parse(&pair, &iter);
        test_assert(!iter.err);
        test_assert(!mmj_cmp(&pair.name, "coord"));
        test_assert(pair.value.type == MMJ_ARRAY);
        test_assert(pair.value.children == 1);

        iter = mmj_begin(pair.value.str, pair.value.len);
        iter = mmj_read(&tok, &iter);
        test_assert(tok.type == MMJ_ARRAY);
        test_assert(tok.children == 3);

        iter = mmj_begin(tok.str, tok.len);
        iter = mmj_read(&tok, &iter);
        while (!iter.err && iter.src) {
            struct mmj_iter it;
            test_assert(tok.type == MMJ_ARRAY);
            test_assert(tok.children == 2);
            it = mmj_begin(tok.str, tok.len);
            it = mmj_read(&tok, &it);
            while (!it.err && it.src) {
                mmj_number n;
                test_assert(tok.type == MMJ_NUMBER);
                mmj_convert(&n, &tok);
                test_assert(n == i++);
                it = mmj_read(&tok, &it);
            }
            iter = mmj_read(&tok, &iter);
        }
    }

    test_section("totalcount")
    {
        const mmj_char buf[] =
            "{\"sub\":{\"a\": \"b\"}, \"list\":[1,2,3,4], \"a\":true, \"b\": \"0a1b2\"}";
        const mmj_char buf2[] = "{\"coord\":[[[1,2], [3,4], [5,6]]]}";
        const mmj_char buf3[] = "{\"list\":[ 1.0, 2.0, 3.0, 4.0 ]}";
        const mmj_char buf4[] = "{\"name\":\"test\", \"age\":42, \"utf8\":\"äöü\", \"alive\":true}";
        test_assert(mmj_num(buf, sizeof(buf)) == 14);
        test_assert(mmj_num(buf2, sizeof(buf2)) == 12);
        test_assert(mmj_num(buf3, sizeof(buf3)) == 6);
        test_assert(mmj_num(buf4, sizeof(buf4)) == 8);
    }

    test_section("load")
    {
        mmj_size count = 0;
        mmj_size read = 0;
        enum mmj_status status;
        struct mmj_token toks[14];
        const mmj_char buf[] =
            "{\"sub\":{\"a\": \"b\"}, \"list\":[1,2,3,4], \"a\":true, \"b\": \"0a1b2\"}";

        memset(toks, 0,  sizeof(toks));
        count = mmj_num(buf, sizeof(buf));
        test_assert(count == 14);
        status = mmj_load(toks, count, &read, buf, sizeof(buf));
        test_assert(status == MMJ_OK);
        test_token(&toks[0], "sub", MMJ_STRING, 0, 0);
        test_token(&toks[1], "{\"a\": \"b\"}", MMJ_OBJECT, 1, 2);
        test_token(&toks[2], "a", MMJ_STRING, 0, 0);
        test_token(&toks[3], "b", MMJ_STRING, 0, 0);
        test_token(&toks[4], "list", MMJ_STRING, 0, 0);
        test_token(&toks[5], "[1,2,3,4]", MMJ_ARRAY, 4, 4);
        test_token(&toks[6], "1", MMJ_NUMBER, 0, 0);
        test_token(&toks[7], "2", MMJ_NUMBER, 0, 0);
        test_token(&toks[8], "3", MMJ_NUMBER, 0, 0);
        test_token(&toks[9], "4", MMJ_NUMBER, 0, 0);
        test_token(&toks[10], "a", MMJ_STRING, 0, 0);
        test_token(&toks[11], "true", MMJ_TRUE, 0, 0);
        test_token(&toks[12], "b", MMJ_STRING, 0, 0);
        test_token(&toks[13], "0a1b2", MMJ_STRING, 0, 0);
        test_assert(read == 14);

    }

    test_section("query_simple")
    {
        mmj_size count = 0;
        mmj_size read = 0;
        enum mmj_status status;
        struct mmj_token toks[14];
        const mmj_char buf[] =
            "{\"sub\":{\"a\": \"b\"}, \"list\":[1,2,3,4], \"a\":true, \"b\": \"0a1b2\"}";

        memset(toks, 0,  sizeof(toks));
        count = mmj_num(buf, sizeof(buf));
        test_assert(count == 14);
        status = mmj_load(toks, count, &read, buf, sizeof(buf));
        test_assert(read == 14);
        test_assert(status == MMJ_OK);
        test_assert(mmj_query(toks, read, "list[0]") == &toks[6]);
        test_assert(mmj_query(toks, read, "list[3]") == &toks[9]);
        test_assert(mmj_query(toks, read, "sub.a") == &toks[3]);
        test_assert(mmj_query(toks, read, "b") == &toks[13]);
    }

    test_section("query_complex")
    {
        mmj_size read = 0;
        enum mmj_status status;
        struct mmj_token toks[128];
        const mmj_char buf[] =
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
        status = mmj_load(toks, 128, &read, buf, sizeof(buf));
        test_assert(status == MMJ_OK);
        test_assert(mmj_query(toks, read, "map") == &toks[1]);
        test_assert(mmj_query(toks, read, "map.entity") == &toks[3]);
        test_assert(mmj_query(toks, read, "map.entity[0]") == &toks[4]);
        test_assert(mmj_query(toks, read, "map.entity[1]") == &toks[17]);
        test_assert(mmj_query(toks, read, "map.entity[2]") == &toks[30]);
        test_assert(mmj_query(toks, read, "map.entity[3]") == &toks[43]);
        test_assert(mmj_query(toks, read, "map.entity[4]") == &toks[56]);
        test_assert(mmj_query(toks, read, "map.entity[0].position.x") == &toks[8]);
        test_assert(mmj_query(toks, read, "map.entity[2].position.y") == &toks[36]);
        test_assert(mmj_query(toks, read, "map.entity[4].size.w") == &toks[66]);
        {
            struct mmj_token *tok = mmj_query(toks, read, "map.entity");
            test_assert(tok->children == 5);
        }
    }

    test_section("query_entangled")
    {
        mmj_size read = 0;
        enum mmj_status status;
        struct mmj_token toks[128];
        const mmj_char buf[] = "{\"b\": {\"a\": {\"b\":5}, \"b\":[1,2,3,4],"
            "\"c\":\"test\", \"d\":true, \"e\":false, \"f\":null, \"g\":10},"
            "\"a\": [{\"b\":5}, [1,2,3,4], \"test\", true, false, null, 10]}";

        memset(toks, 0,  sizeof(toks));
        status = mmj_load(toks, 128 , &read, buf, sizeof(buf));
        test_assert(status == MMJ_OK);
        test_assert(mmj_query(toks, read, "b") == &toks[1]);
        test_assert(mmj_query(toks, read, "b.a") == &toks[3]);
        test_assert(mmj_query(toks, read, "b.a.b") == &toks[5]);
        test_assert(mmj_query(toks, read, "b.b") == &toks[7]);
        test_assert(mmj_query(toks, read, "b.c") == &toks[13]);
        test_assert(mmj_query(toks, read, "b.d") == &toks[15]);
        test_assert(mmj_query(toks, read, "b.e") == &toks[17]);
        test_assert(mmj_query(toks, read, "b.f") == &toks[19]);
        test_assert(mmj_query(toks, read, "a[0]") == &toks[24]);
        test_assert(mmj_query(toks, read, "a[0].b") == &toks[26]);
        test_assert(mmj_query(toks, read, "a[1]") == &toks[27]);
        test_assert(mmj_query(toks, read, "a[1][0]") == &toks[28]);
        test_assert(mmj_query(toks, read, "a[1][1]") == &toks[29]);
        test_assert(mmj_query(toks, read, "a[1][2]") == &toks[30]);
        test_assert(mmj_query(toks, read, "a[1][3]") == &toks[31]);
        test_assert(mmj_query(toks, read, "a[2]") == &toks[32]);
        test_assert(mmj_query(toks, read, "a[3]") == &toks[33]);
        test_assert(mmj_query(toks, read, "a[4]") == &toks[34]);
        test_assert(mmj_query(toks, read, "a[5]") == &toks[35]);
        test_assert(mmj_query(toks, read, "a[6]") == &toks[36]);
    }

    test_section("query_sub")
    {
        mmj_size read = 0;
        enum mmj_status status;
        struct mmj_token toks[128];
        struct mmj_token *entity;
        struct mmj_token *position;
        struct mmj_token *size;

        const mmj_char buf[] =
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
        status = mmj_load(toks, 128, &read, buf, sizeof(buf));
        test_assert(status == MMJ_OK);
        entity = mmj_query(toks, read, "map.entity[2]");
        test_assert(entity == &toks[30]);
        position = mmj_query(entity, entity->sub, "position");
        test_assert(position == &toks[32]);
        size = mmj_query(entity, entity->sub, "size");
        test_assert(size == &toks[38]);
        {
            struct mmj_token *x, *y;
            x = mmj_query(position, position->sub, "x");
            y = mmj_query(position, position->sub, "y");
            test_assert(x == &toks[34]);
            test_assert(y == &toks[36]);
        }
        {
            struct mmj_token *w, *h;
            w = mmj_query(size, size->sub, "w");
            h = mmj_query(size, size->sub, "h");
            test_assert(w == &toks[40]);
            test_assert(h == &toks[42]);
        }
    }

    test_section("query_number")
    {
        mmj_size read = 0;
        enum mmj_status status;
        struct mmj_token toks[128];
        mmj_number num;
        mmj_int ret;
        mmj_int i = 0;

        const mmj_char buf[] =
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
        status = mmj_load(toks, 128, &read, buf, sizeof(buf));
        test_assert(status == MMJ_OK);
        for (i = 0; i < 5; ++i) {
            mmj_char path[32];
            sprintf(path, "map.entity[%d].position.x", i);
            ret = mmj_query_number(&num, toks, read, path);
            test_assert(ret == MMJ_NUMBER);
            test_assert(num == (mmj_number)(i + 1));

            sprintf(path, "map.entity[%d].position.y", i);
            ret = mmj_query_number(&num, toks, read, path);
            test_assert(ret == MMJ_NUMBER);
            test_assert(num == (mmj_number)(i + 1));

            sprintf(path, "map.entity[%d].size.w", i);
            ret = mmj_query_number(&num, toks, read, path);
            test_assert(ret == MMJ_NUMBER);
            test_assert(num == (mmj_number)(i + 1));

            sprintf(path, "map.entity[%d].size.h", i);
            ret = mmj_query_number(&num, toks, read, path);
            test_assert(ret == MMJ_NUMBER);
            test_assert(num == (mmj_number)(i + 1));
        }
        test_assert(mmj_query_number(&num, toks, read, "map.test") == MMJ_NONE);
        test_assert(mmj_query_number(&num, toks, read, "map.entity") == MMJ_ARRAY);
    }

    test_section("query_string")
    {
        mmj_size read = 0;
        enum mmj_status status;
        struct mmj_token toks[128];
        mmj_char buffer[256];
        mmj_size size;

        const mmj_char buf[] = "{\"b\": {\"a\": {\"b\":5}, \"b\":[1,2,3,4],"
            "\"c\":\"test\", \"d\":true, \"e\":false, \"f\":null, \"g\":10},"
            "\"a\": [{\"b\":5}, [1,2,3,4], \"test\", true, false, null, 10]}";

        memset(toks, 0,  sizeof(toks));
        status = mmj_load(toks, 128, &read, buf, sizeof(buf));
        test_assert(status == MMJ_OK);
        test_assert(mmj_query_string(buffer, 256, &size, toks, read, "b.c")==MMJ_STRING);
        test_assert(!strcmp(buffer, "test"));
        test_assert(mmj_query_string(buffer, 256, &size, toks, read, "a[2]")==MMJ_STRING);
        test_assert(!strcmp(buffer, "test"));
        test_assert(mmj_query_string(buffer, 256, &size, toks, read, "a[0]")==MMJ_OBJECT);
        test_assert(mmj_query_string(buffer, 256, &size, toks, read, "b.d")==MMJ_TRUE);
        test_assert(mmj_query_string(buffer, 256, &size, toks, read, "b.h")==MMJ_NONE);
    }

    test_section("query_type")
    {
        mmj_size read = 0;
        enum mmj_status status;
        struct mmj_token toks[128];
        const mmj_char buf[] = "{\"b\": {\"a\": {\"b\":5}, \"b\":[1,2,3,4],"
            "\"c\":\"test\", \"d\":true, \"e\":false, \"f\":null, \"g\":10},"
            "\"a\": [{\"b\":5}, [1,2,3,4], \"test\", true, false, null, 10]}";

        memset(toks, 0,  sizeof(toks));
        status = mmj_load(toks, 128, &read, buf, sizeof(buf));
        test_assert(status == MMJ_OK);
        test_assert(mmj_query_type(toks, read, "b") == MMJ_OBJECT);
        test_assert(mmj_query_type(toks, read, "b.b") == MMJ_ARRAY);
        test_assert(mmj_query_type(toks, read, "b.b[0]") == MMJ_NUMBER);
        test_assert(mmj_query_type(toks, read, "b.c") == MMJ_STRING);
        test_assert(mmj_query_type(toks, read, "b.d") == MMJ_TRUE);
        test_assert(mmj_query_type(toks, read, "b.e") == MMJ_FALSE);
        test_assert(mmj_query_type(toks, read, "b.f") == MMJ_NULL);
    }
    test_result();
    return fail_count;
}

int main(void)
{
    return run_test();
}

