/*
    mm_lexer.h - zlib - Micha Mettke

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
    #define MM_LEXER_IMPLEMENTATION
    before you include this file in *one* C or C++ file to create the implementation

    If you want to keep the implementation in that file you have to do
    #define MM_LEXER_STATIC before including this file

    If you want to use asserts to add validation add
    #define MM_LEXER_ASSERT before including this file

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
    MM_LEXER_IMPLEMENTATION
        Generates the implementation of the library into the included file.
        If not provided the library is in header only mode and can be included
        in other headers or source files without problems. But only ONE file
        should hold the implementation.

    MM_LEXER_STATIC
        The generated implementation will stay private inside implementation
        file and all internal symbols and functions will only be visible inside
        that file.

    MM_LEXER_ASSERT
    MM_LEXER_USE_ASSERT
        If you define MM_LEXER_USE_ASSERT without defining MM_ASSERT mm_lexer.h
        will use assert.h and assert(). Otherwise it will use your assert
        method. If you do not define MM_LEXER_USE_ASSERT no additional checks
        will be added. This is the only C standard library function used
        by mm_lexer.

    MM_LEXER_SIZE_TYPE
        You can define this to 'size_t' if you use the standard library,
        otherwise it needs to be able to hold the maximum addressable memory
        space. If you do not define this it will default to unsigned long.

    MM_LEXER_MEMSET
        You can define this to 'memset' or your own memset replacement.
        If not mm_lexer uses a naive (maybe inefficent) implementation.


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
    struct mm_lexer_context lexer;
    mm_lexer_init(&lexer, text, text_length, NULL, test_log, NULL);

    /* parse tokens */
    mm_lexer_read(&lexer, &tok);
    mm_lexer_expect_string(&lexer, "string");
    mm_lexer_expect_type(&lexer, LXR_TOKEN_NUMBER, LXR_TOKEN_HEX, &tok);
    mm_lexer_expect_any(&lexer, &tok);

    /* check and parse only if correct */
    if (mm_lexer_check_string(&lexer, "string"))
        /* correct string  */
    if (mm_lexer_check_type(&lexer, LXR_TOKEN_NUMBER, LXR_TOKEN_BIN, &tok))
        /* correct type */

    /* only check but don't parse */
    if (mm_lexer_peek_string(&lexer, "string"))
        /* correct string  */
    if (mm_lexer_peek_type(&lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_DOLLAR, &tok))
        /* correct type */

    /* token compare function */
    if (!mm_lexer_token_cmp(&tok, "string"))
        /* token holds string 'string' */
    if (!mm_lexer_token_icmp(&tok, "string"))
        /* token holds case independent string 'string' */

    /* copy token content into buffer */
    char buffer[1024]
    mm_lexer_token_cpy(buffer, 1024, &tok);

    /* token to number conversion */
    /* You should always check if the token (sub)type is correct */
    int i = mm_lexer_token_to_int(&tok);
    float f = mm_lexer_token_to_float(&tok);
    double d = mm_lexer_token_to_double(&tok);
    unsigned long ul = mm_lexer_token_to_unsigned_long(&tok);
#endif

 /* ===============================================================
 *
 *                          HEADER
 *
 * =============================================================== */
#ifndef MM_LEXER_H_
#define MM_LEXER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MM_LEXER_STATIC
#define MM_LEXER_API static
#else
#define MM_LEXER_API extern
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 19901L)
#include <stdint.h>
#ifndef MM_LEXER_SIZE_TYPE
#define MM_LEXER_SIZE_TYPE uintptr_t
#endif
#else
#ifndef MM_LEXER_SIZE_TYPE
#define MM_LEXER_SIZE_TYPE unsigned long
#endif
#endif
typedef unsigned char mm_lexer_byte;
typedef MM_LEXER_SIZE_TYPE mm_lexer_size;

/* ---------------------------------------------------------------
 *                      PUNCTUATION
 * ---------------------------------------------------------------*/
/* This is the default punctuation table to convert from string to
 * an identifier. You can create your own punctuation table
 * and use your custom punctuation parsing scheme.
 * IMPORTANT: the list has to be ordered by string length */
