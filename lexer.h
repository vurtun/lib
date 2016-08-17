/*
    lexer.h - zlib - Micha Mettke

ABOUT:
    This is a single header c-like language lexer/tokenizer header
    and implementation without any dependencies (even the standard library),
    or string memory allocation.
    Instead this library focuses on parsing tokens from a previously
    loaded in memory source file. Each token thereby references the source
    string and limits the allocation to the initial source string instead of
    allocating a new string for each token.

QUICK:
    To use this file do:
    #define LEXER_IMPLEMENTATION
    before you include this file in *one* C or C++ file to create the implementation

    If you want to keep the implementation in that file you have to do
    #define LEXER_STATIC before including this file

    If you want to use asserts to add validation add
    #define LEXER_ASSERT before including this file

LICENSE: (zlib)
    Copyright (c) 2016 Micha Mettke

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1.  The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software
        in a product, an acknowledgment in the product documentation would be
        appreciated but is not required.
    2.  Altered source versions must be plainly marked as such, and must not be
        misrepresented as being the original software.
    3.  This notice may not be removed or altered from any source distribution.

DEFINES:
    LEXER_IMPLEMENTATION
        Generates the implementation of the library into the included file.
        If not provided the library is in header only mode and can be included
        in other headers or source files without problems. But only ONE file
        should hold the implementation.

    LEXER_STATIC
        The generated implementation will stay private inside implementation
        file and all internal symbols and functions will only be visible inside
        that file.

    LEXER_ASSERT
    LEXER_USE_ASSERT
        If you define LEXER_USE_ASSERT without defining ASSERT lexer.h
        will use assert.h and assert(). Otherwise it will use your assert
        method. If you do not define LEXER_USE_ASSERT no additional checks
        will be added. This is the only C standard library function used
        by lexer.

    LEXER_SIZE_TYPE
        You can define this to 'size_t' if you use the standard library,
        otherwise it needs to be able to hold the maximum addressable memory
        space. If you do not define this it will default to unsigned long.

    LEXER_MEMSET
        You can define this to 'memset' or your own memset replacement.
        If not lexer uses a naive (maybe inefficent) implementation.


LIMITATIONS:
    Convert precision:
        Conversion from string to float/double is limited since
        my naive implementation does not account for error rates
        or nasty floating point values while converting.
        If there was a relativly simple implementation I would had have
        added it but there is none. If you require highly precise string to
        float conversion (I am sorry to hear that) please copy the token
        into a string buffer and use strtod.

    Float-point exceptions
        The lexer support the float point exceptions:
            ".#INF, .#IND", .#NAN, .#QNAN and .#SNAN
        but requires IEE 754 floating point numbers without support
        for extended precision floats.

    Preprocessor
        Does not do any preprocessor magic or anything like that. It only
        tokenizes string.

    Multiline strings
        Multiline strings are read in as single strings, since I do not allocate
        strings for tokens I cannot modify the string to create a single string
        out of multiple strings. This should not be a problem practice but probably
        important to know.

    Unicode
        This lexer does not work with unicode character in any way or form.
        I personally do not use non-ASCII characters and C does not require
        unicode support.

    Error handling
        At the moment there is a error flag and a logging function to check for
        errors / warnings. But the overall handling is not very extensive.

EXAMPLES:*/
#if 0
    /* initialize lexer */
    struct lexer lexer;
    lexer_init(&lexer, text, text_length, NULL, NULL, NULL);

    /* parse tokens */
    struct lexer_token tok;
    lexer_read(&lexer, &tok);
    lexer_expect_string(&lexer, "string");
    lexer_expect_type(&lexer, LEXER_TOKEN_NUMBER, LEXER_TOKEN_HEX, &tok);
    lexer_expect_any(&lexer, &tok);

    /* check and parse only if correct */
    if (lexer_check_string(&lexer, "string")) { }
        /* correct string  */
    if (lexer_check_type(&lexer, LEXER_TOKEN_NUMBER, LEXER_TOKEN_BIN, &tok)) { }
        /* correct type */

    /* only check but don't parse */
    if (lexer_peek_string(&lexer, "string")) { }
        /* correct string  */
    if (lexer_peek_type(&lexer, LEXER_TOKEN_PUNCTUATION, LEXER_PUNCT_DOLLAR, &tok)) { }
        /* correct type */

    /* token compare function */
    if (!lexer_token_cmp(&tok, "string")) { }
        /* token holds string 'string' */
    if (!lexer_token_icmp(&tok, "string")) { }
        /* token holds case independent string 'string' */

    /* copy token content into buffer */
    char buffer[1024];
    lexer_token_cpy(buffer, 1024, &tok);

    /* token to number conversion */
    /* You should always check if the token (sub)type is correct */
    int i = lexer_token_to_int(&tok);
    float f = lexer_token_to_float(&tok);
    double d = lexer_token_to_double(&tok);
    unsigned long ul = lexer_token_to_unsigned_long(&tok);
#endif

 /* ===============================================================
 *
 *                          HEADER
 *
 * =============================================================== */
#ifndef LEXER_H_
#define LEXER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LEXER_STATIC
#define LEXER_API static
#else
#define LEXER_API extern
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 19901L)
#include <stdint.h>
#ifndef LEXER_SIZE_TYPE
#define LEXER_SIZE_TYPE uintptr_t
#endif
#else
#ifndef LEXER_SIZE_TYPE
#define LEXER_SIZE_TYPE unsigned long
#endif
#endif
typedef unsigned char lexer_byte;
typedef LEXER_SIZE_TYPE lexer_size;

/* ---------------------------------------------------------------
 *                      PUNCTUATION
 * ---------------------------------------------------------------*/
/* This is the default punctuation table to convert from string to
 * an identifier. You can create your own punctuation table
 * and use your custom punctuation parsing scheme.
 * IMPORTANT: the list has to be ordered by string length */
