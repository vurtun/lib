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
    #define MML_IMPLEMENTATION
    before you include this file in *one* C or C++ file to create the implementation

    If you want to keep the implementation in that file you have to do
    #define MML_STATIC before including this file

    If you want to use asserts to add validation add
    #define MML_ASSERT before including this file

LICENSE: (zlib)
    Copyright (c) 2015 Micha Mettke

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

USAGE:
    This file behaves differently depending on what symbols you define
    before including it.

    Header-File mode:
    If you do not define MML_IMPLEMENTATION before including this file, it
    will operate in header only mode. In this mode it declares all used structs
    and the API of the library without including the implementation of the library.

    Implementation mode:
    If you define MML_IMPLEMENTATIOn before including this file, it will
    compile the implementation of the JSON parser. To specify the visibility
    as private and limit all symbols inside the implementation file
    you can define MML_STATIC before including this file.
    Make sure that you only include this file implementation in *one* C or C++ file
    to prevent collisions.

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
        and unicode support.

    Error handling
        At the moment there is a error flag and a logging function to check for
        errors / warnings. But the overall handling is not very extensive.

EXAMPLES:*/
#if 0
    /* initialize lexer */
    struct mml_context lexer;
    mml_init(&lexer, text, text_length, NULL, test_log, NULL);

    /* parse tokens */
    mml_read(&lexer, &tok);
    mml_expect_string(&lexer, "string");
    mml_expect_type(&lexer, LXR_TOKEN_NUMBER, LXR_TOKEN_HEX, &tok);
    mml_expect_any(&lexer, &tok);

    /* check and parse only if correct */
    if (mml_check_string(&lexer, "string"))
        /* correct string  */
    if (mml_check_type(&lexer, LXR_TOKEN_NUMBER, LXR_TOKEN_BIN, &tok))
        /* correct type */

    /* only check but don't parse */
    if (mml_peek_string(&lexer, "string"))
        /* correct string  */
    if (mml_peek_type(&lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_DOLLAR, &tok))
        /* correct type */

    /* token compare function */
    if (!mml_token_cmp(&tok, "string"))
        /* token holds string 'string' */
    if (!mml_token_icmp(&tok, "string"))
        /* token holds case independent string 'string' */

    /* copy token content into buffer */
    char buffer[1024]
    mml_token_cpy(buffer, 1024, &tok);

    /* token to number conversion */
    /* You should always check if the token (sub)type is correct */
    mml_int i = mml_token_to_int(&tok);
    mml_float f = mml_token_to_float(&tok);
    mml_double d = mml_token_to_double(&tok);
    mml_ulong ul = mml_token_to_unsigned_long(&tok);
#endif

 /* ===============================================================
 *
 *                          HEADER
 *
 * =============================================================== */
#ifndef MML_H_
#define MML_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MML_STATIC
#define MML_API static
#else
#define MML_API extern
#endif

/* ---------------------------------------------------------------
 *                          BASIC
 * ---------------------------------------------------------------*/
#ifdef MML_USE_FIXED_TYPES
/* setting this define adds header <stdint.h> for fixed sized types
 * if not defined each type has to be set to the correct size*/
#include <stdint.h>
typedef int32_t mml_int;
typedef int32_t mml_bool;
typedef int64_t mml_long;
typedef uint32_t mml_uint;
typedef uint64_t mml_ulong;
typedef uint8_t mml_byte;
typedef uint64_t mml_size;
typedef float mml_float;
typedef double mml_double;
#else
typedef int mml_int;
typedef int mml_bool;
typedef long mml_long;
typedef unsigned int mml_uint;
typedef unsigned long mml_ulong;
typedef unsigned char mml_byte;
typedef unsigned long mml_size;
typedef float mml_float;
typedef double mml_double;
#endif

/* ---------------------------------------------------------------
 *                      PUNCTUATION
 * ---------------------------------------------------------------*/
/* This is the default punctuation table to convert from string to
 * an identifier. You can create your own punctuation table
 * and use your custom punctuation parsing scheme.
 * IMPORTANT: the list has to be ordered by string length */