#define MM_LEXER_DEFAULT_PUNCTION_MAP(PUNCTUATION)\
    PUNCTUATION(">>=",  MM_LEXER_PUNCT_RSHIFT_ASSIGN)\
    PUNCTUATION("<<=",  MM_LEXER_PUNCT_LSHIFT_ASSIGN)\
    PUNCTUATION("...",  MM_LEXER_PUNCT_PARAMS)\
    PUNCTUATION("&&",   MM_LEXER_PUNCT_LOGIC_AND)\
    PUNCTUATION("||",   MM_LEXER_PUNCT_LOGIC_OR)\
    PUNCTUATION(">=",   MM_LEXER_PUNCT_LOGIC_GEQ)\
    PUNCTUATION("<=",   MM_LEXER_PUNCT_LOGIC_LEQ)\
    PUNCTUATION("==",   MM_LEXER_PUNCT_LOGIC_EQ)\
    PUNCTUATION("!=",   MM_LEXER_PUNCT_LOGIC_UNEQ)\
    PUNCTUATION("*=",   MM_LEXER_PUNCT_MUL_ASSIGN)\
    PUNCTUATION("/=",   MM_LEXER_PUNCT_DIV_ASSIGN)\
    PUNCTUATION("%=",   MM_LEXER_PUNCT_MOD_ASSIGN)\
    PUNCTUATION("+=",   MM_LEXER_PUNCT_ADD_ASSIGN)\
    PUNCTUATION("-=",   MM_LEXER_PUNCT_SUB_ASSIGN)\
    PUNCTUATION("++",   MM_LEXER_PUNCT_INC)\
    PUNCTUATION("--",   MM_LEXER_PUNCT_DEC)\
    PUNCTUATION("&=",   MM_LEXER_PUNCT_BIN_AND_ASSIGN)\
    PUNCTUATION("|=",   MM_LEXER_PUNCT_BIN_OR_ASSIGN)\
    PUNCTUATION("^=",   MM_LEXER_PUNCT_BIN_XOR_ASSIGN)\
    PUNCTUATION(">>",   MM_LEXER_PUNCT_RSHIFT)\
    PUNCTUATION("<<",   MM_LEXER_PUNCT_LSHIFT)\
    PUNCTUATION("->",   MM_LEXER_PUNCT_POINTER)\
    PUNCTUATION("::",   MM_LEXER_PUNCT_CPP1)\
    PUNCTUATION(".*",   MM_LEXER_PUNCT_CPP2)\
    PUNCTUATION("*",    MM_LEXER_PUNCT_MUL)\
    PUNCTUATION("/",    MM_LEXER_PUNCT_DIV)\
    PUNCTUATION("%",    MM_LEXER_PUNCT_MOD)\
    PUNCTUATION("+",    MM_LEXER_PUNCT_ADD)\
    PUNCTUATION("-",    MM_LEXER_PUNCT_SUB)\
    PUNCTUATION("=",    MM_LEXER_PUNCT_ASSIGN)\
    PUNCTUATION("&",    MM_LEXER_PUNCT_BIN_AND)\
    PUNCTUATION("|",    MM_LEXER_PUNCT_BIN_OR)\
    PUNCTUATION("^",    MM_LEXER_PUNCT_BIN_XOR)\
    PUNCTUATION("~",    MM_LEXER_PUNCT_BIN_NOT)\
    PUNCTUATION("!",    MM_LEXER_PUNCT_LOGIC_NOT)\
    PUNCTUATION(">",    MM_LEXER_PUNCT_LOGIC_GREATER)\
    PUNCTUATION("<",    MM_LEXER_PUNCT_LOGIC_LESS)\
    PUNCTUATION(".",    MM_LEXER_PUNCT_REF)\
    PUNCTUATION(",",    MM_LEXER_PUNCT_COMMA)\
    PUNCTUATION(";",    MM_LEXER_PUNCT_SEMICOLON)\
    PUNCTUATION(":",    MM_LEXER_PUNCT_COLON)\
    PUNCTUATION("?",    MM_LEXER_PUNCT_QUESTIONMARK)\
    PUNCTUATION("(",    MM_LEXER_PUNCT_PARENTHESE_OPEN)\
    PUNCTUATION(")",    MM_LEXER_PUNCT_PARENTHESE_CLOSE)\
    PUNCTUATION("{",    MM_LEXER_PUNCT_BRACE_OPEN)\
    PUNCTUATION("}",    MM_LEXER_PUNCT_BRACE_CLOSE)\
    PUNCTUATION("[",    MM_LEXER_PUNCT_BRACKET_OPEN)\
    PUNCTUATION("]",    MM_LEXER_PUNCT_BRACKET_CLOSE)\
    PUNCTUATION("\\",   MM_LEXER_PUNCT_BACKSLASH)\
    PUNCTUATION("#",    MM_LEXER_PUNCT_PRECOMPILER)\
    PUNCTUATION("$",    MM_LEXER_PUNCT_DOLLAR)

enum mm_lexer_default_punctuation_ids {
#define PUNCTUATION(chars, id) id,
    MM_LEXER_DEFAULT_PUNCTION_MAP(PUNCTUATION)
#undef PUNCTUATION
    MM_LEXER_PUNCT_MAX
};

struct mm_lexer_punctuation {
    const char *string;
    /* string representation of the punctuation */
    int id;
    /* number identitfier to stored inside the token flag */
};

/* ---------------------------------------------------------------
 *                          TOKEN
 * ---------------------------------------------------------------*/
enum mm_lexer_token_type {
    MM_LEXER_TOKEN_STRING,
    /* strings literal: "string" */
    MM_LEXER_TOKEN_LITERAL,
    /* character literal: 'c' */
    MM_LEXER_TOKEN_NUMBER,
    /* integer or floating pointer number */
    MM_LEXER_TOKEN_NAME,
    /* names and keyword */
    MM_LEXER_TOKEN_PUNCTUATION
    /* punctuation from the punctuation table */
};

/* token subtype flags */
enum mm_lexer_token_flags {
    MM_LEXER_TOKEN_INT           = 0x00001,/* integer type */
    MM_LEXER_TOKEN_DEC           = 0x00002,/* decimal number */
    MM_LEXER_TOKEN_HEX           = 0x00004,/* hexadecimal number */
    MM_LEXER_TOKEN_OCT           = 0x00008,/* octal number */
    MM_LEXER_TOKEN_BIN           = 0x00010,/* binary number */
    MM_LEXER_TOKEN_LONG          = 0x00020,/* long integer number */
    MM_LEXER_TOKEN_UNSIGNED      = 0x00040,/* unsigned number */
    MM_LEXER_TOKEN_FLOAT         = 0x00080,/* floating pointer number */
    MM_LEXER_TOKEN_SINGLE_PREC   = 0x00100,/* single precision float */
    MM_LEXER_TOKEN_DOUBLE_PREC   = 0x00200,/* double precision float */
    MM_LEXER_TOKEN_INFINITE      = 0x00400,/* infinit float number */
    MM_LEXER_TOKEN_INDEFINITE    = 0x00800,/* indefinite float number */
    MM_LEXER_TOKEN_NAN           = 0x01000,/* Not a number float */
    MM_LEXER_TOKEN_VALIDVAL      = 0x02000 /* flag if the token is a number */
};