#define LEXER_DEFAULT_PUNCTION_MAP(PUNCTUATION)\
    PUNCTUATION(">>=",  LEXER_PUNCT_RSHIFT_ASSIGN)\
    PUNCTUATION("<<=",  LEXER_PUNCT_LSHIFT_ASSIGN)\
    PUNCTUATION("...",  LEXER_PUNCT_PARAMS)\
    PUNCTUATION("&&",   LEXER_PUNCT_LOGIC_AND)\
    PUNCTUATION("||",   LEXER_PUNCT_LOGIC_OR)\
    PUNCTUATION(">=",   LEXER_PUNCT_LOGIC_GEQ)\
    PUNCTUATION("<=",   LEXER_PUNCT_LOGIC_LEQ)\
    PUNCTUATION("==",   LEXER_PUNCT_LOGIC_EQ)\
    PUNCTUATION("!=",   LEXER_PUNCT_LOGIC_UNEQ)\
    PUNCTUATION("*=",   LEXER_PUNCT_MUL_ASSIGN)\
    PUNCTUATION("/=",   LEXER_PUNCT_DIV_ASSIGN)\
    PUNCTUATION("%=",   LEXER_PUNCT_MOD_ASSIGN)\
    PUNCTUATION("+=",   LEXER_PUNCT_ADD_ASSIGN)\
    PUNCTUATION("-=",   LEXER_PUNCT_SUB_ASSIGN)\
    PUNCTUATION("++",   LEXER_PUNCT_INC)\
    PUNCTUATION("--",   LEXER_PUNCT_DEC)\
    PUNCTUATION("&=",   LEXER_PUNCT_BIN_AND_ASSIGN)\
    PUNCTUATION("|=",   LEXER_PUNCT_BIN_OR_ASSIGN)\
    PUNCTUATION("^=",   LEXER_PUNCT_BIN_XOR_ASSIGN)\
    PUNCTUATION(">>",   LEXER_PUNCT_RSHIFT)\
    PUNCTUATION("<<",   LEXER_PUNCT_LSHIFT)\
    PUNCTUATION("->",   LEXER_PUNCT_POINTER)\
    PUNCTUATION("::",   LEXER_PUNCT_CPP1)\
    PUNCTUATION(".*",   LEXER_PUNCT_CPP2)\
    PUNCTUATION("*",    LEXER_PUNCT_MUL)\
    PUNCTUATION("/",    LEXER_PUNCT_DIV)\
    PUNCTUATION("%",    LEXER_PUNCT_MOD)\
    PUNCTUATION("+",    LEXER_PUNCT_ADD)\
    PUNCTUATION("-",    LEXER_PUNCT_SUB)\
    PUNCTUATION("=",    LEXER_PUNCT_ASSIGN)\
    PUNCTUATION("&",    LEXER_PUNCT_BIN_AND)\
    PUNCTUATION("|",    LEXER_PUNCT_BIN_OR)\
    PUNCTUATION("^",    LEXER_PUNCT_BIN_XOR)\
    PUNCTUATION("~",    LEXER_PUNCT_BIN_NOT)\
    PUNCTUATION("!",    LEXER_PUNCT_LOGIC_NOT)\
    PUNCTUATION(">",    LEXER_PUNCT_LOGIC_GREATER)\
    PUNCTUATION("<",    LEXER_PUNCT_LOGIC_LESS)\
    PUNCTUATION(".",    LEXER_PUNCT_REF)\
    PUNCTUATION(",",    LEXER_PUNCT_COMMA)\
    PUNCTUATION(";",    LEXER_PUNCT_SEMICOLON)\
    PUNCTUATION(":",    LEXER_PUNCT_COLON)\
    PUNCTUATION("?",    LEXER_PUNCT_QUESTIONMARK)\
    PUNCTUATION("(",    LEXER_PUNCT_PARENTHESE_OPEN)\
    PUNCTUATION(")",    LEXER_PUNCT_PARENTHESE_CLOSE)\
    PUNCTUATION("{",    LEXER_PUNCT_BRACE_OPEN)\
    PUNCTUATION("}",    LEXER_PUNCT_BRACE_CLOSE)\
    PUNCTUATION("[",    LEXER_PUNCT_BRACKET_OPEN)\
    PUNCTUATION("]",    LEXER_PUNCT_BRACKET_CLOSE)\
    PUNCTUATION("\\",   LEXER_PUNCT_BACKSLASH)\
    PUNCTUATION("#",    LEXER_PUNCT_PRECOMPILER)\
    PUNCTUATION("$",    LEXER_PUNCT_DOLLAR)

enum lexer_default_punctuation_ids {
#define PUNCTUATION(chars, id) id,
    LEXER_DEFAULT_PUNCTION_MAP(PUNCTUATION)
#undef PUNCTUATION
    LEXER_PUNCT_MAX
};
struct lexer_punctuation {const char *string;int id;};

/* ---------------------------------------------------------------
 *                          TOKEN
 * ---------------------------------------------------------------*/
enum lexer_token_type {
    LEXER_TOKEN_STRING,
    /* strings literal: "string" */
    LEXER_TOKEN_LITERAL,
    /* character literal: 'c' */
    LEXER_TOKEN_NUMBER,
    /* integer or floating pointer number */
    LEXER_TOKEN_NAME,
    /* names and keyword */
    LEXER_TOKEN_PUNCTUATION
    /* punctuation from the punctuation table */
};

/* token subtype flags */
enum lexer_token_flags {
    LEXER_TOKEN_INT           = 0x00001,/* integer type */
    LEXER_TOKEN_DEC           = 0x00002,/* decimal number */
    LEXER_TOKEN_HEX           = 0x00004,/* hexadecimal number */
    LEXER_TOKEN_OCT           = 0x00008,/* octal number */
    LEXER_TOKEN_BIN           = 0x00010,/* binary number */
    LEXER_TOKEN_LONG          = 0x00020,/* long integer number */
    LEXER_TOKEN_UNSIGNED      = 0x00040,/* unsigned number */
    LEXER_TOKEN_FLOAT         = 0x00080,/* floating pointer number */
    LEXER_TOKEN_SINGLE_PREC   = 0x00100,/* single precision float */
    LEXER_TOKEN_DOUBLE_PREC   = 0x00200,/* double precision float */
    LEXER_TOKEN_INFINITE      = 0x00400,/* infinit float number */
    LEXER_TOKEN_INDEFINITE    = 0x00800,/* indefinite float number */
    LEXER_TOKEN_NAN           = 0x01000,/* Not a number float */
    LEXER_TOKEN_VALIDVAL      = 0x02000 /* flag if the token is a number */
};

