#define LEXER_USE_FIXED_TYPES
#define LEXER_USE_ASSERT
#define LEXER_IMPLEMENTATION
#include "../mm_lexer.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define PASS "PASS"
#define FAIL "FAIL"

#define test_section(desc) \
    do { \
        printf("--------------- {%s} ---------------\n", desc);\
    } while (0);

#define test_assert(cond) \
    do { \
        int pass = cond; \
        printf("[%s] %s:%d: ", pass ? PASS : FAIL, __FILE__, __LINE__);\
        printf((strlen(#cond) > 60 ? "%.47s...\n" : "%s\n"), #cond);\
        if (pass) pass_count++; else fail_count++; \
    } while (0)

#define test_result()\
    do { \
        printf("======================================================\n"); \
        printf("== Result:  %3d Total   %3d Passed      %3d Failed  ==\n", \
                pass_count  + fail_count, pass_count, fail_count); \
        printf("======================================================\n"); \
    } while (0)

#define test_token(t, content, types, subtypes)\
    {test_assert((t)->type == types);\
     test_assert(((t)->subtype & subtypes) == subtypes);\
     test_assert(!lexer_token_cmp((t), content));}

static void
test_log(void *pArg, enum lexer_log_level type, lexer_size line, const char *fmt, ...)
{
    char buffer[1024];
    va_list arglist;
    (void)pArg;
    va_start(arglist, fmt);
    printf("%s(%lu):  ", (type == LEXER_WARNING) ? "Warning" : "Error", line);
    vprintf(fmt, arglist);
    va_end(arglist);
}

int main(void)
{
    int pass_count = 0;
    int fail_count = 0;

    test_section("name")
    {
        const char text[] = "name";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "name", LEXER_TOKEN_NAME, 0);
    }

    test_section("int")
    {
        const char text[] = "47845";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "47845", LEXER_TOKEN_NUMBER, (LEXER_TOKEN_DEC|LEXER_TOKEN_INT));
        test_assert(lexer_token_to_int(&tok) == 47845);
    }

    test_section("hex")
    {
        const char text[] = "0xDEADBEEF";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "0xDEADBEEF", LEXER_TOKEN_NUMBER, LEXER_TOKEN_HEX);
        test_assert(lexer_token_to_unsigned_long(&tok) == 3735928559);
    }

    test_section("oct")
    {
        const char text[] = "013471";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "013471", LEXER_TOKEN_NUMBER, LEXER_TOKEN_OCT);
        test_assert(lexer_token_to_int(&tok) == 5945);
    }

    test_section("bin")
    {
        const char text[] = "0b10";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "0b10", LEXER_TOKEN_NUMBER, LEXER_TOKEN_BIN);
        test_assert(lexer_token_to_int(&tok) == 2);
    }

    test_section("float")
    {
        float value;
        const char text[] = "5684.675f";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "5684.675", LEXER_TOKEN_NUMBER, 0);
        value = lexer_token_to_float(&tok);
        test_assert(value >= 5684.675f && value <= 5684.676f);
    }

    test_section("double")
    {
        const char text[] = "0.544";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "0.544", LEXER_TOKEN_NUMBER, (LEXER_TOKEN_FLOAT|LEXER_TOKEN_DOUBLE_PREC));
        test_assert(lexer_token_to_double(&tok) == 0.544);
    }

    test_section("neg_int")
    {
        const char text[] = "-23957";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "-", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_SUB);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "23957", LEXER_TOKEN_NUMBER, 0);
        test_assert((-lexer_token_to_int(&tok)) == -23957);
    }

    test_section("neg_float")
    {
        const char text[] = "-1.845f";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "-", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_SUB);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "1.845", LEXER_TOKEN_NUMBER, 0);
        test_assert((-lexer_token_to_float(&tok)) == -1.845f);
    }

    test_section("neg_double")
    {
        const char text[] = "-34356.4384";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "-", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_SUB);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "34356.4384", LEXER_TOKEN_NUMBER, (LEXER_TOKEN_FLOAT|LEXER_TOKEN_DOUBLE_PREC));
        test_assert((-lexer_token_to_double(&tok)) == -34356.4384);
    }

    test_section("string")
    {
        const char text[] = "\"string\"";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "string", LEXER_TOKEN_STRING, 0);
    }

    test_section("whitespace")
    {
        const char text[] = "  \t     register";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "register", LEXER_TOKEN_NAME, 0);
    }

    test_section("code_decl")
    {
        const char text[] = "\t\nconst char\t*text = \"test\";\n";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);

        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "const", LEXER_TOKEN_NAME, 0);
        test_assert(tok.line == 2);
        test_assert(tok.line_crossed == 1);

        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "char", LEXER_TOKEN_NAME, 0);

        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "*", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_MUL);

        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "text", LEXER_TOKEN_NAME, 0);

        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "=", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_ASSIGN);

        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "test", LEXER_TOKEN_STRING, 0);

        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, ";", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_SEMICOLON);
    }

    test_section("struct")
    {
        const char text[] =
            "struct device {"
            "   int version;"
            "   char *name;"
            "};";
        struct lexer_token tok;
        struct lexer lexer;
        lexer_init(&lexer, text, sizeof(text), NULL, test_log, NULL);

        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "struct", LEXER_TOKEN_NAME, 0);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "device", LEXER_TOKEN_NAME, 0);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "{", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_BRACE_OPEN);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "int", LEXER_TOKEN_NAME, 0);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "version", LEXER_TOKEN_NAME, 0);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, ";", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_SEMICOLON);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "char", LEXER_TOKEN_NAME, 0);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "*", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_MUL);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "name", LEXER_TOKEN_NAME, 0);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, ";", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_SEMICOLON);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, "}", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_BRACE_CLOSE);
        test_assert(lexer_read(&lexer, &tok));
        test_token(&tok, ";", LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_SEMICOLON);
    }
    test_result();
    exit(EXIT_SUCCESS);
}