struct mm_lexer_token {
/*  A Token represents a section in a string containing a substring.
    Since the token only references the text, no actual memory needs
    to be allocated from the library to hold each token. Therefore the library does not
    represent a classical datastructure holding a string, instead
    it is only responsible for referencing the string.
    In addition it is to note that the string
    inside of the token is not terminated with '\0' since it would require write
    access to the text which is not always wanted.*/
    enum mm_lexer_token_type type;
    /* main type of the token */
    unsigned int subtype;
    /* subtype flags of the token */
    mm_lexer_size line;
    /* text line the token was read from */
    int line_crossed;
    /* flag indicating if the token spans over multible lines */
    struct {unsigned long i; double f;} value;
    /* number representation of the token */
    const char *str;
    /* text pointer to the beginning of the token inside the text */
    mm_lexer_size len;
    /* byte length of the token */
};

MM_LEXER_API mm_lexer_size mm_lexer_token_cpy(char*, mm_lexer_size max, const struct mm_lexer_token*);
/*  this function copies the content of a token into a buffer
    Input:
    - buffer to copy the string into
    - maximum number of character to copy into the buffer
    - token to copy the string from
    Output:
    - length of the copied string
*/
MM_LEXER_API int mm_lexer_token_cmp(const struct mm_lexer_token*, const char*);
/*  this function compares the token content with a user string
    Input:
    - token to check the string against
    - string to check
    Output:
    - if equal will return 1 otherwise 0 (same as strcmp)
*/
MM_LEXER_API int mm_lexer_token_icmp(const struct mm_lexer_token*, const char*);
/*  this function makes a case insensitive compare of the
 *  token content with a user string.
    Input:
    - token to check the string against
    - string to check
    Output:
    - if equal will return 1 otherwise 0 (same as strcmp)
*/
MM_LEXER_API int mm_lexer_token_to_int(struct mm_lexer_token*);
/*  this function converts the token content into an int
    Input:
    - token to convert into an integer
    Output:
    - converted integer value if token is a number or 0 otherwise
*/
MM_LEXER_API float mm_lexer_token_to_float(struct mm_lexer_token*);
/*  this function converts the token content into an float
    Input:
    - token to convert into an floating point number
    Output:
    - converted float value if token is a number or 0.0f otherwise
*/
MM_LEXER_API double mm_lexer_token_to_double(struct mm_lexer_token*);
/*  this function converts the token content into an double
    Input:
    - token to convert into an double floating point number
    Output:
    - converted double value if token is a number or 0.0 otherwise
*/
MM_LEXER_API unsigned long mm_lexer_token_to_unsigned_long(struct mm_lexer_token*);
/*  this function converts the token content into an unsigned long integer
    Input:
    - token to convert into an unsigned long integer number
    Output:
    - converted unsigned long value if token is a number or 0 otherwise
*/

/* ---------------------------------------------------------------
 *                          LEXER
 * ---------------------------------------------------------------*/
/* logging callback */
enum mm_lexer_log_level {MM_LEXER_WARNING,MM_LEXER_ERROR};
typedef void(*mm_lexer_log_f)(void*, enum mm_lexer_log_level, mm_lexer_size line, const char *msg, ...);

struct mm_lexer {
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
    mm_lexer_size length;
    /* length of the text to parse */
    mm_lexer_size line;
    /* current line the text */
    mm_lexer_size last_line;
    /* last parsed line */
    const struct mm_lexer_punctuation *puncts;
    /* internally used punctuation table */
    mm_lexer_log_f log;
    /* logging callback for outputing error messages */
    void *userdata;
    /* userdata passed to the logging callback */
};

MM_LEXER_API void mm_lexer_init(struct mm_lexer *lexer, const char *ptr, mm_lexer_size len,
                    const struct mm_lexer_punctuation *punct, mm_lexer_log_f log, void *usr);