struct lexer_token {
    enum lexer_token_type type;
    /* main type of the token */
    unsigned int subtype;
    /* subtype flags of the token */
    lexer_size line;
    /* text line the token was read from */
    int line_crossed;
    /* flag indicating if the token spans over multible lines */
    struct {unsigned long i; double f;} value;
    /* number representation of the token */
    const char *str;
    /* text pointer to the beginning of the token inside the text */
    lexer_size len;
    /* byte length of the token */
};

LEXER_API lexer_size lexer_token_cpy(char*, lexer_size max, const struct lexer_token*);
LEXER_API int lexer_token_cmp(const struct lexer_token*, const char*);
LEXER_API int lexer_token_icmp(const struct lexer_token*, const char*);
LEXER_API int lexer_token_to_int(struct lexer_token*);
LEXER_API float lexer_token_to_float(struct lexer_token*);
LEXER_API double lexer_token_to_double(struct lexer_token*);
LEXER_API unsigned long lexer_token_to_unsigned_long(struct lexer_token*);

/* ---------------------------------------------------------------
 *
 *                          LEXER
 *
 * ---------------------------------------------------------------*/
/* logging callback */
enum lexer_log_level {LEXER_WARNING,LEXER_ERROR};
typedef void(*lexer_log_f)(void*, enum lexer_log_level, lexer_size line, const char *msg, ...);

struct lexer {
/*  The lexer context holds the current state of the parsing process,
    and only refrences a string that actually holds the source text.
    To parse punctuations the lexer uses a punctuation table to map
    between a string and identifier for each punctuation. The library
    thereby already has a default table but you can create a custom
    table to suite your needs.
    For error handling the lexer supports an error flag that can be
    checked as well a logging callback for more extensive error handling.*/
    int error;
    /* error flags that will be set if an error occurs*/
    const char *buffer;
    /* pointer to the beginning the the text to parse */
    const char *current;
    /* current position inside the text */
    const char *last;
    /* internal helper pointer to the last read text position */
    const char *end;
    /* pointer to the end of the text to parse */
    lexer_size length;
    /* length of the text to parse */
    lexer_size line;
    /* current line the text */
    lexer_size last_line;
    /* last parsed line */
    const struct lexer_punctuation *puncts;
    /* internally used punctuation table */
    lexer_log_f log;
    /* logging callback for outputing error messages */
    void *userdata;
    /* userdata passed to the logging callback */
};

LEXER_API void lexer_init(struct lexer *lexer, const char *ptr, lexer_size len,
                    const struct lexer_punctuation *punct, lexer_log_f log, void *usr);
/*  this function initializes the lexer
    Input:
    - pointer to a text buffer to parse
    - length of the text buffer
    - custom punctuation table or NULL for default table
    - logging callback or NULL if not needed
    - userdata passed into the callback or NULL if not needed
*/
LEXER_API void lexer_reset(struct lexer*);
/*  this function resets the lexer back to beginning */
LEXER_API int lexer_read(struct lexer*, struct lexer_token*);
/*  this function reads a token from a loaded lexer
    Input:
    - token to hold the parsed content information
    Output:
    - if successfully 1 or 0 otherwise
*/
LEXER_API int lexer_read_on_line(struct lexer*, struct lexer_token*);
/*  this function reads a token from a loaded lexer only if on the same line
    Input:
    - token to hold the parsed content information
    Output:
    - if successfully read 1 or 0 otherwise
*/
LEXER_API void lexer_unread(struct lexer*, struct lexer_token*);
/*  this function pushes back a token (limitied to exactly one token)
    Input:
    - token push back into the lexer
*/
LEXER_API int lexer_expect_string(struct lexer*, const char*);
/*  this function reads a token and check the token content. If the token
    content is not equal to the provided string a error occurs.
    Input:
    - the expected parsed token string
    Output:
    - 1 if a token could be read or 0 otherwise
*/
LEXER_API int lexer_expect_type(struct lexer*, enum lexer_token_type type,
                                        unsigned int subtype, struct lexer_token*);
/*  this function reads a token and checks for token type + subtype. If the token
    type and subtype or not correct an error will be raised.
    Input:
    - expected token type
    - expected token subtype
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
LEXER_API int lexer_expect_any(struct lexer*, struct lexer_token*);
/*  this function tries to read in a token and if not possible will raise and error
    Input:
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
LEXER_API int lexer_check_string(struct lexer*, const char*);
/*  this function tries to read in a token holding with given content.
 *  If it succeeds the token will be returned. If not the read token will be unread.
    Input:
    - the expected parsed token string
    Output:
    - 1 if a token could be read or 0 otherwise
*/
LEXER_API int lexer_check_type(struct lexer*, enum lexer_token_type type,
                                unsigned int subtype, struct lexer_token*);
/*  this function tries to read in a token holding with token type and subtype.
 *  If it succeeds the token will be returned. If not the read token will be unread.
    Input:
    - expected token type
    - expected token subtype
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
LEXER_API int lexer_peek_string(struct lexer*, const char*);
/*  this function checks the next token for the given string content
    Input:
    - the expected parsed token string
    Output:
    - 1 if a token could be read or 0 otherwise
*/
LEXER_API int lexer_peek_type(struct lexer*, enum lexer_token_type type,
                                    unsigned int subtype, struct lexer_token*);
/*  this function checks the next token for the given type and subtype
    Input:
    - expected token type
    - expected token subtype
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
LEXER_API int lexer_skip_until(struct lexer*, const char*);
/*  this function skips all tokens until a token holding a certain string
    Input:
    - a expected string the parse should skip to
    Output:
    - 1 if successful, 0 otherwise
*/
LEXER_API int lexer_skip_line(struct lexer*);
/*  this function skips the current line */
LEXER_API int lexer_parse_int(struct lexer*);
/*  this function reads in a token and tries to convert it into an integer.
    If the conversion fails an error will be raised.
    Output:
    - parsed integer value
*/
LEXER_API int lexer_parse_bool(struct lexer*);
/*  this function reads in a token and tries to convert it into an boolean.
    If the conversion fails an error will be raised.
    Output:
    - parsed boolean value
*/
LEXER_API float lexer_parse_float(struct lexer*);
/*  this function reads in a token and tries to convert it into an float.
    If the conversion fails an error will be raised.
    Output:
    - parsed floating point value
*/
#ifdef __cplusplus
}
#endif
#endif /* LEXER_H_ */