#define MML_DEFAULT_PUNCTION_MAP(PUNCTUATION)\
    PUNCTUATION(">>=",  MML_PUNCT_RSHIFT_ASSIGN)\
    PUNCTUATION("<<=",  MML_PUNCT_LSHIFT_ASSIGN)\
    PUNCTUATION("...",  MML_PUNCT_PARAMS)\
    PUNCTUATION("&&",   MML_PUNCT_LOGIC_AND)\
    PUNCTUATION("||",   MML_PUNCT_LOGIC_OR)\
    PUNCTUATION(">=",   MML_PUNCT_LOGIC_GEQ)\
    PUNCTUATION("<=",   MML_PUNCT_LOGIC_LEQ)\
    PUNCTUATION("==",   MML_PUNCT_LOGIC_EQ)\
    PUNCTUATION("!=",   MML_PUNCT_LOGIC_UNEQ)\
    PUNCTUATION("*=",   MML_PUNCT_MUL_ASSIGN)\
    PUNCTUATION("/=",   MML_PUNCT_DIV_ASSIGN)\
    PUNCTUATION("%=",   MML_PUNCT_MOD_ASSIGN)\
    PUNCTUATION("+=",   MML_PUNCT_ADD_ASSIGN)\
    PUNCTUATION("-=",   MML_PUNCT_SUB_ASSIGN)\
    PUNCTUATION("++",   MML_PUNCT_INC)\
    PUNCTUATION("--",   MML_PUNCT_DEC)\
    PUNCTUATION("&=",   MML_PUNCT_BIN_AND_ASSIGN)\
    PUNCTUATION("|=",   MML_PUNCT_BIN_OR_ASSIGN)\
    PUNCTUATION("^=",   MML_PUNCT_BIN_XOR_ASSIGN)\
    PUNCTUATION(">>",   MML_PUNCT_RSHIFT)\
    PUNCTUATION("<<",   MML_PUNCT_LSHIFT)\
    PUNCTUATION("->",   MML_PUNCT_POINTER)\
    PUNCTUATION("::",   MML_PUNCT_CPP1)\
    PUNCTUATION(".*",   MML_PUNCT_CPP2)\
    PUNCTUATION("*",    MML_PUNCT_MUL)\
    PUNCTUATION("/",    MML_PUNCT_DIV)\
    PUNCTUATION("%",    MML_PUNCT_MOD)\
    PUNCTUATION("+",    MML_PUNCT_ADD)\
    PUNCTUATION("-",    MML_PUNCT_SUB)\
    PUNCTUATION("=",    MML_PUNCT_ASSIGN)\
    PUNCTUATION("&",    MML_PUNCT_BIN_AND)\
    PUNCTUATION("|",    MML_PUNCT_BIN_OR)\
    PUNCTUATION("^",    MML_PUNCT_BIN_XOR)\
    PUNCTUATION("~",    MML_PUNCT_BIN_NOT)\
    PUNCTUATION("!",    MML_PUNCT_LOGIC_NOT)\
    PUNCTUATION(">",    MML_PUNCT_LOGIC_GREATER)\
    PUNCTUATION("<",    MML_PUNCT_LOGIC_LESS)\
    PUNCTUATION(".",    MML_PUNCT_REF)\
    PUNCTUATION(",",    MML_PUNCT_COMMA)\
    PUNCTUATION(";",    MML_PUNCT_SEMICOLON)\
    PUNCTUATION(":",    MML_PUNCT_COLON)\
    PUNCTUATION("?",    MML_PUNCT_QUESTIONMARK)\
    PUNCTUATION("(",    MML_PUNCT_PARENTHESE_OPEN)\
    PUNCTUATION(")",    MML_PUNCT_PARENTHESE_CLOSE)\
    PUNCTUATION("{",    MML_PUNCT_BRACE_OPEN)\
    PUNCTUATION("}",    MML_PUNCT_BRACE_CLOSE)\
    PUNCTUATION("[",    MML_PUNCT_BRACKET_OPEN)\
    PUNCTUATION("]",    MML_PUNCT_BRACKET_CLOSE)\
    PUNCTUATION("\\",   MML_PUNCT_BACKSLASH)\
    PUNCTUATION("#",    MML_PUNCT_PRECOMPILER)\
    PUNCTUATION("$",    MML_PUNCT_DOLLAR)

enum mml_default_punctuation_ids {
#define PUNCTUATION(chars, id) id,
    MML_DEFAULT_PUNCTION_MAP(PUNCTUATION)
#undef PUNCTUATION
    MML_PUNCT_MAX
};

struct mml_punctuation {
    const char *string;
    /* string representation of the punctuation */
    mml_int id;
    /* number identitfier to stored inside the token flag */
};

/* ---------------------------------------------------------------
 *                          TOKEN
 * ---------------------------------------------------------------*/
enum mml_token_type {
    MML_TOKEN_STRING,
    /* strings literal: "string" */
    MML_TOKEN_LITERAL,
    /* character literal: 'c' */
    MML_TOKEN_NUMBER,
    /* integer or floating pointer number */
    MML_TOKEN_NAME,
    /* names and keyword */
    MML_TOKEN_PUNCTUATION
    /* punctuation from the punctuation table */
};

/* token subtype flags */
enum mml_token_flags {
    MML_TOKEN_INT           = 0x00001,/* integer type */
    MML_TOKEN_DEC           = 0x00002,/* decimal number */
    MML_TOKEN_HEX           = 0x00004,/* hexadecimal number */
    MML_TOKEN_OCT           = 0x00008,/* octal number */
    MML_TOKEN_BIN           = 0x00010,/* binary number */
    MML_TOKEN_LONG          = 0x00020,/* long integer number */
    MML_TOKEN_UNSIGNED      = 0x00040,/* unsigned number */
    MML_TOKEN_FLOAT         = 0x00080,/* floating pointer number */
    MML_TOKEN_SINGLE_PREC   = 0x00100,/* single precision float */
    MML_TOKEN_DOUBLE_PREC   = 0x00200,/* double precision float */
    MML_TOKEN_INFINITE      = 0x00400,/* infinit float number */
    MML_TOKEN_INDEFINITE    = 0x00800,/* indefinite float number */
    MML_TOKEN_NAN           = 0x01000,/* Not a number float */
    MML_TOKEN_VALIDVAL      = 0x02000 /* flag if the token is a number */
};