/*  this function initializes the lexer
    Input:
    - pointer to a text buffer to parse
    - length of the text buffer
    - custom punctuation table or NULL for default table
    - logging callback or NULL if not needed
    - userdata passed into the callback or NULL if not needed
*/
MM_LEXER_API void mm_lexer_reset(struct mm_lexer*);
/*  this function resets the lexer back to beginning */
MM_LEXER_API int mm_lexer_read(struct mm_lexer*, struct mm_lexer_token*);
/*  this function reads a token from a loaded lexer
    Input:
    - token to hold the parsed content information
    Output:
    - if successfully 1 or 0 otherwise
*/
MM_LEXER_API int mm_lexer_read_on_line(struct mm_lexer*, struct mm_lexer_token*);
/*  this function reads a token from a loaded lexer only if on the same line
    Input:
    - token to hold the parsed content information
    Output:
    - if successfully read 1 or 0 otherwise
*/
MM_LEXER_API void mm_lexer_unread(struct mm_lexer*, struct mm_lexer_token*);
/*  this function pushes back a token (limitied to exactly one token)
    Input:
    - token push back into the lexer
*/
MM_LEXER_API int mm_lexer_expect_string(struct mm_lexer*, const char*);
/*  this function reads a token and check the token content. If the token
    content is not equal to the provided string a error occurs.
    Input:
    - the expected parsed token string
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MM_LEXER_API int mm_lexer_expect_type(struct mm_lexer*, enum mm_lexer_token_type type,
                                        unsigned int subtype, struct mm_lexer_token*);
/*  this function reads a token and checks for token type + subtype. If the token
    type and subtype or not correct an error will be raised.
    Input:
    - expected token type
    - expected token subtype
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MM_LEXER_API int mm_lexer_expect_any(struct mm_lexer*, struct mm_lexer_token*);
/*  this function tries to read in a token and if not possible will raise and error
    Input:
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MM_LEXER_API int mm_lexer_check_string(struct mm_lexer*, const char*);
/*  this function tries to read in a token holding with given content.
 *  If it succeeds the token will be returned. If not the read token will be unread.
    Input:
    - the expected parsed token string
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MM_LEXER_API int mm_lexer_check_type(struct mm_lexer*, enum mm_lexer_token_type type,
                                unsigned int subtype, struct mm_lexer_token*);
/*  this function tries to read in a token holding with token type and subtype.
 *  If it succeeds the token will be returned. If not the read token will be unread.
    Input:
    - expected token type
    - expected token subtype
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MM_LEXER_API int mm_lexer_peek_string(struct mm_lexer*, const char*);
/*  this function checks the next token for the given string content
    Input:
    - the expected parsed token string
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MM_LEXER_API int mm_lexer_peek_type(struct mm_lexer*, enum mm_lexer_token_type type,
                                    unsigned int subtype, struct mm_lexer_token*);
/*  this function checks the next token for the given type and subtype
    Input:
    - expected token type
    - expected token subtype
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MM_LEXER_API int mm_lexer_skip_until(struct mm_lexer*, const char*);
/*  this function skips all tokens until a token holding a certain string
    Input:
    - a expected string the parse should skip to
    Output:
    - 1 if successful, 0 otherwise
*/
MM_LEXER_API int mm_lexer_skip_line(struct mm_lexer*);
/*  this function skips the current line */
MM_LEXER_API int mm_lexer_parse_int(struct mm_lexer*);
/*  this function reads in a token and tries to convert it into an integer.
    If the conversion fails an error will be raised.
    Output:
    - parsed integer value
*/
MM_LEXER_API int mm_lexer_parse_bool(struct mm_lexer*);
/*  this function reads in a token and tries to convert it into an boolean.
    If the conversion fails an error will be raised.
    Output:
    - parsed boolean value
*/
MM_LEXER_API float mm_lexer_parse_float(struct mm_lexer*);
/*  this function reads in a token and tries to convert it into an float.
    If the conversion fails an error will be raised.
    Output:
    - parsed floating point value
*/
#ifdef __cplusplus
}
#endif
#endif /* MM_LEXER_H_ */

/* ===============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================*/
#ifdef MM_LEXER_IMPLEMENTATION

#define MM_LEXER_LEN(a) (sizeof(a)/sizeof((a)[0]))
#define MM_LEXER_UNUSED(a) ((void)(a))

#ifdef MM_LEXER_USE_ASSERT
#ifndef MM_LEXER_ASSERT
#include <assert.h>
#define MM_LEXER_ASSERT(expr) assert(expr)
#endif
#else
#define MM_LEXER_ASSERT(expr)
#endif

/* Pointer to Integer type conversion for pointer alignment */
#if defined(__PTRDIFF_TYPE__) /* This case should work for GCC*/
# define MM_LEXER_UINT_TO_PTR(x) ((void*)(__PTRDIFF_TYPE__)(x))
# define MM_LEXER_PTR_TO_UINT(x) ((mm_lexer_size)(__PTRDIFF_TYPE__)(x))
#elif !defined(__GNUC__) /* works for compilers other than LLVM */
# define MM_LEXER_UINT_TO_PTR(x) ((void*)&((char*)0)[x])
# define MM_LEXER_PTR_TO_UINT(x) ((wby_size)(((char*)x)-(char*)0))
#elif defined(MM_LEXER_USE_FIXED_TYPES) /* used if we have <stdint.h> */
# define MM_LEXER_UINT_TO_PTR(x) ((void*)(uintptr_t)(x))
# define MM_LEXER_PTR_TO_UINT(x) ((uintptr_t)(x))
#else /* generates warning but works */
# define MM_LEXER_UINT_TO_PTR(x) ((void*)(x))
# define MM_LEXER_PTR_TO_UINT(x) ((wby_size)(x))
#endif

#define MM_LEXER_INTERN static
#define MM_LEXER_GLOBAL static
#define MM_LEXER_STORAGE static

#ifndef MM_LEXER_MEMSET
#define MM_LEXER_MEMSET mm_lexer_memset
#endif

/* library intern default punctuation map */
MM_LEXER_GLOBAL const struct mm_lexer_punctuation
mm_lexer_default_punctuations[] = {
#define PUNCTUATION(chars, id) {chars, id},
    MM_LEXER_DEFAULT_PUNCTION_MAP(PUNCTUATION)
#undef PUNCTUATION
    {0, 0}
};
/* ---------------------------------------------------------------
 *                          UTIL
 * ---------------------------------------------------------------*/