/* ===============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================*/
#ifdef LEXER_IMPLEMENTATION

#define LEXER_LEN(a) (sizeof(a)/sizeof((a)[0]))
#define LEXER_UNUSED(a) ((void)(a))

#ifdef LEXER_USE_ASSERT
#ifndef LEXER_ASSERT
#include <assert.h>
#define LEXER_ASSERT(expr) assert(expr)
#endif
#else
#define LEXER_ASSERT(expr)
#endif

/* Pointer to Integer type conversion for pointer alignment */
#if defined(__PTRDIFF_TYPE__) /* This case should work for GCC*/
# define LEXER_UINT_TO_PTR(x) ((void*)(__PTRDIFF_TYPE__)(x))
# define LEXER_PTR_TO_UINT(x) ((lexer_size)(__PTRDIFF_TYPE__)(x))
#elif !defined(__GNUC__) /* works for compilers other than LLVM */
# define LEXER_UINT_TO_PTR(x) ((void*)&((char*)0)[x])
# define LEXER_PTR_TO_UINT(x) ((wby_size)(((char*)x)-(char*)0))
#elif defined(LEXER_USE_FIXED_TYPES) /* used if we have <stdint.h> */
# define LEXER_UINT_TO_PTR(x) ((void*)(uintptr_t)(x))
# define LEXER_PTR_TO_UINT(x) ((uintptr_t)(x))
#else /* generates warning but works */
# define LEXER_UINT_TO_PTR(x) ((void*)(x))
# define LEXER_PTR_TO_UINT(x) ((wby_size)(x))
#endif

#define LEXER_INTERN static
#define LEXER_GLOBAL static
#define LEXER_STORAGE static

#ifndef LEXER_MEMSET
#define LEXER_MEMSET lexer_memset
#endif

/* library intern default punctuation map */
LEXER_GLOBAL const struct lexer_punctuation
lexer_default_punctuations[] = {
#define PUNCTUATION(chars, id) {chars, id},
    LEXER_DEFAULT_PUNCTION_MAP(PUNCTUATION)
#undef PUNCTUATION
    {0, 0}
};
/* ---------------------------------------------------------------
 *                          UTIL
 * ---------------------------------------------------------------*/
LEXER_INTERN char
lexer_char_upper(char c)
{
    if (c >= 'a' && c <= 'z')
        return (char)('A' + (c - 'a'));
    return c;
}

LEXER_INTERN void
lexer_memset(void *ptr, int c0, lexer_size size)
{
    #define word unsigned
    #define wsize sizeof(word)
    #define wmask (wsize - 1)
    unsigned char *dst = (unsigned char*)ptr;
    unsigned c = 0;
    lexer_size t = 0;

    if ((c = (unsigned char)c0) != 0) {
        c = (c << 8) | c; /* at least 16-bits  */
        if (sizeof(unsigned int) > 2)
            c = (c << 16) | c; /* at least 32-bits*/
    }

    /* to small of a word count */
    dst = (unsigned char*)ptr;
    if (size < 3 * wsize) {
        while (size--) *dst++ = (unsigned char)c0;
        return;
    }

    /* align destination */
    if ((t = LEXER_PTR_TO_UINT(dst) & wmask) != 0) {
        t = wsize -t;
        size -= t;
        do {
            *dst++ = (unsigned char)c0;
        } while (--t != 0);
    }

    /* fill word */
    t = size / wsize;
    do {
        *(word*)((void*)dst) = c;
        dst += wsize;
    } while (--t != 0);

    /* fill trailing bytes */
    t = (size & wmask);
    if (t != 0) {
        do {
            *dst++ = (unsigned char)c0;
        } while (--t != 0);
    }

    #undef word
    #undef wsize
    #undef wmask
}

#define lexer_zero_struct(s) lexer_zero_size(&s, sizeof(s))
#define lexer_zero_array(p,n) lexer_zero_size(p, (n) * sizeof((p)[0]))
LEXER_INTERN void
lexer_zero_size(void *ptr, lexer_size size)
{
    LEXER_MEMSET(ptr, 0, size);
}

/* ---------------------------------------------------------------
 *                          TOKEN
 * ---------------------------------------------------------------*/
LEXER_INTERN void
lexer_token_clear(struct lexer_token *tok)
{
    tok->line_crossed = 0;
}

LEXER_API lexer_size
lexer_token_cpy(char *dst, lexer_size max, const struct lexer_token* tok)
{
    unsigned i = 0;
    lexer_size ret;
    lexer_size siz;

    LEXER_ASSERT(dst);
    LEXER_ASSERT(tok);
    if (!dst || !max || !tok)
        return 0;

    ret = (max < (tok->len + 1)) ? max : tok->len;
    siz = (max < (tok->len + 1)) ? max-1 : tok->len;
    for (i = 0; i < siz; i++)
        dst[i] = tok->str[i];
    dst[siz] = '\0';
    return ret;
}

LEXER_API int
lexer_token_icmp(const struct lexer_token* tok, const char* str)
{
    lexer_size i;
    LEXER_ASSERT(tok);
    LEXER_ASSERT(str);
    if (!tok || !str) return 1;
    for (i = 0; (i < tok->len && *str); i++, str++){
        if (lexer_char_upper(tok->str[i]) != lexer_char_upper(*str))
            return 1;
    }
    if (*str != 0 || i < tok->len)
        return 1;
    return 0;
}

LEXER_API int
lexer_token_cmp(const struct lexer_token* tok, const char* str)
{
    lexer_size i;
    LEXER_ASSERT(tok);
    LEXER_ASSERT(str);
    if (!tok || !str) return 1;
    for (i = 0; (i < tok->len && *str); i++, str++){
        if (tok->str[i] != *str)
            return 1;
    }
    if (*str != 0 || i < tok->len)
        return 1;
    return 0;
}