struct mml_token {
/*  A Token represents a section in a string containing a substring.
    Since the token only references the text, no actual memory needs
    to be allocated from the library to hold each token. Therefore the library does not
    represent a classical datastructure holding a string, instead
    it is only responsible for referencing the string.
    In addition it is to note that the string
    inside of the token is not terminated with '\0' since it would require write
    access to the text which is not always wanted.*/
    enum mml_token_type type;
    /* main type of the token */
    mml_uint subtype;
    /* subtype flags of the token */
    mml_size line;
    /* text line the token was read from */
    mml_bool line_crossed;
    /* flag indicating if the token spans over multible lines */
    struct {mml_ulong i; mml_double f;} value;
    /* number representation of the token */
    const char *str;
    /* text pointer to the beginning of the token inside the text */
    mml_size len;
    /* byte length of the token */
};

MML_API mml_size mml_token_cpy(char*, mml_size max, const struct mml_token*);
/*  this function copies the content of a token into a buffer
    Input:
    - buffer to copy the string into
    - maximum number of character to copy into the buffer
    - token to copy the string from
    Output:
    - length of the copied string
*/
MML_API mml_int mml_token_cmp(const struct mml_token*, const char*);
/*  this function compares the token content with a user string
    Input:
    - token to check the string against
    - string to check
    Output:
    - if equal will return 1 otherwise 0 (same as strcmp)
*/
MML_API mml_int mml_token_icmp(const struct mml_token*, const char*);
/*  this function makes a case insensitive compare of the
 *  token content with a user string.
    Input:
    - token to check the string against
    - string to check
    Output:
    - if equal will return 1 otherwise 0 (same as strcmp)
*/
MML_API mml_int mml_token_to_int(struct mml_token*);
/*  this function converts the token content into an int
    Input:
    - token to convert into an integer
    Output:
    - converted integer value if token is a number or 0 otherwise
*/
MML_API mml_float mml_token_to_float(struct mml_token*);
/*  this function converts the token content into an float
    Input:
    - token to convert into an floating point number
    Output:
    - converted float value if token is a number or 0.0f otherwise
*/
MML_API mml_double mml_token_to_double(struct mml_token*);
/*  this function converts the token content into an double
    Input:
    - token to convert into an double floating point number
    Output:
    - converted double value if token is a number or 0.0 otherwise
*/
MML_API mml_ulong mml_token_to_unsigned_long(struct mml_token*);
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
enum mml_log_level {MML_WARNING,MML_ERROR};
typedef void(*mml_log_f)(void*, enum mml_log_level, mml_size line, const char *msg, ...);

struct mml_lexer {
/*  The lexer context holds the current state of the parsing process,
    and only refrences a string that actually holds the source text.
    To parse punctuations the lexer uses a punctuation table to map
    between a string and identifier for each punctuation. The library
    thereby already has a default table but you can create a custom
    table to suite your needs.
    For error handling the lexer supports an error flag that can be
    checked as well a logging callback for more extensive error handling.*/
    mml_bool error;
    /* error flags that will be set if an error occurs*/
    const char *buffer;
    /* pointer to the beginning the the text to parse */
    const char *current;
    /* current position inside the text */
    const char *last;
    /* internal helper pointer to the last read text position */
    const char *end;
    /* pointer to the end of the text to parse */
    mml_size length;
    /* length of the text to parse */
    mml_size line;
    /* current line the text */
    mml_size last_line;
    /* last parsed line */
    const struct mml_punctuation *puncts;
    /* internally used punctuation table */
    mml_log_f log;
    /* logging callback for outputing error messages */
    void *userdata;
    /* userdata passed to the logging callback */
};

MML_API void mml_init(struct mml_lexer *lexer, const char *ptr, mml_size len,
                    const struct mml_punctuation *punct, mml_log_f log, void *usr);
