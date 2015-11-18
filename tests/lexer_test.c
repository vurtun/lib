#define MML_USE_FIXED_TYPES
#define MML_USE_ASSERT
#define MML_IMPLEMENTATION
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
     test_assert(!mml_token_cmp((t), content));}

static void
test_log(void *pArg, enum mml_log_level type, mml_size line, const char *fmt, ...)
{
    char buffer[1024];
    va_list arglist;
    va_start(arglist, fmt);
    printf("%s(%lu):  ", (type == MML_WARNING) ? "Warning" : "Error", line);
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
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "name", MML_TOKEN_NAME, 0);
    }

    test_section("int")
    {
        const char text[] = "47845";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "47845", MML_TOKEN_NUMBER, (MML_TOKEN_DEC|MML_TOKEN_INT));
        test_assert(mml_token_to_int(&tok) == 47845);
    }

    test_section("hex")
    {
        const char text[] = "0xDEADBEEF";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "0xDEADBEEF", MML_TOKEN_NUMBER, MML_TOKEN_HEX);
        test_assert(mml_token_to_unsigned_long(&tok) == 3735928559);
    }

    test_section("oct")
    {
        const char text[] = "013471";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "013471", MML_TOKEN_NUMBER, MML_TOKEN_OCT);
        test_assert(mml_token_to_int(&tok) == 5945);
    }

    test_section("bin")
    {
        const char text[] = "0b10";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "0b10", MML_TOKEN_NUMBER, MML_TOKEN_BIN);
        test_assert(mml_token_to_int(&tok) == 2);
    }

    test_section("float")
    {
        float value;
        const char text[] = "5684.675f";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "5684.675", MML_TOKEN_NUMBER, 0);
        value = mml_token_to_float(&tok);
        test_assert(value >= 5684.675f && value <= 5684.676f);
    }

    test_section("double")
    {
        const char text[] = "0.544";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "0.544", MML_TOKEN_NUMBER, MML_TOKEN_FLOAT|MML_TOKEN_DOUBLE_PREC);
        test_assert(mml_token_to_double(&tok) == 0.544);
    }

    test_section("neg_int")
    {
        const char text[] = "-23957";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "-", MML_TOKEN_PUNCTUATION, MML_PUNCT_SUB);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "23957", MML_TOKEN_NUMBER, MML_TOKEN_FLOAT|MML_TOKEN_DOUBLE_PREC);
        test_assert((-mml_token_to_int(&tok)) == -23957);
    }

    test_section("neg_float")
    {
        const char text[] = "-1.845f";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "-", MML_TOKEN_PUNCTUATION, MML_PUNCT_SUB);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "1.845", MML_TOKEN_NUMBER, 0);
        test_assert((-mml_token_to_float(&tok)) == -1.845f);
    }

    test_section("neg_double")
    {
        const char text[] = "-34356.4384";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "-", MML_TOKEN_PUNCTUATION, MML_PUNCT_SUB);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "34356.4384", MML_TOKEN_NUMBER, MML_TOKEN_FLOAT|MML_TOKEN_DOUBLE_PREC);
        test_assert((-mml_token_to_double(&tok)) == -34356.4384);
    }

    test_section("string")
    {
        const char text[] = "\"string\"";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "string", MML_TOKEN_STRING, 0);
    }

    test_section("whitespace")
    {
        const char text[] = "  \t     register";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "register", MML_TOKEN_NAME, 0);
    }

    test_section("code_decl")
    {
        const char text[] = "\t\nconst char\t*text = \"test\";\n";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);

        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "const", MML_TOKEN_NAME, 0);
        test_assert(tok.line == 2);
        test_assert(tok.line_crossed == 1);

        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "char", MML_TOKEN_NAME, 0);

        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "*", MML_TOKEN_PUNCTUATION, MML_PUNCT_MUL);

        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "text", MML_TOKEN_NAME, 0);

        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "=", MML_TOKEN_PUNCTUATION, MML_PUNCT_ASSIGN);

        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "test", MML_TOKEN_STRING, 0);

        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, ";", MML_TOKEN_PUNCTUATION, MML_PUNCT_SEMICOLON);
    }

    test_section("struct")
    {
        const char text[] =
            "struct device {"
            "   int version;"
            "   char *name;"
            "};";
        struct mml_token tok;
        struct mml_lexer lexer;
        mml_init(&lexer, text, sizeof(text), NULL, test_log, NULL);

        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "struct", MML_TOKEN_NAME, 0);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "device", MML_TOKEN_NAME, 0);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "{", MML_TOKEN_PUNCTUATION, MML_PUNCT_BRACE_OPEN);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "int", MML_TOKEN_NAME, 0);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "version", MML_TOKEN_NAME, 0);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, ";", MML_TOKEN_PUNCTUATION, MML_PUNCT_SEMICOLON);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "char", MML_TOKEN_NAME, 0);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "*", MML_TOKEN_PUNCTUATION, MML_PUNCT_MUL);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "name", MML_TOKEN_NAME, 0);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, ";", MML_TOKEN_PUNCTUATION, MML_PUNCT_SEMICOLON);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, "}", MML_TOKEN_PUNCTUATION, MML_PUNCT_BRACE_CLOSE);
        test_assert(mml_read(&lexer, &tok));
        test_token(&tok, ";", MML_TOKEN_PUNCTUATION, MML_PUNCT_SEMICOLON);
    }
    test_result();
    exit(EXIT_SUCCESS);
}