LEXER_INTERN double
lexer_token_parse_double(const char *p, lexer_size length)
{
    int i, div, pow;
    double m;
    lexer_size len = 0;
    double f = 0;
    while (len < length && p[len] != '.' && p[len] != 'e') {
        f = f * 10.0 + (double)(p[len] - '0');
        len++;
    }

    if (len < length && p[len] == '.') {
        len++;
        for (m = 0.1; len < length; len++) {
            f = f + (double)(p[len] - '0') * m;
            m *= 0.1;
        }
    }
    if (len < length && p[len] == 'e' ) {
        len++;
        if (p[len] == '-') {
            div = 1;
            len++;
        } else if (p[len] == '+') {
            div = 0;
            len++;
        } else div = 0;
        pow = 0;
        for (pow = 0; len < length; len++)
            pow = pow * 10 + (int)(p[len] - '0');
        for (m = 1.0, i = 0; i < pow; ++i)
            m *= 100.0;
        if (div) f /= m;
        else f *= m;
    }
    return f;
}

LEXER_INTERN unsigned long
lexer_token_parse_int(const char *p, lexer_size length)
{
    unsigned long i = 0;
    lexer_size len = 0;
    while (len < length) {
        i = i * 10 + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

LEXER_INTERN unsigned long
lexer_token_parse_oct(const char *p, lexer_size length)
{
    unsigned long i = 0;
    lexer_size len = 1;
    while (len < length) {
        i = (i << 3) + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

LEXER_INTERN unsigned long
lexer_token_parse_hex(const char *p, lexer_size length)
{
    unsigned long i = 0;
    lexer_size len = 2;
    while (len < length) {
        i <<= 4;
        if (p[len] >= 'a' && p[len] <= 'f')
            i += (unsigned long)((p[len] - 'a') + 10);
        else if (p[len] >= 'A' && p[len] <= 'F') {
            i += (unsigned long)((p[len] - 'A') + 10);
        } else i += (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

LEXER_INTERN unsigned long
lexer_token_parse_bin(const char *p, lexer_size length)
{
    unsigned long i = 0;
    lexer_size len = 2;
    while (len < length) {
        i = (i << 1) + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

LEXER_INTERN void
lexer_token_number_value(struct lexer_token *tok)
{
    const char *p;
    LEXER_ASSERT(tok->type == LEXER_TOKEN_NUMBER);
    tok->value.i = 0;
    tok->value.f = 0;

    p = tok->str;
    if (tok->subtype & LEXER_TOKEN_FLOAT) {
        if (tok->subtype & ((LEXER_TOKEN_INFINITE|LEXER_TOKEN_INDEFINITE|LEXER_TOKEN_NAN))) {
            /* special real number constants */
            union {float f; unsigned int u;} convert;
            if (tok->subtype & LEXER_TOKEN_INFINITE) {
                convert.u = 0x7f800000;
                tok->value.f = (double)convert.f;
            } else if (tok->subtype & LEXER_TOKEN_INDEFINITE) {
                convert.u = 0xffc00000;
                tok->value.f = (double)convert.f;
            } else if (tok->subtype & LEXER_TOKEN_NAN) {
                convert.u = 0x7fc00000;
                tok->value.f = (double)convert.f;
            }
        } else tok->value.f = lexer_token_parse_double(tok->str, tok->len);
        tok->value.i = (unsigned long)tok->value.f;
    } else if (tok->subtype & LEXER_TOKEN_DEC) {
        /* paser decimal number */
        tok->value.i = lexer_token_parse_int(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & LEXER_TOKEN_OCT) {
        /* parse octal number */
        tok->value.i = lexer_token_parse_oct(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & LEXER_TOKEN_HEX) {
        /* parse hex number */
        tok->value.i = lexer_token_parse_hex(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & LEXER_TOKEN_BIN) {
        /* parse binary number */
        tok->value.i = lexer_token_parse_bin(p, tok->len);
        tok->value.f = (double)tok->value.i;
    }
    tok->subtype |= LEXER_TOKEN_VALIDVAL;
}

LEXER_API int
lexer_token_to_int(struct lexer_token *tok)
{
    return (int)lexer_token_to_unsigned_long(tok);
}

LEXER_API float
lexer_token_to_float(struct lexer_token *tok)
{
    double d = lexer_token_to_double(tok);
    float f = (float)d;;
    return f;
}

LEXER_API double
lexer_token_to_double(struct lexer_token *tok)
{
    if (tok->type != LEXER_TOKEN_NUMBER)
        return 0.0;
    if (!(tok->subtype & LEXER_TOKEN_VALIDVAL))
        lexer_token_number_value(tok);
    if (!(tok->subtype & LEXER_TOKEN_VALIDVAL))
        return 0.0;
    return tok->value.f;
}

LEXER_API unsigned long
lexer_token_to_unsigned_long(struct lexer_token *tok)
{
    if (tok->type != LEXER_TOKEN_NUMBER)
        return 0;
    if (!(tok->subtype & LEXER_TOKEN_VALIDVAL))
        lexer_token_number_value(tok);
    if (!(tok->subtype & LEXER_TOKEN_VALIDVAL))
        return 0;
    return tok->value.i;
}

/* ---------------------------------------------------------------
 *                          LEXER
 * ---------------------------------------------------------------*/
LEXER_API void
lexer_init(struct lexer *lexer, const char *ptr, lexer_size len,
    const struct lexer_punctuation *punct, lexer_log_f log, void *userdata)
{
    lexer_zero_struct(*lexer);
    lexer->buffer = ptr;
    lexer->current = ptr;
    lexer->last = ptr;
    lexer->end = ptr + len;
    lexer->length = len;
    lexer->line = lexer->last_line = 1;
    if (!punct)
        lexer->puncts = lexer_default_punctuations;
    else lexer->puncts = punct;
    lexer->log = log;
    lexer->userdata = userdata;
}

LEXER_API void
lexer_reset(struct lexer *lexer)
{
    lexer->current = lexer->buffer;
    lexer->last = lexer->buffer;
    lexer->line = lexer->last_line = 1;
}

LEXER_INTERN int
lexer_read_white_space(struct lexer *lexer, int current_line)
{
    while (1) {
        /* skip white spaces */
        while (*lexer->current <= ' ' && lexer->current < lexer->end) {
            if (!*lexer->current || lexer->current == lexer->end)
                return 0;
            if (*lexer->current == '\n') {
                lexer->line++;
                if (current_line) {
                    lexer->current++;
                    return 1;
                }
            }
            lexer->current++;
        }

        /* skip comments */
        if (*lexer->current == '/' && lexer->current < lexer->end) {
            if (lexer->current+1 >= lexer->end)
                return 0;

            if (*(lexer->current + 1) == '/') {
                /* C++ style comments */
                lexer->current++;
                do {
                    lexer->current++;
                    if ((lexer->current >= lexer->end) || !*lexer->current)
                        return 0;
                } while (*lexer->current != '\n');
                lexer->line++;
                lexer->current++;
                if (current_line)
                    return 1;
                if (lexer->current >= lexer->end || !*lexer->current)
                    return 0;
                continue;
            } else if (*(lexer->current + 1) == '*') {
                /* C style comments */
                lexer->current++;
                while (1) {
                    lexer->current++;
                    if (lexer->current >= lexer->end || !*lexer->current)
                        return 0;
                    if (*lexer->current == '\n') {
                        lexer->line++;
                    } else if (*lexer->current == '/' && lexer->current+1 < lexer->end) {
                        if (*(lexer->current-1) == '*') break;
                        if (*(lexer->current+1) == '*' && lexer->log) {
                            lexer->log(lexer->userdata, LEXER_WARNING, lexer->line,
                                "nested comment");
                        }
                    }
                }
                lexer->current++;
                if (lexer->current >= lexer->end || !*lexer->current)
                    return 0;
                lexer->current++;
                if (lexer->current >= lexer->end || !*lexer->current)
                    return 0;
                continue;
            }
        }
        break;
    }
    return 1;
}

LEXER_INTERN int
lexer_read_esc_chars(struct lexer *lexer, char *ch)
{
    int c, val, i;
    lexer->current++;
    if (lexer->current >= lexer->end)
        return 0;

    switch (*lexer->current) {
    case '\\': c = '\\'; break;
    case 'n': c = '\n'; break;
    case 'r': c = '\r'; break;
    case 't': c = '\t'; break;
    case 'v': c = '\v'; break;
    case 'b': c = '\b'; break;
    case 'f': c = '\f'; break;
    case 'a': c = '\a'; break;
    case '\'': c = '\''; break;
    case '\?': c = '\?'; break;
    case 'x': {
        lexer->current++;
        if (lexer->current >= lexer->end)
            return 0;

        for (i = 0, val = 0; ++i; lexer->current++) {
            c = *lexer->current;
            if (c >= '0' && c <= '9')
                c = c - '0';
            if (c >= 'A' && c <= 'Z')
                c = c - 'A' + 10;
            if (c >= 'a' && c <= 'z')
                c = c - 'a' + 10;
            else break;
            val = (val << 4) + c;
        }
        lexer->current--;
        if (val > 0xFF) {
            if (lexer->log) {
                lexer->log(lexer->userdata, LEXER_WARNING, lexer->line,
                    "to large value in esc char: %d", val);
            }
            val = 0xFF;
        }
        c = val;
        break;
    } break;
    default: {
        if (*lexer->current < '0' || *lexer->current > '9') {
            if (lexer->log) {
                lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                    "unkown escape character: %c", *lexer->current);
            }
            lexer->error = 1;
        }
        for (i = 0, val = 0; i++; lexer->current++) {
            c = *lexer->current;
            if (c >= '0' && c <= '9')
                c = c - '0';
            else break;
            val = val * 10 + c;
        }
        lexer->current--;
        if (val > 0xFF) {
            if (lexer->log) {
                lexer->log(lexer->userdata, LEXER_WARNING, lexer->line,
                    "to large value in esc char: %d", val);
            }
            val = 0xFF;
        }
        c = val;
    } break;
    }
    lexer->current++;
    *ch = (char)c;
    return 1;
}

LEXER_INTERN int
lexer_read_string(struct lexer *lexer, struct lexer_token *token, int quote)
{
    lexer_size tmpline;
    const char *tmp;
    char ch;
    if (quote == '\"')
        token->type = LEXER_TOKEN_STRING;
    else token->type = LEXER_TOKEN_LITERAL;
    lexer->current++;
    if (lexer->current >= lexer->end)
        return 0;

    token->len = 0;
    token->str = lexer->current;
    while (1) {
        if (*lexer->current == '\\') {
            if (!lexer_read_esc_chars(lexer, &ch))
                return 0;
        } else if (*lexer->current == quote) {
            lexer->current++;
            if (lexer->current >= lexer->end)
                return 0;

            tmp = lexer->current;
            tmpline = lexer->line;
            if (!lexer_read_white_space(lexer, 0)) {
                lexer->current = tmp;
                lexer->line = tmpline;
                break;
            }
            if (*lexer->current == '\0') {
                if (lexer->log) {
                        lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                            "expecting string after '\' terminated line");
                }
                lexer->error = 1;
                return 0;
            }
            break;
        } else {
            if (*lexer->current == '\0') {
                if (lexer->log) {
                    lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                        "missing trailing quote");
                }
                lexer->error = 1;
                return 0;
            }
            if (*lexer->current == '\n') {
                if (lexer->log) {
                    lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                        "newline inside string");
                }
                lexer->error = 1;
                return 0;
            }
            lexer->current++;
        }
    }
    if (token->str) {
        token->len = (lexer_size)(lexer->current - token->str) - 1;
        if (token->type == LEXER_TOKEN_LITERAL)
            token->subtype = (unsigned int)token->str[0];
        else token->subtype = (unsigned int)token->len;
    }
    return 1;
}

LEXER_INTERN int
lexer_read_name(struct lexer *lexer, struct lexer_token *token)
{
    char c;
    token->type = LEXER_TOKEN_NAME;
    token->str = lexer->current;
    token->len = 0;
    do {
        token->len++;
        lexer->current++;
        if (lexer->current >= lexer->end)
            break;
        c = *lexer->current;
    } while ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '_');
    token->subtype = (unsigned int)token->len;
    return 1;
}

LEXER_INTERN int
lexer_check_str(const struct lexer *lexer, const char *str, lexer_size len)
{
    lexer_size i;
    for (i = 0; i < len && (&lexer->current[i] < lexer->end); ++i) {
        if (lexer->current[i] != str[i])
            return 0;
    }
    if (i < len) return 0;
    return 1;
}

LEXER_INTERN int
lexer_read_number(struct lexer *lexer, struct lexer_token *token)
{
    lexer_size i;
    int dot;
    char c, c2;

    token->type = LEXER_TOKEN_NUMBER;
    token->subtype = 0;
    token->value.f = 0;
    token->value.i = 0;
    token->str = 0;
    token->len = 0;

    c = *lexer->current;
    if ((lexer->current + 1) < lexer->end)
        c2 = *(lexer->current + 1);
    else c2 = 0;

    if (c == '0' && c2 != '.') {
        if (c2 == 'x' || c2 == 'X') {
            /* hex number */
            token->str = lexer->current;
            token->len += 2;
            lexer->current += 2;
            c = *lexer->current;
            while ((c >= '0' && c <= '9') ||
                   (c >= 'a' && c <= 'f') ||
                   (c >= 'A' && c <= 'F')) {
                token->len++;
                if (lexer->current+1 >= lexer->end) break;
                c = *(++lexer->current);
            }
            token->subtype = LEXER_TOKEN_HEX | LEXER_TOKEN_INT;
        } else if (c2 == 'b' || c2 == 'B') {
            /* binary number */
            token->str = lexer->current;
            token->len += 2;
            lexer->current += 2;
            c = *lexer->current;
            while (c == '0' || c == '1') {
                token->len++;
                if (lexer->current+1 >= lexer->end) break;
                c = *(++lexer->current);
            }
            token->subtype = LEXER_TOKEN_BIN | LEXER_TOKEN_INT;
        } else {
            /* octal number */
            token->str = lexer->current;
            token->len += 1;
            lexer->current += 1;
            c = *lexer->current;
            while (c >= '0' && c <= '7') {
                token->len++;
                if (lexer->current+1 >= lexer->end) break;
                c = *(++lexer->current);
            }
            token->subtype = LEXER_TOKEN_OCT | LEXER_TOKEN_INT;
        }
    } else {
        /* decimal or floating point number */
        dot = 0;
        token->str = lexer->current;
        while (1) {
            if (c >= '0' && c <= '9') {
            } else if (c == '.') dot++;
            else break;
            token->len++;
            if (lexer->current+1 >= lexer->end) break;
            c = *(++lexer->current);
        }
        if (c == 'e' && dot == 0)
            dot++; /* scientific notation */
        if (dot) {
            token->subtype = LEXER_TOKEN_DEC | LEXER_TOKEN_FLOAT;
            if (c == 'e') {
                if (lexer->current+1 >= lexer->end)
                    return 0;

                token->len++;
                c = *(++lexer->current);
                if (c == '-' || c == '+') {
                    token->len++;
                    if (lexer->current+1 >= lexer->end)
                        return 0;
                    c = *(++lexer->current);
                }
                while (c >= '0' && c <= '9') {
                    if (lexer->current+1 >= lexer->end) break;
                    c = *(++lexer->current);
                    token->len++;
                }
            } else if (c == '#') {
                /* floating point exception */
                char n;
                c2 = 4;
                if (lexer_check_str(lexer, "INF", 3))
                    token->subtype  |= LEXER_TOKEN_INFINITE;
                else if (lexer_check_str(lexer, "IND", 3))
                    token->subtype  |= LEXER_TOKEN_INDEFINITE;
                else if (lexer_check_str(lexer, "NAN", 3))
                    token->subtype  |= LEXER_TOKEN_NAN;
                else if (lexer_check_str(lexer, "QNAN", 4)) {
                    token->subtype  |= LEXER_TOKEN_NAN; c2++;
                } else if (lexer_check_str(lexer, "SNAN", 4)) {
                    token->subtype  |= LEXER_TOKEN_NAN; c2++;
                }
                for (n = 0; n < c2; ++n) {
                    if (lexer->current+1 >= lexer->end) break;
                    c = *(++lexer->current);
                    token->len++;
                }
                while (c >= '0' && c <= '9') {
                    if (lexer->current+1 >= lexer->end) break;
                    c = *(++lexer->current);
                    token->len++;
                }
            }
        } else {
            token->subtype = LEXER_TOKEN_DEC | LEXER_TOKEN_INT;
        }
    }

    if (token->subtype & LEXER_TOKEN_FLOAT) {
        /* float point precision subtype */
        if (c > ' ') {
            if (c == 'f' || c == 'F') {
                token->subtype |= LEXER_TOKEN_SINGLE_PREC;
                lexer->current++;
            } else token->subtype |= LEXER_TOKEN_DOUBLE_PREC;
        } else token->subtype |= LEXER_TOKEN_DOUBLE_PREC;
    } else if (token->subtype & LEXER_TOKEN_INT) {
        /* integer subtype */
        if (c > ' '){
            for (i = 0; i < 2; ++i) {
                if (c == 'l' || c == 'L')
                    token->subtype |= LEXER_TOKEN_LONG;
                else if (c == 'u' || c == 'U')
                    token->subtype |= LEXER_TOKEN_UNSIGNED;
                else break;
                if (lexer->current+1 >= lexer->end) break;
                c = *(++lexer->current);
            }
        }
    }
    return 1;
}

LEXER_INTERN int
lexer_read_punctuation(struct lexer *lexer, struct lexer_token *token)
{
    const struct lexer_punctuation *punc;
    int l, i;
    const char *p;

    token->len = 0;
    token->str = lexer->current;
    for (i = 0; lexer->puncts[i].string; ++i) {
        punc = &lexer->puncts[i];
        p = punc->string;
        for (l = 0; p[l] && lexer->current < lexer->end && lexer->current[l]; ++l) {
            if (lexer->current[l] != p[l])
                break;
        }
        if (!p[l]) {
            token->len += (lexer_size)l;
            lexer->current += l;
            token->type = LEXER_TOKEN_PUNCTUATION;
            token->subtype = (unsigned int)punc->id;
            return 1;
        }
    }
    return 0;
}

LEXER_API int
lexer_read(struct lexer *lexer, struct lexer_token *token)
{
    int c;
    if (!lexer->current) return 0;
    if (lexer->current >= lexer->end) return 0;
    if (lexer->error == 1) return 0;

    lexer_zero_struct(*token);
    lexer->last = lexer->current;
    lexer->last_line = lexer->line;
    lexer->error = 0;
    if (!lexer_read_white_space(lexer, 0))
        return 0;

    token->line = lexer->line;
    token->line_crossed = (lexer->line - lexer->last_line) ? 1 : 0;

    c = *lexer->current;
    if ((c >= '0' && c <= '9') ||
        (c == '.' && (*(lexer->current + 1)) >= '0' &&
        (c == '.' && (*(lexer->current + 1)) <= '9'))) {
        if (!lexer_read_number(lexer, token))
            return 0;
    } else if (c == '\"' || c == '\'') {
        if (!lexer_read_string(lexer, token, c))
            return 0;
    } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        if (!lexer_read_name(lexer, token))
            return 0;
    } else if ((c == '/' || c == '\\') || c == '.') {
        if (!lexer_read_name(lexer, token))
            return 0;
    } else if (!lexer_read_punctuation(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                "unkown punctuation: %c", c);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

LEXER_API int
lexer_read_on_line(struct lexer *lexer, struct lexer_token *token)
{
    struct lexer_token tok;
    if (!lexer_read(lexer, &tok)) {
        lexer->current = lexer->last;
        lexer->line = lexer->last_line;
    }
    if (!tok.line_crossed) {
        *token = tok;
        return 1;
    }
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    lexer_token_clear(token);
    return 0;
}

LEXER_API int
lexer_expect_string(struct lexer *lexer, const char *string)
{
    struct lexer_token token;
    if (!lexer_read(lexer, &token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                "failed to read expected token: %s", string);
        }
        lexer->error = 1;
        return 0;
    }
    if (lexer_token_cmp(&token, string)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                "read token is not expected string: %s", string);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

LEXER_API int
lexer_expect_type(struct lexer *lexer, enum lexer_token_type type,
    unsigned int subtype, struct lexer_token *token)
{
    if (!lexer_read(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                "could not read expected token with type: %d", type);
        }
        lexer->error = 1;
        return 0;
    }
    if (token->type != type) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                "read token has type %d instead of expected type: %d", token->type, type);
        }
        lexer->error = 1;
        return 0;
    }
    if ((token->subtype & subtype) != subtype) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                "read token has subtype %d instead of expected subtype %d", token->subtype, type);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

LEXER_API int
lexer_expect_any(struct lexer *lexer, struct lexer_token *token)
{
    if (!lexer_read(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                "could not read any expected token");
        }
        return 0;
    }
    return 1;
}

LEXER_API int
lexer_check_string(struct lexer *lexer, const char *string)
{
    struct lexer_token token;
    if (!lexer_read(lexer, &token))
        return 0;
    if (!lexer_token_cmp(&token, string))
        return 1;

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    return 0;
}

LEXER_API int
lexer_check_type(struct lexer *lexer, enum lexer_token_type type,
    unsigned int subtype, struct lexer_token *token)
{
    struct lexer_token tok;
    if (!lexer_read(lexer, &tok))
        return 0;
    if (tok.type == type && (tok.subtype & subtype) == subtype) {
        *token = tok;
        return 1;
    }

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    return 0;
}

LEXER_API int
lexer_peek_string(struct lexer *lexer, const char *string)
{
    struct lexer_token tok;
    if (!lexer_read(lexer, &tok))
        return 0;

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    if (!lexer_token_cmp(&tok, string))
        return 1;
    return 0;
}

LEXER_API int
lexer_peek_type(struct lexer *lexer, enum lexer_token_type type,
    unsigned int subtype, struct lexer_token *token)
{
    struct lexer_token tok;
    if (!lexer_read(lexer, &tok))
        return 0;

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;

    if (tok.type == type && (tok.subtype & subtype) == subtype) {
        *token = tok;
        return 1;
    }
    return 0;
}

LEXER_API int
lexer_skip_until(struct lexer *lexer, const char *string)
{
    struct lexer_token tok;
    while (lexer_read(lexer, &tok)) {
        if (!lexer_token_cmp(&tok, string))
            return 1;
    }
    return 0;
}

LEXER_API int
lexer_skip_line(struct lexer *lexer)
{
    struct lexer_token tok;
    while (lexer_read(lexer, &tok)) {
        if (tok.line_crossed) {
            lexer->current = lexer->last;
            lexer->line = lexer->last_line;
            return 1;
        }
    }
    return 0;
}

LEXER_API int
lexer_parse_int(struct lexer *lexer)
{
    struct lexer_token tok;
    if (!lexer_read(lexer, &tok))
        return 0;
    if (tok.type == LEXER_TOKEN_PUNCTUATION && tok.str[0] == '-') {
        lexer_expect_type(lexer, LEXER_TOKEN_NUMBER, LEXER_TOKEN_INT, &tok);
        return -lexer_token_to_int(&tok);
    } else if (tok.type != LEXER_TOKEN_NUMBER || tok.subtype == LEXER_TOKEN_FLOAT) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                "expected int value but found float");
        }
        lexer->error = 1;
    }
    return lexer_token_to_int(&tok);
}

LEXER_API int
lexer_parse_bool(struct lexer *lexer)
{
    struct lexer_token tok;
    if (!lexer_expect_type(lexer, LEXER_TOKEN_NUMBER, 0, &tok)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                "could not read expected boolean");
        }
        lexer->error = 1;
        return 0;
    }
    return (lexer_token_to_int(&tok) != 0);
}

LEXER_API float
lexer_parse_float(struct lexer *lexer)
{
    struct lexer_token tok;
    if (!lexer_read(lexer, &tok)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                "could not read expected float number");
        }
        lexer->error = 1;
        return 0;
    }
    if (tok.type == LEXER_TOKEN_PUNCTUATION && tok.str[0] == '-') {
        lexer_expect_type(lexer, LEXER_TOKEN_NUMBER, 0, &tok);
        return -(float)tok.value.f;
    } else if (tok.type != LEXER_TOKEN_NUMBER) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LEXER_ERROR, lexer->line,
                "expected float number is not a number");
        }
        lexer->error = 1;
        return 0;
    }
    return lexer_token_to_float(&tok);
}

#endif /* LEXER_IMPLEMENTATION */