/*  this function initializes the lexer
    Input:
    - pointer to a text buffer to parse
    - length of the text buffer
    - custom punctuation table or NULL for default table
    - logging callback or NULL if not needed
    - userdata passed into the callback or NULL if not needed
*/
MML_API void mml_reset(struct mml_lexer*);
/*  this function resets the lexer back to beginning */
MML_API mml_int mml_read(struct mml_lexer*, struct mml_token*);
/*  this function reads a token from a loaded lexer
    Input:
    - token to hold the parsed content information
    Output:
    - if successfully 1 or 0 otherwise
*/
MML_API mml_int mml_read_on_line(struct mml_lexer*, struct mml_token*);
/*  this function reads a token from a loaded lexer only if on the same line
    Input:
    - token to hold the parsed content information
    Output:
    - if successfully read 1 or 0 otherwise
*/
MML_API void mml_unread(struct mml_lexer*, struct mml_token*);
/*  this function pushes back a token (limitied to exactly one token)
    Input:
    - token push back into the lexer
*/
MML_API mml_int mml_expect_string(struct mml_lexer*, const char*);
/*  this function reads a token and check the token content. If the token
    content is not equal to the provided string a error occurs.
    Input:
    - the expected parsed token string
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MML_API mml_int mml_expect_type(struct mml_lexer*, enum mml_token_type type,
                                mml_uint subtype, struct mml_token*);
/*  this function reads a token and checks for token type + subtype. If the token
    type and subtype or not correct an error will be raised.
    Input:
    - expected token type
    - expected token subtype
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MML_API mml_int mml_expect_any(struct mml_lexer*, struct mml_token*);
/*  this function tries to read in a token and if not possible will raise and error
    Input:
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MML_API mml_int mml_check_string(struct mml_lexer*, const char*);
/*  this function tries to read in a token holding with given content.
 *  If it succeeds the token will be returned. If not the read token will be unread.
    Input:
    - the expected parsed token string
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MML_API mml_int mml_check_type(struct mml_lexer*, enum mml_token_type type,
                                mml_uint subtype, struct mml_token*);
/*  this function tries to read in a token holding with token type and subtype.
 *  If it succeeds the token will be returned. If not the read token will be unread.
    Input:
    - expected token type
    - expected token subtype
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MML_API mml_int mml_peek_string(struct mml_lexer*, const char*);
/*  this function checks the next token for the given string content
    Input:
    - the expected parsed token string
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MML_API mml_int mml_peek_type(struct mml_lexer*, enum mml_token_type type,
                            mml_uint subtype, struct mml_token*);
/*  this function checks the next token for the given type and subtype
    Input:
    - expected token type
    - expected token subtype
    - token to hold the parsed content information
    Output:
    - 1 if a token could be read or 0 otherwise
*/
MML_API mml_int mml_skip_until(struct mml_lexer*, const char*);
/*  this function skips all tokens until a token holding a certain string
    Input:
    - a expected string the parse should skip to
    Output:
    - 1 if successful, 0 otherwise
*/
MML_API mml_int mml_skip_line(struct mml_lexer*);
/*  this function skips the current line */
MML_API mml_int mml_parse_int(struct mml_lexer*);
/*  this function reads in a token and tries to convert it into an integer.
    If the conversion fails an error will be raised.
    Output:
    - parsed integer value
*/
MML_API mml_bool mml_parse_bool(struct mml_lexer*);
/*  this function reads in a token and tries to convert it into an boolean.
    If the conversion fails an error will be raised.
    Output:
    - parsed boolean value
*/
MML_API mml_float mml_parse_float(struct mml_lexer*);
/*  this function reads in a token and tries to convert it into an float.
    If the conversion fails an error will be raised.
    Output:
    - parsed floating point value
*/
#ifdef __cplusplus
}
#endif
#endif /* MML_H_ */

/* ===============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================*/
#ifdef MML_IMPLEMENTATION

#define MML_LEN(a) (sizeof(a)/sizeof((a)[0]))
#define MML_UNUSED(a) ((void)(a))

#ifdef MML_USE_ASSERT
#ifndef MML_ASSERT
#include <assert.h>
#define MML_ASSERT(expr) assert(expr)
#endif
#else
#define MML_ASSERT(expr)
#endif

/* Pointer to Integer type conversion for pointer alignment */
#if defined(__PTRDIFF_TYPE__) /* This case should work for GCC*/
# define MML_UINT_TO_PTR(x) ((void*)(__PTRDIFF_TYPE__)(x))
# define MML_PTR_TO_UINT(x) ((mml_size)(__PTRDIFF_TYPE__)(x))
#elif !defined(__GNUC__) /* works for compilers other than LLVM */
# define MML_UINT_TO_PTR(x) ((void*)&((char*)0)[x])
# define MML_PTR_TO_UINT(x) ((wby_size)(((char*)x)-(char*)0))
#elif defined(MML_USE_FIXED_TYPES) /* used if we have <stdint.h> */
# define MML_UINT_TO_PTR(x) ((void*)(uintptr_t)(x))
# define MML_PTR_TO_UINT(x) ((uintptr_t)(x))
#else /* generates warning but works */
# define MML_UINT_TO_PTR(x) ((void*)(x))
# define MML_PTR_TO_UINT(x) ((wby_size)(x))
#endif

#define MML_INTERN static
#define MML_GLOBAL static
#define MML_STORAGE static

/* library intern default punctuation map */
MML_GLOBAL const struct mml_punctuation mml_default_punctuations[] = {
#define PUNCTUATION(chars, id) {chars, id},
    MML_DEFAULT_PUNCTION_MAP(PUNCTUATION)
#undef PUNCTUATION
    {0, 0}
};
/* ---------------------------------------------------------------
 *                          UTIL
 * ---------------------------------------------------------------*/
MML_INTERN char
mml_char_upper(char c)
{
    if (c >= 'a' && c <= 'z')
        return (char)('A' + (c - 'a'));
    return c;
}