MM_LEXER_INTERN char
mm_lexer_char_upper(char c)
{
    if (c >= 'a' && c <= 'z')
        return (char)('A' + (c - 'a'));
    return c;
}

MM_LEXER_INTERN void
mm_lexer_memset(void *ptr, int c0, mm_lexer_size size)
{
    #define word unsigned
    #define wsize sizeof(word)
    #define wmask (wsize - 1)
    unsigned char *dst = (unsigned char*)ptr;
    unsigned c = 0;
    mm_lexer_size t = 0;

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
    if ((t = MM_LEXER_PTR_TO_UINT(dst) & wmask) != 0) {
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

#define mm_lexer_zero_struct(s) mm_lexer_zero_size(&s, sizeof(s))
#define mm_lexer_zero_array(p,n) mm_lexer_zero_size(p, (n) * sizeof((p)[0]))
MM_LEXER_INTERN void
mm_lexer_zero_size(void *ptr, mm_lexer_size size)
{
    MM_LEXER_MEMSET(ptr, 0, size);
}

/* ---------------------------------------------------------------
 *                          TOKEN
 * ---------------------------------------------------------------*/
MM_LEXER_INTERN void
mm_lexer_token_clear(struct mm_lexer_token *tok)
{
    tok->line_crossed = 0;
}

MM_LEXER_API mm_lexer_size
mm_lexer_token_cpy(char *dst, mm_lexer_size max, const struct mm_lexer_token* tok)
{
    unsigned i = 0;
    mm_lexer_size ret;
    mm_lexer_size siz;

    MM_LEXER_ASSERT(dst);
    MM_LEXER_ASSERT(tok);
    if (!dst || !max || !tok)
        return 0;

    ret = (max < (tok->len + 1)) ? max : tok->len;
    siz = (max < (tok->len + 1)) ? max-1 : tok->len;
    for (i = 0; i < siz; i++)
        dst[i] = tok->str[i];
    dst[siz] = '\0';
    return ret;
}

MM_LEXER_API int
mm_lexer_token_icmp(const struct mm_lexer_token* tok, const char* str)
{
    mm_lexer_size i;
    MM_LEXER_ASSERT(tok);
    MM_LEXER_ASSERT(str);
    if (!tok || !str) return 1;
    for (i = 0; (i < tok->len && *str); i++, str++){
        if (mm_lexer_char_upper(tok->str[i]) != mm_lexer_char_upper(*str))
            return 1;
    }
    if (*str != 0 || i < tok->len)
        return 1;
    return 0;
}

MM_LEXER_API int
mm_lexer_token_cmp(const struct mm_lexer_token* tok, const char* str)
{
    mm_lexer_size i;
    MM_LEXER_ASSERT(tok);
    MM_LEXER_ASSERT(str);
    if (!tok || !str) return 1;
    for (i = 0; (i < tok->len && *str); i++, str++){
        if (tok->str[i] != *str)
            return 1;
    }
    if (*str != 0 || i < tok->len)
        return 1;
    return 0;
}

MM_LEXER_INTERN double
mm_lexer_token_parse_double(const char *p, mm_lexer_size length)
{
    int i, div, pow;
    double m;
    mm_lexer_size len = 0;
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

MM_LEXER_INTERN unsigned long
mm_lexer_token_parse_int(const char *p, mm_lexer_size length)
{
    unsigned long i = 0;
    mm_lexer_size len = 0;
    while (len < length) {
        i = i * 10 + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

MM_LEXER_INTERN unsigned long
mm_lexer_token_parse_oct(const char *p, mm_lexer_size length)
{
    unsigned long i = 0;
    mm_lexer_size len = 1;
    while (len < length) {
        i = (i << 3) + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

MM_LEXER_INTERN unsigned long
mm_lexer_token_parse_hex(const char *p, mm_lexer_size length)
{
    unsigned long i = 0;
    mm_lexer_size len = 2;
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

MM_LEXER_INTERN unsigned long
mm_lexer_token_parse_bin(const char *p, mm_lexer_size length)
{
    unsigned long i = 0;
    mm_lexer_size len = 2;
    while (len < length) {
        i = (i << 1) + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

MM_LEXER_INTERN void
mm_lexer_token_number_value(struct mm_lexer_token *tok)
{
    const char *p;
    MM_LEXER_ASSERT(tok->type == MM_LEXER_TOKEN_NUMBER);
    tok->value.i = 0;
    tok->value.f = 0;

    p = tok->str;
    if (tok->subtype & MM_LEXER_TOKEN_FLOAT) {
        if (tok->subtype & ((MM_LEXER_TOKEN_INFINITE|MM_LEXER_TOKEN_INDEFINITE|MM_LEXER_TOKEN_NAN))) {
            /* special real number constants */
            union {float f; unsigned int u;} convert;
            if (tok->subtype & MM_LEXER_TOKEN_INFINITE) {
                convert.u = 0x7f800000;
                tok->value.f = (double)convert.f;
            } else if (tok->subtype & MM_LEXER_TOKEN_INDEFINITE) {
                convert.u = 0xffc00000;
                tok->value.f = (double)convert.f;
            } else if (tok->subtype & MM_LEXER_TOKEN_NAN) {
                convert.u = 0x7fc00000;
                tok->value.f = (double)convert.f;
            }
        } else tok->value.f = mm_lexer_token_parse_double(tok->str, tok->len);
        tok->value.i = (unsigned long)tok->value.f;
    } else if (tok->subtype & MM_LEXER_TOKEN_DEC) {
        /* paser decimal number */
        tok->value.i = mm_lexer_token_parse_int(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & MM_LEXER_TOKEN_OCT) {
        /* parse octal number */
        tok->value.i = mm_lexer_token_parse_oct(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & MM_LEXER_TOKEN_HEX) {
        /* parse hex number */
        tok->value.i = mm_lexer_token_parse_hex(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & MM_LEXER_TOKEN_BIN) {
        /* parse binary number */
        tok->value.i = mm_lexer_token_parse_bin(p, tok->len);
        tok->value.f = (double)tok->value.i;
    }
    tok->subtype |= MM_LEXER_TOKEN_VALIDVAL;
}

MM_LEXER_API int
mm_lexer_token_to_int(struct mm_lexer_token *tok)
{
    return (int)mm_lexer_token_to_unsigned_long(tok);
}

MM_LEXER_API float
mm_lexer_token_to_float(struct mm_lexer_token *tok)
{
    double d = mm_lexer_token_to_double(tok);
    float f = (float)d;;
    return f;
}

MM_LEXER_API double
mm_lexer_token_to_double(struct mm_lexer_token *tok)
{
    if (tok->type != MM_LEXER_TOKEN_NUMBER)
        return 0.0;
    if (!(tok->subtype & MM_LEXER_TOKEN_VALIDVAL))
        mm_lexer_token_number_value(tok);
    if (!(tok->subtype & MM_LEXER_TOKEN_VALIDVAL))
        return 0.0;
    return tok->value.f;
}

MM_LEXER_API unsigned long
mm_lexer_token_to_unsigned_long(struct mm_lexer_token *tok)
{
    if (tok->type != MM_LEXER_TOKEN_NUMBER)
        return 0;
    if (!(tok->subtype & MM_LEXER_TOKEN_VALIDVAL))
        mm_lexer_token_number_value(tok);
    if (!(tok->subtype & MM_LEXER_TOKEN_VALIDVAL))
        return 0;
    return tok->value.i;
}

/* ---------------------------------------------------------------
 *                          LEXER
 * ---------------------------------------------------------------*/
MM_LEXER_API void
mm_lexer_init(struct mm_lexer *lexer, const char *ptr, mm_lexer_size len,
    const struct mm_lexer_punctuation *punct, mm_lexer_log_f log, void *userdata)
{
    mm_lexer_zero_struct(*lexer);
    lexer->buffer = ptr;
    lexer->current = ptr;
    lexer->last = ptr;
    lexer->end = ptr + len;
    lexer->length = len;
    lexer->line = lexer->last_line = 1;
    if (!punct)
        lexer->puncts = mm_lexer_default_punctuations;
    else lexer->puncts = punct;
    lexer->log = log;
    lexer->userdata = userdata;
}

MM_LEXER_API void
mm_lexer_reset(struct mm_lexer *lexer)
{
    lexer->current = lexer->buffer;
    lexer->last = lexer->buffer;
    lexer->line = lexer->last_line = 1;
}

MM_LEXER_INTERN int
mm_lexer_read_white_space(struct mm_lexer *lexer, int current_line)
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
            } else if ((*lexer->current + 1) == '*') {
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
                            lexer->log(lexer->userdata, MM_LEXER_WARNING, lexer->line,
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

MM_LEXER_INTERN int
mm_lexer_read_esc_chars(struct mm_lexer *lexer, char *ch)
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
                lexer->log(lexer->userdata, MM_LEXER_WARNING, lexer->line,
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
                lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
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
                lexer->log(lexer->userdata, MM_LEXER_WARNING, lexer->line,
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

MM_LEXER_INTERN int
mm_lexer_read_string(struct mm_lexer *lexer, struct mm_lexer_token *token, int quote)
{
    mm_lexer_size tmpline;
    const char *tmp;
    char ch;
    if (quote == '\"')
        token->type = MM_LEXER_TOKEN_STRING;
    else token->type = MM_LEXER_TOKEN_LITERAL;
    lexer->current++;
    if (lexer->current >= lexer->end)
        return 0;

    token->len = 0;
    token->str = lexer->current;
    while (1) {
        if (*lexer->current == '\\') {
            if (!mm_lexer_read_esc_chars(lexer, &ch))
                return 0;
        } else if (*lexer->current == quote) {
            lexer->current++;
            if (lexer->current >= lexer->end)
                return 0;

            tmp = lexer->current;
            tmpline = lexer->line;
            if (!mm_lexer_read_white_space(lexer, 0)) {
                lexer->current = tmp;
                lexer->line = tmpline;
                break;
            }
            if (*lexer->current == '\0') {
                if (lexer->log) {
                        lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                            "expecting string after '\' terminated line");
                }
                lexer->error = 1;
                return 0;
            }
            break;
        } else {
            if (*lexer->current == '\0') {
                if (lexer->log) {
                    lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                        "missing trailing quote");
                }
                lexer->error = 1;
                return 0;
            }
            if (*lexer->current == '\n') {
                if (lexer->log) {
                    lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                        "newline inside string");
                }
                lexer->error = 1;
                return 0;
            }
            lexer->current++;
        }
    }
    if (token->str) {
        token->len = (mm_lexer_size)(lexer->current - token->str) - 1;
        if (token->type == MM_LEXER_TOKEN_LITERAL)
            token->subtype = (unsigned int)token->str[0];
        else token->subtype = (unsigned int)token->len;
    }
    return 1;
}

MM_LEXER_INTERN int
mm_lexer_read_name(struct mm_lexer *lexer, struct mm_lexer_token *token)
{
    char c;
    token->type = MM_LEXER_TOKEN_NAME;
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

MM_LEXER_INTERN int
mm_lexer_check_str(const struct mm_lexer *lexer, const char *str, mm_lexer_size len)
{
    mm_lexer_size i;
    for (i = 0; i < len && (&lexer->current[i] < lexer->end); ++i) {
        if (lexer->current[i] != str[i])
            return 0;
    }
    if (i < len) return 0;
    return 1;
}

MM_LEXER_INTERN int
mm_lexer_read_number(struct mm_lexer *lexer, struct mm_lexer_token *token)
{
    mm_lexer_size i;
    int dot;
    char c, c2;

    token->type = MM_LEXER_TOKEN_NUMBER;
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
            token->subtype = MM_LEXER_TOKEN_HEX | MM_LEXER_TOKEN_INT;
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
            token->subtype = MM_LEXER_TOKEN_BIN | MM_LEXER_TOKEN_INT;
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
            token->subtype = MM_LEXER_TOKEN_OCT | MM_LEXER_TOKEN_INT;
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
            token->subtype = MM_LEXER_TOKEN_DEC | MM_LEXER_TOKEN_FLOAT;
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
                if (mm_lexer_check_str(lexer, "INF", 3))
                    token->subtype  |= MM_LEXER_TOKEN_INFINITE;
                else if (mm_lexer_check_str(lexer, "IND", 3))
                    token->subtype  |= MM_LEXER_TOKEN_INDEFINITE;
                else if (mm_lexer_check_str(lexer, "NAN", 3))
                    token->subtype  |= MM_LEXER_TOKEN_NAN;
                else if (mm_lexer_check_str(lexer, "QNAN", 4)) {
                    token->subtype  |= MM_LEXER_TOKEN_NAN; c2++;
                } else if (mm_lexer_check_str(lexer, "SNAN", 4)) {
                    token->subtype  |= MM_LEXER_TOKEN_NAN; c2++;
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
            token->subtype = MM_LEXER_TOKEN_DEC | MM_LEXER_TOKEN_INT;
        }
    }

    if (token->subtype & MM_LEXER_TOKEN_FLOAT) {
        /* float point precision subtype */
        if (c > ' ') {
            if (c == 'f' || c == 'F') {
                token->subtype |= MM_LEXER_TOKEN_SINGLE_PREC;
                lexer->current++;
            } else token->subtype |= MM_LEXER_TOKEN_DOUBLE_PREC;
        } else token->subtype |= MM_LEXER_TOKEN_DOUBLE_PREC;
    } else if (token->subtype & MM_LEXER_TOKEN_INT) {
        /* integer subtype */
        if (c > ' '){
            for (i = 0; i < 2; ++i) {
                if (c == 'l' || c == 'L')
                    token->subtype |= MM_LEXER_TOKEN_LONG;
                else if (c == 'u' || c == 'U')
                    token->subtype |= MM_LEXER_TOKEN_UNSIGNED;
                else break;
                if (lexer->current+1 >= lexer->end) break;
                c = *(++lexer->current);
            }
        }
    }
    return 1;
}

MM_LEXER_INTERN int
mm_lexer_read_punctuation(struct mm_lexer *lexer, struct mm_lexer_token *token)
{
    const struct mm_lexer_punctuation *punc;
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
            token->len += (mm_lexer_size)l;
            lexer->current += l;
            token->type = MM_LEXER_TOKEN_PUNCTUATION;
            token->subtype = (unsigned int)punc->id;
            return 1;
        }
    }
    return 0;
}

MM_LEXER_API int
mm_lexer_read(struct mm_lexer *lexer, struct mm_lexer_token *token)
{
    int c;
    if (!lexer->current) return 0;
    if (lexer->current >= lexer->end) return 0;
    if (lexer->error == 1) return 0;

    mm_lexer_zero_struct(*token);
    lexer->last = lexer->current;
    lexer->last_line = lexer->line;
    lexer->error = 0;
    if (!mm_lexer_read_white_space(lexer, 0))
        return 0;

    token->line = lexer->line;
    token->line_crossed = (lexer->line - lexer->last_line) ? 1 : 0;

    c = *lexer->current;
    if ((c >= '0' && c <= '9') ||
        (c == '.' && (*(lexer->current + 1)) >= '0' &&
        (c == '.' && (*(lexer->current + 1)) <= '9'))) {
        if (!mm_lexer_read_number(lexer, token))
            return 0;
    } else if (c == '\"' || c == '\'') {
        if (!mm_lexer_read_string(lexer, token, c))
            return 0;
    } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        if (!mm_lexer_read_name(lexer, token))
            return 0;
    } else if ((c == '/' || c == '\\') || c == '.') {
        if (!mm_lexer_read_name(lexer, token))
            return 0;
    } else if (!mm_lexer_read_punctuation(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                "unkown punctuation: %c", c);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

MM_LEXER_API int
mm_lexer_read_on_line(struct mm_lexer *lexer, struct mm_lexer_token *token)
{
    struct mm_lexer_token tok;
    if (!mm_lexer_read(lexer, &tok)) {
        lexer->current = lexer->last;
        lexer->line = lexer->last_line;
    }
    if (!tok.line_crossed) {
        *token = tok;
        return 1;
    }
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    mm_lexer_token_clear(token);
    return 0;
}

MM_LEXER_API int
mm_lexer_expect_string(struct mm_lexer *lexer, const char *string)
{
    struct mm_lexer_token token;
    if (!mm_lexer_read(lexer, &token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                "failed to read expected token: %s", string);
        }
        lexer->error = 1;
        return 0;
    }
    if (mm_lexer_token_cmp(&token, string)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                "read token is not expected string: %s", string);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

MM_LEXER_API int
mm_lexer_expect_type(struct mm_lexer *lexer, enum mm_lexer_token_type type,
    unsigned int subtype, struct mm_lexer_token *token)
{
    if (!mm_lexer_read(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                "could not read expected token with type: %d", type);
        }
        lexer->error = 1;
        return 0;
    }
    if (token->type != type) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                "read token has type %s instead of expected type: %d", token->type, type);
        }
        lexer->error = 1;
        return 0;
    }
    if ((token->subtype & subtype) != subtype) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                "read token has subtype %d instead of expected subtype %d", token->subtype, type);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

MM_LEXER_API int
mm_lexer_expect_any(struct mm_lexer *lexer, struct mm_lexer_token *token)
{
    if (!mm_lexer_read(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                "could not read any expected token");
        }
        return 0;
    }
    return 1;
}

MM_LEXER_API int
mm_lexer_check_string(struct mm_lexer *lexer, const char *string)
{
    struct mm_lexer_token token;
    if (!mm_lexer_read(lexer, &token))
        return 0;
    if (!mm_lexer_token_cmp(&token, string))
        return 1;

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    return 0;
}

MM_LEXER_API int
mm_lexer_check_type(struct mm_lexer *lexer, enum mm_lexer_token_type type,
    unsigned int subtype, struct mm_lexer_token *token)
{
    struct mm_lexer_token tok;
    if (!mm_lexer_read(lexer, &tok))
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

MM_LEXER_API int
mm_lexer_peek_string(struct mm_lexer *lexer, const char *string)
{
    struct mm_lexer_token tok;
    if (!mm_lexer_read(lexer, &tok))
        return 0;

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    if (!mm_lexer_token_cmp(&tok, string))
        return 1;
    return 0;
}

MM_LEXER_API int
mm_lexer_peek_type(struct mm_lexer *lexer, enum mm_lexer_token_type type,
    unsigned int subtype, struct mm_lexer_token *token)
{
    struct mm_lexer_token tok;
    if (!mm_lexer_read(lexer, &tok))
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

MM_LEXER_API int
mm_lexer_skip_until(struct mm_lexer *lexer, const char *string)
{
    struct mm_lexer_token tok;
    while (mm_lexer_read(lexer, &tok)) {
        if (!mm_lexer_token_cmp(&tok, string))
            return 1;
    }
    return 0;
}

MM_LEXER_API int
mm_lexer_skip_line(struct mm_lexer *lexer)
{
    struct mm_lexer_token tok;
    while (mm_lexer_read(lexer, &tok)) {
        if (tok.line_crossed) {
            lexer->current = lexer->last;
            lexer->line = lexer->last_line;
            return 1;
        }
    }
    return 0;
}

MM_LEXER_API int
mm_lexer_parse_int(struct mm_lexer *lexer)
{
    struct mm_lexer_token tok;
    if (!mm_lexer_read(lexer, &tok))
        return 0;
    if (tok.type == MM_LEXER_TOKEN_PUNCTUATION && tok.str[0] == '-') {
        mm_lexer_expect_type(lexer, MM_LEXER_TOKEN_NUMBER, MM_LEXER_TOKEN_INT, &tok);
        return -mm_lexer_token_to_int(&tok);
    } else if (tok.type != MM_LEXER_TOKEN_NUMBER || tok.subtype == MM_LEXER_TOKEN_FLOAT) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                "expected int value but found float");
        }
        lexer->error = 1;
    }
    return mm_lexer_token_to_int(&tok);
}

MM_LEXER_API int
mm_lexer_parse_bool(struct mm_lexer *lexer)
{
    struct mm_lexer_token tok;
    if (!mm_lexer_expect_type(lexer, MM_LEXER_TOKEN_NUMBER, 0, &tok)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                "could not read expected boolean");
        }
        lexer->error = 1;
        return 0;
    }
    return (mm_lexer_token_to_int(&tok) != 0);
}

MM_LEXER_API float
mm_lexer_parse_float(struct mm_lexer *lexer)
{
    struct mm_lexer_token tok;
    if (!mm_lexer_read(lexer, &tok)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                "could not read expected float number");
        }
        lexer->error = 1;
        return 0;
    }
    if (tok.type == MM_LEXER_TOKEN_PUNCTUATION && tok.str[0] == '-') {
        mm_lexer_expect_type(lexer, MM_LEXER_TOKEN_NUMBER, 0, &tok);
        return -(float)tok.value.f;
    } else if (tok.type != MM_LEXER_TOKEN_NUMBER) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MM_LEXER_ERROR, lexer->line,
                "expected float number is not a number");
        }
        lexer->error = 1;
        return 0;
    }
    return mm_lexer_token_to_float(&tok);
}

#endif /* MM_LEXER_IMPLEMENTATION */