MML_INTERN void
mml_fill_size(void *ptr, int c0, mml_size size)
{
    #define word unsigned
    #define wsize sizeof(word)
    #define wmask (wsize - 1)
    unsigned char *dst = (unsigned char*)ptr;
    unsigned c = 0;
    mml_size t = 0;

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
    if ((t = MML_PTR_TO_UINT(dst) & wmask) != 0) {
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

#define mml_zero_struct(s) mml_zero_size(&s, sizeof(s))
#define mml_zero_array(p,n) mml_zero_size(p, (n) * sizeof((p)[0]))
MML_INTERN void
mml_zero_size(void *ptr, mml_size size)
{
    mml_fill_size(ptr, 0, size);
}

/* ---------------------------------------------------------------
 *                          TOKEN
 * ---------------------------------------------------------------*/
MML_INTERN void
mml_token_clear(struct mml_token *tok)
{
    tok->line_crossed = 0;
}

MML_API mml_size
mml_token_cpy(char *dst, mml_size max, const struct mml_token* tok)
{
    unsigned i = 0;
    mml_size ret;
    mml_size siz;

    MML_ASSERT(dst);
    MML_ASSERT(tok);
    if (!dst || !max || !tok)
        return 0;

    ret = (max < (tok->len + 1)) ? max : tok->len;
    siz = (max < (tok->len + 1)) ? max-1 : tok->len;
    for (i = 0; i < siz; i++)
        dst[i] = tok->str[i];
    dst[siz] = '\0';
    return ret;
}

MML_API mml_int
mml_token_icmp(const struct mml_token* tok, const char* str)
{
    mml_size i;
    MML_ASSERT(tok);
    MML_ASSERT(str);
    if (!tok || !str) return 1;
    for (i = 0; (i < tok->len && *str); i++, str++){
        if (mml_char_upper(tok->str[i]) != mml_char_upper(*str))
            return 1;
    }
    if (*str != 0 || i < tok->len)
        return 1;
    return 0;
}

MML_API mml_int
mml_token_cmp(const struct mml_token* tok, const char* str)
{
    mml_size i;
    MML_ASSERT(tok);
    MML_ASSERT(str);
    if (!tok || !str) return 1;
    for (i = 0; (i < tok->len && *str); i++, str++){
        if (tok->str[i] != *str)
            return 1;
    }
    if (*str != 0 || i < tok->len)
        return 1;
    return 0;
}

MML_INTERN mml_double
mml_token_parse_double(const char *p, mml_size length)
{
    mml_int i, div, pow;
    mml_double m;
    mml_size len = 0;
    mml_double f = 0;
    while (len < length && p[len] != '.' && p[len] != 'e') {
        f = f * 10.0 + (mml_double)(p[len] - '0');
        len++;
    }

    if (len < length && p[len] == '.') {
        len++;
        for (m = 0.1; len < length; len++) {
            f = f + (mml_double)(p[len] - '0') * m;
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
            pow = pow * 10 + (mml_int)(p[len] - '0');
        for (m = 1.0, i = 0; i < pow; ++i)
            m *= 100.0;
        if (div) f /= m;
        else f *= m;
    }
    return f;
}

MML_INTERN mml_ulong
mml_token_parse_int(const char *p, mml_size length)
{
    mml_ulong i = 0;
    mml_size len = 0;
    while (len < length) {
        i = i * 10 + (mml_ulong)(p[len] - '0');
        len++;
    }
    return i;
}

MML_INTERN mml_ulong
mml_token_parse_oct(const char *p, mml_size length)
{
    mml_ulong i = 0;
    mml_size len = 1;
    while (len < length) {
        i = (i << 3) + (mml_ulong)(p[len] - '0');
        len++;
    }
    return i;
}

MML_INTERN mml_ulong
mml_token_parse_hex(const char *p, mml_size length)
{
    mml_ulong i = 0;
    mml_size len = 2;
    while (len < length) {
        i <<= 4;
        if (p[len] >= 'a' && p[len] <= 'f')
            i += (mml_ulong)((p[len] - 'a') + 10);
        else if (p[len] >= 'A' && p[len] <= 'F') {
            i += (mml_ulong)((p[len] - 'A') + 10);
        } else i += (mml_ulong)(p[len] - '0');
        len++;
    }
    return i;
}

MML_INTERN mml_ulong
mml_token_parse_bin(const char *p, mml_size length)
{
    mml_ulong i = 0;
    mml_size len = 2;
    while (len < length) {
        i = (i << 1) + (mml_ulong)(p[len] - '0');
        len++;
    }
    return i;
}

MML_INTERN void
mml_token_number_value(struct mml_token *tok)
{
    mml_int i, pow, c;
    const char *p;
    MML_ASSERT(tok->type == MML_TOKEN_NUMBER);
    tok->value.i = 0;
    tok->value.f = 0;

    p = tok->str;
    if (tok->subtype & MML_TOKEN_FLOAT) {
        if (tok->subtype & ((MML_TOKEN_INFINITE|MML_TOKEN_INDEFINITE|MML_TOKEN_NAN))) {
            /* special real number constants */
            union {mml_float f; mml_uint u;} convert;
            if (tok->subtype & MML_TOKEN_INFINITE) {
                convert.u = 0x7f800000;
                tok->value.f = (mml_double)convert.f;
            } else if (tok->subtype & MML_TOKEN_INDEFINITE) {
                convert.u = 0xffc00000;
                tok->value.f = (mml_double)convert.f;
            } else if (tok->subtype & MML_TOKEN_NAN) {
                convert.u = 0x7fc00000;
                tok->value.f = (mml_double)convert.f;
            }
        } else tok->value.f = mml_token_parse_double(tok->str, tok->len);
        tok->value.i = (mml_ulong)tok->value.f;
    } else if (tok->subtype & MML_TOKEN_DEC) {
        /* paser decimal number */
        tok->value.i = mml_token_parse_int(p, tok->len);
        tok->value.f = (mml_double)tok->value.i;
    } else if (tok->subtype & MML_TOKEN_OCT) {
        /* parse octal number */
        tok->value.i = mml_token_parse_oct(p, tok->len);
        tok->value.f = (mml_double)tok->value.i;
    } else if (tok->subtype & MML_TOKEN_HEX) {
        /* parse hex number */
        tok->value.i = mml_token_parse_hex(p, tok->len);
        tok->value.f = (mml_double)tok->value.i;
    } else if (tok->subtype & MML_TOKEN_BIN) {
        /* parse binary number */
        tok->value.i = mml_token_parse_bin(p, tok->len);
        tok->value.f = (mml_double)tok->value.i;
    }
    tok->subtype |= MML_TOKEN_VALIDVAL;
}

MML_API mml_int
mml_token_to_int(struct mml_token *tok)
{
    return (mml_int)mml_token_to_unsigned_long(tok);
}

MML_API mml_float
mml_token_to_float(struct mml_token *tok)
{
    mml_double d = mml_token_to_double(tok);
    mml_float f = (mml_float)d;;
    return f;
}

MML_API mml_double
mml_token_to_double(struct mml_token *tok)
{
    if (tok->type != MML_TOKEN_NUMBER)
        return 0.0;
    if (!(tok->subtype & MML_TOKEN_VALIDVAL))
        mml_token_number_value(tok);
    if (!(tok->subtype & MML_TOKEN_VALIDVAL))
        return 0.0;
    return tok->value.f;
}

MML_API mml_ulong
mml_token_to_unsigned_long(struct mml_token *tok)
{
    if (tok->type != MML_TOKEN_NUMBER)
        return 0;
    if (!(tok->subtype & MML_TOKEN_VALIDVAL))
        mml_token_number_value(tok);
    if (!(tok->subtype & MML_TOKEN_VALIDVAL))
        return 0;
    return tok->value.i;
}

/* ---------------------------------------------------------------
 *                          LEXER
 * ---------------------------------------------------------------*/
MML_API void
mml_init(struct mml_lexer *lexer, const char *ptr, mml_size len,
    const struct mml_punctuation *punct, mml_log_f log, void *userdata)
{
    mml_zero_struct(*lexer);
    lexer->buffer = ptr;
    lexer->current = ptr;
    lexer->last = ptr;
    lexer->end = ptr + len;
    lexer->length = len;
    lexer->line = lexer->last_line = 1;
    if (!punct)
        lexer->puncts = mml_default_punctuations;
    else lexer->puncts = punct;
    lexer->log = log;
    lexer->userdata = userdata;
}

MML_API void
mml_reset(struct mml_lexer *lexer)
{
    lexer->current = lexer->buffer;
    lexer->last = lexer->buffer;
    lexer->line = lexer->last_line = 1;
}

MML_INTERN mml_int
mml_read_white_space(struct mml_lexer *lexer, mml_bool current_line)
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
                            lexer->log(lexer->userdata, MML_WARNING, lexer->line,
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

MML_INTERN mml_int
mml_read_esc_chars(struct mml_lexer *lexer, char *ch)
{
    mml_int c, val, i;
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
                lexer->log(lexer->userdata, MML_WARNING, lexer->line,
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
                lexer->log(lexer->userdata, MML_ERROR, lexer->line,
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
                lexer->log(lexer->userdata, MML_WARNING, lexer->line,
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

MML_INTERN mml_int
mml_read_string(struct mml_lexer *lexer, struct mml_token *token, mml_int quote)
{
    mml_size tmpline;
    const char *tmp;
    char ch;
    if (quote == '\"')
        token->type = MML_TOKEN_STRING;
    else token->type = MML_TOKEN_LITERAL;
    lexer->current++;
    if (lexer->current >= lexer->end)
        return 0;

    token->len = 0;
    token->str = lexer->current;
    while (1) {
        if (*lexer->current == '\\') {
            if (!mml_read_esc_chars(lexer, &ch))
                return 0;
        } else if (*lexer->current == quote) {
            lexer->current++;
            if (lexer->current >= lexer->end)
                return 0;

            tmp = lexer->current;
            tmpline = lexer->line;
            if (!mml_read_white_space(lexer, 0)) {
                lexer->current = tmp;
                lexer->line = tmpline;
                break;
            }
            if (*lexer->current == '\0') {
                if (lexer->log) {
                        lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                            "expecting string after '\' terminated line");
                }
                lexer->error = 1;
                return 0;
            }
            break;
        } else {
            if (*lexer->current == '\0') {
                if (lexer->log) {
                    lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                        "missing trailing quote");
                }
                lexer->error = 1;
                return 0;
            }
            if (*lexer->current == '\n') {
                if (lexer->log) {
                    lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                        "newline inside string");
                }
                lexer->error = 1;
                return 0;
            }
            lexer->current++;
        }
    }
    if (token->str) {
        token->len = (mml_size)(lexer->current - token->str) - 1;
        if (token->type == MML_TOKEN_LITERAL)
            token->subtype = (mml_uint)token->str[0];
        else token->subtype = (mml_uint)token->len;
    }
    return 1;
}

MML_INTERN mml_int
mml_read_name(struct mml_lexer *lexer, struct mml_token *token)
{
    char c;
    token->type = MML_TOKEN_NAME;
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
    token->subtype = (mml_uint)token->len;
    return 1;
}

MML_INTERN mml_int
mml_check_str(const struct mml_lexer *lexer, const char *str, mml_size len)
{
    mml_size i;
    for (i = 0; i < len && (&lexer->current[i] < lexer->end); ++i) {
        if (lexer->current[i] != str[i])
            return 0;
    }
    if (i < len) return 0;
    return 1;
}

MML_INTERN mml_int
mml_read_number(struct mml_lexer *lexer, struct mml_token *token)
{
    mml_size i;
    mml_bool dot;
    char c, c2;

    token->type = MML_TOKEN_NUMBER;
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
            token->subtype = MML_TOKEN_HEX | MML_TOKEN_INT;
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
            token->subtype = MML_TOKEN_BIN | MML_TOKEN_INT;
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
            token->subtype = MML_TOKEN_OCT | MML_TOKEN_INT;
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
            token->subtype = MML_TOKEN_DEC | MML_TOKEN_FLOAT;
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
                if (mml_check_str(lexer, "INF", 3))
                    token->subtype  |= MML_TOKEN_INFINITE;
                else if (mml_check_str(lexer, "IND", 3))
                    token->subtype  |= MML_TOKEN_INDEFINITE;
                else if (mml_check_str(lexer, "NAN", 3))
                    token->subtype  |= MML_TOKEN_NAN;
                else if (mml_check_str(lexer, "QNAN", 4)) {
                    token->subtype  |= MML_TOKEN_NAN; c2++;
                } else if (mml_check_str(lexer, "SNAN", 4)) {
                    token->subtype  |= MML_TOKEN_NAN; c2++;
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
            token->subtype = MML_TOKEN_DEC | MML_TOKEN_INT;
        }
    }

    if (token->subtype & MML_TOKEN_FLOAT) {
        /* float point precision subtype */
        if (c > ' ') {
            if (c == 'f' || c == 'F') {
                token->subtype |= MML_TOKEN_SINGLE_PREC;
                lexer->current++;
            } else token->subtype |= MML_TOKEN_DOUBLE_PREC;
        } else token->subtype |= MML_TOKEN_DOUBLE_PREC;
    } else if (token->subtype & MML_TOKEN_INT) {
        /* integer subtype */
        if (c > ' '){
            for (i = 0; i < 2; ++i) {
                if (c == 'l' || c == 'L')
                    token->subtype |= MML_TOKEN_LONG;
                else if (c == 'u' || c == 'U')
                    token->subtype |= MML_TOKEN_UNSIGNED;
                else break;
                if (lexer->current+1 >= lexer->end) break;
                c = *(++lexer->current);
            }
        }
    }
    return 1;
}

MML_INTERN mml_int
mml_read_punctuation(struct mml_lexer *lexer, struct mml_token *token)
{
    const struct mml_punctuation *punc;
    mml_int l, i;
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
            token->len += (mml_size)l;
            lexer->current += l;
            token->type = MML_TOKEN_PUNCTUATION;
            token->subtype = (mml_uint)punc->id;
            return 1;
        }
    }
    return 0;
}

MML_API mml_int
mml_read(struct mml_lexer *lexer, struct mml_token *token)
{
    mml_int c;
    if (!lexer->current) return 0;
    if (lexer->current >= lexer->end) return 0;
    if (lexer->error == 1) return 0;

    mml_zero_struct(*token);
    lexer->last = lexer->current;
    lexer->last_line = lexer->line;
    lexer->error = 0;
    if (!mml_read_white_space(lexer, 0))
        return 0;

    token->line = lexer->line;
    token->line_crossed = (lexer->line - lexer->last_line) ? 1 : 0;

    c = *lexer->current;
    if ((c >= '0' && c <= '9') ||
        (c == '.' && (*(lexer->current + 1)) >= '0' &&
        (c == '.' && (*(lexer->current + 1)) <= '9'))) {
        if (!mml_read_number(lexer, token))
            return 0;
    } else if (c == '\"' || c == '\'') {
        if (!mml_read_string(lexer, token, c))
            return 0;
    } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        if (!mml_read_name(lexer, token))
            return 0;
    } else if ((c == '/' || c == '\\') || c == '.') {
        if (!mml_read_name(lexer, token))
            return 0;
    } else if (!mml_read_punctuation(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                "unkown punctuation: %c", c);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

MML_API mml_int
mml_read_on_line(struct mml_lexer *lexer, struct mml_token *token)
{
    struct mml_token tok;
    if (!mml_read(lexer, &tok)) {
        lexer->current = lexer->last;
        lexer->line = lexer->last_line;
    }
    if (!tok.line_crossed) {
        *token = tok;
        return 1;
    }
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    mml_token_clear(token);
    return 0;
}

MML_API mml_int
mml_expect_string(struct mml_lexer *lexer, const char *string)
{
    struct mml_token token;
    if (!mml_read(lexer, &token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                "failed to read expected token: %s", string);
        }
        lexer->error = 1;
        return 0;
    }
    if (mml_token_cmp(&token, string)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                "read token is not expected string: %s", string);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

MML_API mml_int
mml_expect_type(struct mml_lexer *lexer, enum mml_token_type type,
    mml_uint subtype, struct mml_token *token)
{
    if (!mml_read(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                "could not read expected token with type: %d", type);
        }
        lexer->error = 1;
        return 0;
    }
    if (token->type != type) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                "read token has type %s instead of expected type: %d", token->type, type);
        }
        lexer->error = 1;
        return 0;
    }
    if ((token->subtype & subtype) != subtype) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                "read token has subtype %d instead of expected subtype %d", token->subtype, type);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

MML_API mml_int
mml_expect_any(struct mml_lexer *lexer, struct mml_token *token)
{
    if (!mml_read(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                "could not read any expected token");
        }
        return 0;
    }
    return 1;
}

MML_API mml_int
mml_check_string(struct mml_lexer *lexer, const char *string)
{
    struct mml_token token;
    if (!mml_read(lexer, &token))
        return 0;
    if (!mml_token_cmp(&token, string))
        return 1;

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    return 0;
}

MML_API mml_int
mml_check_type(struct mml_lexer *lexer, enum mml_token_type type,
    mml_uint subtype, struct mml_token *token)
{
    struct mml_token tok;
    if (!mml_read(lexer, &tok))
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

MML_API mml_int
mml_peek_string(struct mml_lexer *lexer, const char *string)
{
    struct mml_token tok;
    if (!mml_read(lexer, &tok))
        return 0;

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    if (!mml_token_cmp(&tok, string))
        return 1;
    return 0;
}

MML_API mml_int
mml_peek_type(struct mml_lexer *lexer, enum mml_token_type type,
    mml_uint subtype, struct mml_token *token)
{
    struct mml_token tok;
    if (!mml_read(lexer, &tok))
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

MML_API mml_int
mml_skip_until(struct mml_lexer *lexer, const char *string)
{
    struct mml_token tok;
    while (mml_read(lexer, &tok)) {
        if (!mml_token_cmp(&tok, string))
            return 1;
    }
    return 0;
}

MML_API mml_int
mml_skip_line(struct mml_lexer *lexer)
{
    struct mml_token tok;
    while (mml_read(lexer, &tok)) {
        if (tok.line_crossed) {
            lexer->current = lexer->last;
            lexer->line = lexer->last_line;
            return 1;
        }
    }
    return 0;
}

MML_API mml_int
mml_parse_int(struct mml_lexer *lexer)
{
    struct mml_token tok;
    if (!mml_read(lexer, &tok))
        return 0;
    if (tok.type == MML_TOKEN_PUNCTUATION && tok.str[0] == '-') {
        mml_expect_type(lexer, MML_TOKEN_NUMBER, MML_TOKEN_INT, &tok);
        return -mml_token_to_int(&tok);
    } else if (tok.type != MML_TOKEN_NUMBER || tok.subtype == MML_TOKEN_FLOAT) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                "expected int value but found float");
        }
        lexer->error = 1;
    }
    return mml_token_to_int(&tok);
}

MML_API mml_bool
mml_parse_bool(struct mml_lexer *lexer)
{
    struct mml_token tok;
    if (!mml_expect_type(lexer, MML_TOKEN_NUMBER, 0, &tok)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                "could not read expected boolean");
        }
        lexer->error = 1;
        return 0;
    }
    return (mml_token_to_int(&tok) != 0);
}

MML_API mml_float
mml_parse_float(struct mml_lexer *lexer)
{
    struct mml_token tok;
    if (!mml_read(lexer, &tok)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                "could not read expected float number");
        }
        lexer->error = 1;
        return 0;
    }
    if (tok.type == MML_TOKEN_PUNCTUATION && tok.str[0] == '-') {
        mml_expect_type(lexer, MML_TOKEN_NUMBER, 0, &tok);
        return -(mml_float)tok.value.f;
    } else if (tok.type != MML_TOKEN_NUMBER) {
        if (lexer->log) {
            lexer->log(lexer->userdata, MML_ERROR, lexer->line,
                "expected float number is not a number");
        }
        lexer->error = 1;
        return 0;
    }
    return mml_token_to_float(&tok);
}

#endif /* MML_IMPLEMENTATION */
