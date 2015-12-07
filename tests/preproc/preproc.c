#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* ===============================================================
 *
 *                          LEXER
 *
 * ===============================================================*/
#define LXR_DEFAULT_PUNCTION_MAP(PUNCTUATION)\
    PUNCTUATION(">>=",  LXR_PUNCT_RSHIFT_ASSIGN)\
    PUNCTUATION("<<=",  LXR_PUNCT_LSHIFT_ASSIGN)\
    PUNCTUATION("...",  LXR_PUNCT_PARAMS)\
    PUNCTUATION("&&",   LXR_PUNCT_LOGIC_AND)\
    PUNCTUATION("||",   LXR_PUNCT_LOGIC_OR)\
    PUNCTUATION(">=",   LXR_PUNCT_LOGIC_GEQ)\
    PUNCTUATION("<=",   LXR_PUNCT_LOGIC_LEQ)\
    PUNCTUATION("==",   LXR_PUNCT_LOGIC_EQ)\
    PUNCTUATION("!=",   LXR_PUNCT_LOGIC_UNEQ)\
    PUNCTUATION("*=",   LXR_PUNCT_MUL_ASSIGN)\
    PUNCTUATION("/=",   LXR_PUNCT_DIV_ASSIGN)\
    PUNCTUATION("%=",   LXR_PUNCT_MOD_ASSIGN)\
    PUNCTUATION("+=",   LXR_PUNCT_ADD_ASSIGN)\
    PUNCTUATION("-=",   LXR_PUNCT_SUB_ASSIGN)\
    PUNCTUATION("++",   LXR_PUNCT_INC)\
    PUNCTUATION("--",   LXR_PUNCT_DEC)\
    PUNCTUATION("&=",   LXR_PUNCT_BIN_AND_ASSIGN)\
    PUNCTUATION("|=",   LXR_PUNCT_BIN_OR_ASSIGN)\
    PUNCTUATION("^=",   LXR_PUNCT_BIN_XOR_ASSIGN)\
    PUNCTUATION(">>",   LXR_PUNCT_RSHIFT)\
    PUNCTUATION("<<",   LXR_PUNCT_LSHIFT)\
    PUNCTUATION("->",   LXR_PUNCT_POINTER)\
    PUNCTUATION("::",   LXR_PUNCT_CPP1)\
    PUNCTUATION(".*",   LXR_PUNCT_CPP2)\
    PUNCTUATION("*",    LXR_PUNCT_MUL)\
    PUNCTUATION("/",    LXR_PUNCT_DIV)\
    PUNCTUATION("%",    LXR_PUNCT_MOD)\
    PUNCTUATION("+",    LXR_PUNCT_ADD)\
    PUNCTUATION("-",    LXR_PUNCT_SUB)\
    PUNCTUATION("=",    LXR_PUNCT_ASSIGN)\
    PUNCTUATION("&",    LXR_PUNCT_BIN_AND)\
    PUNCTUATION("|",    LXR_PUNCT_BIN_OR)\
    PUNCTUATION("^",    LXR_PUNCT_BIN_XOR)\
    PUNCTUATION("~",    LXR_PUNCT_BIN_NOT)\
    PUNCTUATION("!",    LXR_PUNCT_LOGIC_NOT)\
    PUNCTUATION(">",    LXR_PUNCT_LOGIC_GREATER)\
    PUNCTUATION("<",    LXR_PUNCT_LOGIC_LESS)\
    PUNCTUATION(".",    LXR_PUNCT_REF)\
    PUNCTUATION(",",    LXR_PUNCT_COMMA)\
    PUNCTUATION(";",    LXR_PUNCT_SEMICOLON)\
    PUNCTUATION(":",    LXR_PUNCT_COLON)\
    PUNCTUATION("?",    LXR_PUNCT_QUESTIONMARK)\
    PUNCTUATION("(",    LXR_PUNCT_PARENTHESE_OPEN)\
    PUNCTUATION(")",    LXR_PUNCT_PARENTHESE_CLOSE)\
    PUNCTUATION("{",    LXR_PUNCT_BRACE_OPEN)\
    PUNCTUATION("}",    LXR_PUNCT_BRACE_CLOSE)\
    PUNCTUATION("[",    LXR_PUNCT_BRACKET_OPEN)\
    PUNCTUATION("]",    LXR_PUNCT_BRACKET_CLOSE)\
    PUNCTUATION("\\",   LXR_PUNCT_BACKSLASH)\
    PUNCTUATION("#",    LXR_PUNCT_PRECOMPILER)\
    PUNCTUATION("$",    LXR_PUNCT_DOLLAR)

enum lxr_default_punctuation_ids {
#define PUNCTUATION(chars, id) id,
    LXR_DEFAULT_PUNCTION_MAP(PUNCTUATION)
#undef PUNCTUATION
    LXR_PUNCT_MAX
};

struct lxr_punctuation {
    const char *string;
    /* string representation of the punctuation */
    int id;
    /* number identitfier to stored inside the token flag */
};

enum lxr_token_type {
    LXR_TOKEN_STRING,
    /* strings literal: "string" */
    LXR_TOKEN_LITERAL,
    /* character literal: 'c' */
    LXR_TOKEN_NUMBER,
    /* integer or floating pointer number */
    LXR_TOKEN_NAME,
    /* names and keyword */
    LXR_TOKEN_PUNCTUATION,
    /* punctuation from the punctuation table */
    LXR_TOKEN_EOS
    /* end of stream */
};

/* token subtype flags */
enum lxr_token_flags {
    LXR_TOKEN_INT           = 0x00001,/* integer type */
    LXR_TOKEN_DEC           = 0x00002,/* decimal number */
    LXR_TOKEN_HEX           = 0x00004,/* hexadecimal number */
    LXR_TOKEN_OCT           = 0x00008,/* octal number */
    LXR_TOKEN_BIN           = 0x00010,/* binary number */
    LXR_TOKEN_LONG          = 0x00020,/* long integer number */
    LXR_TOKEN_UNSIGNED      = 0x00040,/* unsigned number */
    LXR_TOKEN_FLOAT         = 0x00080,/* floating pointer number */
    LXR_TOKEN_SINGLE_PREC   = 0x00100,/* single precision float */
    LXR_TOKEN_DOUBLE_PREC   = 0x00200,/* double precision float */
    LXR_TOKEN_INFINITE      = 0x00400,/* infinit float number */
    LXR_TOKEN_INDEFINITE    = 0x00800,/* indefinite float number */
    LXR_TOKEN_NAN           = 0x01000,/* Not a number float */
    LXR_TOKEN_VALIDVAL      = 0x02000 /* flag if the token is a number */
};

struct lxr_token {
/*  A Token represents a section in a string containing a substring.
    Since the token only references the text, no actual memory needs
    to be allocated from the library to hold each token. Therefore the library does not
    represent a classical datastructure holding a string, instead
    it is only responsible for referencing the string.
    In addition it is to note that the string
    inside of the token is not terminated with '\0' since it would require write
    access to the text which is not always wanted.*/
    enum lxr_token_type type;
    /* main type of the token */
    unsigned int subtype;
    /* subtype flags of the token */
    size_t line;
    /* text line the token was read from */
    int line_crossed;
    /* flag indicating if the token spans over multible lines */
    struct {unsigned long i; double f;} value;
    /* number representation of the token */
    const char *str;
    /* text pointer to the beginning of the token inside the text */
    size_t len;
    /* byte length of the token */
};

enum lxr_log_level {LXR_WARNING,LXR_ERROR};
typedef void(*lxr_log_f)(void*, enum lxr_log_level, size_t line, const char *msg, ...);

struct lxr_lexer {
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
    size_t length;
    /* length of the text to parse */
    size_t line;
    /* current line the text */
    size_t last_line;
    /* last parsed line */
    const struct lxr_punctuation *puncts;
    /* internally used punctuation table */
    lxr_log_f log;
    /* logging callback for outputing error messages */
    void *userdata;
    /* userdata passed to the logging callback */
};

#define LXR_LEN(a) (sizeof(a)/sizeof((a)[0]))
#define LXR_UNUSED(a) ((void)(a))

#ifdef LXR_USE_ASSERT
#ifndef LXR_ASSERT
#include <assert.h>
#define LXR_ASSERT(expr) assert(expr)
#endif
#else
#define LXR_ASSERT(expr)
#endif

/* Pointer to Integer type conversion for pointer alignment */
#if defined(__PTRDIFF_TYPE__) /* This case should work for GCC*/
# define LXR_UINT_TO_PTR(x) ((void*)(__PTRDIFF_TYPE__)(x))
# define LXR_PTR_TO_UINT(x) ((size_t)(__PTRDIFF_TYPE__)(x))
#elif !defined(__GNUC__) /* works for compilers other than LLVM */
# define LXR_UINT_TO_PTR(x) ((void*)&((char*)0)[x])
# define LXR_PTR_TO_UINT(x) ((wby_size)(((char*)x)-(char*)0))
#elif defined(LXR_USE_FIXED_TYPES) /* used if we have <stdint.h> */
# define LXR_UINT_TO_PTR(x) ((void*)(uintptr_t)(x))
# define LXR_PTR_TO_UINT(x) ((uintptr_t)(x))
#else /* generates warning but works */
# define LXR_UINT_TO_PTR(x) ((void*)(x))
# define LXR_PTR_TO_UINT(x) ((wby_size)(x))
#endif

#define LXR_INTERN static
#define LXR_GLOBAL static
#define LXR_STORAGE static

#ifndef LXR_MEMSET
#define LXR_MEMSET lxr_memset
#endif

/* library intern default punctuation map */
LXR_GLOBAL const struct lxr_punctuation
lxr_default_punctuations[] = {
#define PUNCTUATION(chars, id) {chars, id},
    LXR_DEFAULT_PUNCTION_MAP(PUNCTUATION)
#undef PUNCTUATION
    {0, 0}
};
/* ---------------------------------------------------------------
 *                          UTIL
 * ---------------------------------------------------------------*/
LXR_INTERN char
lxr_char_upper(char c)
{
    if (c >= 'a' && c <= 'z')
        return (char)('A' + (c - 'a'));
    return c;
}

LXR_INTERN void
lxr_memset(void *ptr, int c0, size_t size)
{
    #define word unsigned
    #define wsize sizeof(word)
    #define wmask (wsize - 1)
    unsigned char *dst = (unsigned char*)ptr;
    unsigned c = 0;
    size_t t = 0;

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
    if ((t = LXR_PTR_TO_UINT(dst) & wmask) != 0) {
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

#define lxr_zero_struct(s) lxr_zero_size(&s, sizeof(s))
#define lxr_zero_array(p,n) lxr_zero_size(p, (n) * sizeof((p)[0]))
LXR_INTERN void
lxr_zero_size(void *ptr, size_t size)
{
    LXR_MEMSET(ptr, 0, size);
}

/* ---------------------------------------------------------------
 *                          TOKEN
 * ---------------------------------------------------------------*/
LXR_INTERN void
lxr_token_clear(struct lxr_token *tok)
{
    tok->line_crossed = 0;
}

static char*
lxr_token_dup(struct lxr_token *tok)
{
    char *buffer;
    if (!tok->len) return 0;
    buffer = calloc(tok->len+1, 1);
    memcpy(buffer, tok->str, tok->len);
    return buffer;
}

static size_t
lxr_token_cpy(char *dst, size_t max, const struct lxr_token* tok)
{
    unsigned i = 0;
    size_t ret;
    size_t siz;

    LXR_ASSERT(dst);
    LXR_ASSERT(tok);
    if (!dst || !max || !tok)
        return 0;

    ret = (max < (tok->len + 1)) ? max : tok->len;
    siz = (max < (tok->len + 1)) ? max-1 : tok->len;
    for (i = 0; i < siz; i++)
        dst[i] = tok->str[i];
    dst[siz] = '\0';
    return ret;
}

static int
lxr_token_icmp(const struct lxr_token* tok, const char* str)
{
    size_t i;
    LXR_ASSERT(tok);
    LXR_ASSERT(str);
    if (!tok || !str) return 1;
    for (i = 0; (i < tok->len && *str); i++, str++){
        if (lxr_char_upper(tok->str[i]) != lxr_char_upper(*str))
            return 1;
    }
    if (*str != 0 || i < tok->len)
        return 1;
    return 0;
}

static int
lxr_token_cmp(const struct lxr_token* tok, const char* str)
{
    size_t i;
    LXR_ASSERT(tok);
    LXR_ASSERT(str);
    if (!tok || !str) return 1;
    for (i = 0; (i < tok->len && *str); i++, str++){
        if (tok->str[i] != *str)
            return 1;
    }
    if (*str != 0 || i < tok->len)
        return 1;
    return 0;
}

LXR_INTERN double
lxr_token_parse_double(const char *p, size_t length)
{
    int i, div, pow;
    double m;
    size_t len = 0;
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

LXR_INTERN unsigned long
lxr_token_parse_int(const char *p, size_t length)
{
    unsigned long i = 0;
    size_t len = 0;
    while (len < length) {
        i = i * 10 + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

LXR_INTERN unsigned long
lxr_token_parse_oct(const char *p, size_t length)
{
    unsigned long i = 0;
    size_t len = 1;
    while (len < length) {
        i = (i << 3) + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

LXR_INTERN unsigned long
lxr_token_parse_hex(const char *p, size_t length)
{
    unsigned long i = 0;
    size_t len = 2;
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

LXR_INTERN unsigned long
lxr_token_parse_bin(const char *p, size_t length)
{
    unsigned long i = 0;
    size_t len = 2;
    while (len < length) {
        i = (i << 1) + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

LXR_INTERN void
lxr_token_number_value(struct lxr_token *tok)
{
    int i, pow, c;
    const char *p;
    LXR_ASSERT(tok->type == LXR_TOKEN_NUMBER);
    tok->value.i = 0;
    tok->value.f = 0;

    p = tok->str;
    if (tok->subtype & LXR_TOKEN_FLOAT) {
        if (tok->subtype & ((LXR_TOKEN_INFINITE|LXR_TOKEN_INDEFINITE|LXR_TOKEN_NAN))) {
            /* special real number constants */
            union {float f; unsigned int u;} convert;
            if (tok->subtype & LXR_TOKEN_INFINITE) {
                convert.u = 0x7f800000;
                tok->value.f = (double)convert.f;
            } else if (tok->subtype & LXR_TOKEN_INDEFINITE) {
                convert.u = 0xffc00000;
                tok->value.f = (double)convert.f;
            } else if (tok->subtype & LXR_TOKEN_NAN) {
                convert.u = 0x7fc00000;
                tok->value.f = (double)convert.f;
            }
        } else tok->value.f = lxr_token_parse_double(tok->str, tok->len);
        tok->value.i = (unsigned long)tok->value.f;
    } else if (tok->subtype & LXR_TOKEN_DEC) {
        /* paser decimal number */
        tok->value.i = lxr_token_parse_int(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & LXR_TOKEN_OCT) {
        /* parse octal number */
        tok->value.i = lxr_token_parse_oct(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & LXR_TOKEN_HEX) {
        /* parse hex number */
        tok->value.i = lxr_token_parse_hex(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & LXR_TOKEN_BIN) {
        /* parse binary number */
        tok->value.i = lxr_token_parse_bin(p, tok->len);
        tok->value.f = (double)tok->value.i;
    }
    tok->subtype |= LXR_TOKEN_VALIDVAL;
}

static double
lxr_token_to_double(struct lxr_token *tok)
{
    if (tok->type != LXR_TOKEN_NUMBER)
        return 0.0;
    if (!(tok->subtype & LXR_TOKEN_VALIDVAL))
        lxr_token_number_value(tok);
    if (!(tok->subtype & LXR_TOKEN_VALIDVAL))
        return 0.0;
    return tok->value.f;
}

static unsigned long
lxr_token_to_unsigned_long(struct lxr_token *tok)
{
    if (tok->type != LXR_TOKEN_NUMBER)
        return 0;
    if (!(tok->subtype & LXR_TOKEN_VALIDVAL))
        lxr_token_number_value(tok);
    if (!(tok->subtype & LXR_TOKEN_VALIDVAL))
        return 0;
    return tok->value.i;
}

static int
lxr_token_to_int(struct lxr_token *tok)
{
    return (int)lxr_token_to_unsigned_long(tok);
}

static float
lxr_token_to_float(struct lxr_token *tok)
{
    double d = lxr_token_to_double(tok);
    float f = (float)d;;
    return f;
}

/* ---------------------------------------------------------------
 *                          LEXER
 * ---------------------------------------------------------------*/
static void
lxr_init(struct lxr_lexer *lexer, const char *ptr, size_t len,
    const struct lxr_punctuation *punct, lxr_log_f log, void *userdata)
{
    lxr_zero_struct(*lexer);
    lexer->buffer = ptr;
    lexer->current = ptr;
    lexer->last = ptr;
    lexer->end = ptr + len;
    lexer->length = len;
    lexer->line = lexer->last_line = 1;
    if (!punct)
        lexer->puncts = lxr_default_punctuations;
    else lexer->puncts = punct;
    lexer->log = log;
    lexer->userdata = userdata;
}

static void
lxr_reset(struct lxr_lexer *lexer)
{
    lexer->current = lexer->buffer;
    lexer->last = lexer->buffer;
    lexer->line = lexer->last_line = 1;
}

LXR_INTERN int
lxr_read_white_space(struct lxr_lexer *lexer, int current_line)
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
                            lexer->log(lexer->userdata, LXR_WARNING, lexer->line,
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

LXR_INTERN int
lxr_read_esc_chars(struct lxr_lexer *lexer, char *ch)
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
                lexer->log(lexer->userdata, LXR_WARNING, lexer->line,
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
                lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
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
                lexer->log(lexer->userdata, LXR_WARNING, lexer->line,
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

LXR_INTERN int
lxr_read_string(struct lxr_lexer *lexer, struct lxr_token *token, int quote)
{
    size_t tmpline;
    const char *tmp;
    char ch;
    if (quote == '\"')
        token->type = LXR_TOKEN_STRING;
    else token->type = LXR_TOKEN_LITERAL;
    lexer->current++;
    if (lexer->current >= lexer->end)
        return 0;

    token->len = 0;
    token->str = lexer->current;
    while (1) {
        if (*lexer->current == '\\') {
            if (!lxr_read_esc_chars(lexer, &ch))
                return 0;
        } else if (*lexer->current == quote) {
            lexer->current++;
            if (lexer->current >= lexer->end)
                return 0;

            tmp = lexer->current;
            tmpline = lexer->line;
            if (!lxr_read_white_space(lexer, 0)) {
                lexer->current = tmp;
                lexer->line = tmpline;
                break;
            }
            if (*lexer->current == '\0') {
                if (lexer->log) {
                        lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                            "expecting string after '\' terminated line");
                }
                lexer->error = 1;
                return 0;
            }
            break;
        } else {
            if (*lexer->current == '\0') {
                if (lexer->log) {
                    lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                        "missing trailing quote");
                }
                lexer->error = 1;
                return 0;
            }
            if (*lexer->current == '\n') {
                if (lexer->log) {
                    lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                        "newline inside string");
                }
                lexer->error = 1;
                return 0;
            }
            lexer->current++;
        }
    }
    if (token->str) {
        token->len = (size_t)(lexer->current - token->str) - 1;
        if (token->type == LXR_TOKEN_LITERAL)
            token->subtype = (unsigned int)token->str[0];
        else token->subtype = (unsigned int)token->len;
    }
    return 1;
}

LXR_INTERN int
lxr_read_name(struct lxr_lexer *lexer, struct lxr_token *token)
{
    char c;
    token->type = LXR_TOKEN_NAME;
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

LXR_INTERN int
lxr_check_str(const struct lxr_lexer *lexer, const char *str, size_t len)
{
    size_t i;
    for (i = 0; i < len && (&lexer->current[i] < lexer->end); ++i) {
        if (lexer->current[i] != str[i])
            return 0;
    }
    if (i < len) return 0;
    return 1;
}

LXR_INTERN int
lxr_read_number(struct lxr_lexer *lexer, struct lxr_token *token)
{
    size_t i;
    int dot;
    char c, c2;

    token->type = LXR_TOKEN_NUMBER;
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
            token->subtype = LXR_TOKEN_HEX | LXR_TOKEN_INT;
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
            token->subtype = LXR_TOKEN_BIN | LXR_TOKEN_INT;
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
            token->subtype = LXR_TOKEN_OCT | LXR_TOKEN_INT;
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
            token->subtype = LXR_TOKEN_DEC | LXR_TOKEN_FLOAT;
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
                if (lxr_check_str(lexer, "INF", 3))
                    token->subtype  |= LXR_TOKEN_INFINITE;
                else if (lxr_check_str(lexer, "IND", 3))
                    token->subtype  |= LXR_TOKEN_INDEFINITE;
                else if (lxr_check_str(lexer, "NAN", 3))
                    token->subtype  |= LXR_TOKEN_NAN;
                else if (lxr_check_str(lexer, "QNAN", 4)) {
                    token->subtype  |= LXR_TOKEN_NAN; c2++;
                } else if (lxr_check_str(lexer, "SNAN", 4)) {
                    token->subtype  |= LXR_TOKEN_NAN; c2++;
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
            token->subtype = LXR_TOKEN_DEC | LXR_TOKEN_INT;
        }
    }

    if (token->subtype & LXR_TOKEN_FLOAT) {
        /* float point precision subtype */
        if (c > ' ') {
            if (c == 'f' || c == 'F') {
                token->subtype |= LXR_TOKEN_SINGLE_PREC;
                lexer->current++;
            } else token->subtype |= LXR_TOKEN_DOUBLE_PREC;
        } else token->subtype |= LXR_TOKEN_DOUBLE_PREC;
    } else if (token->subtype & LXR_TOKEN_INT) {
        /* integer subtype */
        if (c > ' '){
            for (i = 0; i < 2; ++i) {
                if (c == 'l' || c == 'L')
                    token->subtype |= LXR_TOKEN_LONG;
                else if (c == 'u' || c == 'U')
                    token->subtype |= LXR_TOKEN_UNSIGNED;
                else break;
                if (lexer->current+1 >= lexer->end) break;
                c = *(++lexer->current);
            }
        }
    }
    return 1;
}

LXR_INTERN int
lxr_read_punctuation(struct lxr_lexer *lexer, struct lxr_token *token)
{
    const struct lxr_punctuation *punc;
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
            token->len += (size_t)l;
            lexer->current += l;
            token->type = LXR_TOKEN_PUNCTUATION;
            token->subtype = (unsigned int)punc->id;
            return 1;
        }
    }
    return 0;
}

static int
lxr_read(struct lxr_lexer *lexer, struct lxr_token *token)
{
    int c;
    if (!lexer->current || lexer->current >= lexer->end || lexer->error)
        token->type = LXR_TOKEN_EOS;

    lxr_zero_struct(*token);
    lexer->last = lexer->current;
    lexer->last_line = lexer->line;
    lexer->error = 0;
    if (!lxr_read_white_space(lexer, 0))
        return 0;

    token->line = lexer->line;
    token->line_crossed = (lexer->line - lexer->last_line) ? 1 : 0;

    c = *lexer->current;
    if ((c >= '0' && c <= '9') ||
        (c == '.' && (*(lexer->current + 1)) >= '0' &&
        (c == '.' && (*(lexer->current + 1)) <= '9'))) {
        if (!lxr_read_number(lexer, token))
            return 0;
    } else if (c == '\"' || c == '\'') {
        if (!lxr_read_string(lexer, token, c))
            return 0;
    } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        if (!lxr_read_name(lexer, token))
            return 0;
    } else if ((c == '/' || c == '\\') || c == '.') {
        if (!lxr_read_name(lexer, token))
            return 0;
    } else if (!lxr_read_punctuation(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                "unkown punctuation: %c", c);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

static int
lxr_read_on_line(struct lxr_lexer *lexer, struct lxr_token *token)
{
    struct lxr_token tok;
    if (!lxr_read(lexer, &tok)) {
        lexer->current = lexer->last;
        lexer->line = lexer->last_line;
    }
    if (!tok.line_crossed) {
        *token = tok;
        return 1;
    }
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    lxr_token_clear(token);
    return 0;
}

static int
lxr_expect_string(struct lxr_lexer *lexer, const char *string)
{
    struct lxr_token token;
    if (!lxr_read(lexer, &token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                "failed to read expected token: %s", string);
        }
        lexer->error = 1;
        return 0;
    }
    if (lxr_token_cmp(&token, string)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                "read token is not expected string: %s", string);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

static int
lxr_expect_type(struct lxr_lexer *lexer, enum lxr_token_type type,
    unsigned int subtype, struct lxr_token *token)
{
    if (!lxr_read(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                "could not read expected token with type: %d", type);
        }
        lexer->error = 1;
        return 0;
    }
    if (token->type != type) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                "read token has type %s instead of expected type: %d", token->type, type);
        }
        lexer->error = 1;
        return 0;
    }
    if ((token->subtype & subtype) != subtype) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                "read token has subtype %d instead of expected subtype %d", token->subtype, type);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

static int
lxr_expect_any(struct lxr_lexer *lexer, struct lxr_token *token)
{
    if (!lxr_read(lexer, token)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                "could not read any expected token");
        }
        return 0;
    }
    return 1;
}

static int
lxr_check_string(struct lxr_lexer *lexer, const char *string)
{
    struct lxr_token token;
    if (!lxr_read(lexer, &token))
        return 0;
    if (!lxr_token_cmp(&token, string))
        return 1;

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    return 0;
}

static int
lxr_check_type(struct lxr_lexer *lexer, enum lxr_token_type type,
    unsigned int subtype, struct lxr_token *token)
{
    struct lxr_token tok;
    if (!lxr_read(lexer, &tok))
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

static int
lxr_peek_string(struct lxr_lexer *lexer, const char *string)
{
    struct lxr_token tok;
    if (!lxr_read(lexer, &tok))
        return 0;

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    if (!lxr_token_cmp(&tok, string))
        return 1;
    return 0;
}

static int
lxr_peek_type(struct lxr_lexer *lexer, enum lxr_token_type type,
    unsigned int subtype, struct lxr_token *token)
{
    struct lxr_token tok;
    if (!lxr_read(lexer, &tok))
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

static int
lxr_read_until(struct lxr_lexer *lexer, const char *string, struct lxr_token *token)
{
    int begin = 1;
    struct lxr_token tok;
    token->str = lexer->current;
    while (lxr_read(lexer, &tok)) {
        if (!lxr_token_cmp(&tok, string)) {
            token->len = (size_t)(tok.str - token->str);
            return 1;
        }
    }
    return 0;
}


static int
lxr_skip_until(struct lxr_lexer *lexer, const char *string)
{
    struct lxr_token tok;
    while (lxr_read(lexer, &tok)) {
        if (!lxr_token_cmp(&tok, string))
            return 1;
    }
    return 0;
}

static int
lxr_skip_line(struct lxr_lexer *lexer)
{
    struct lxr_token tok;
    while (lxr_read(lexer, &tok)) {
        if (tok.line_crossed) {
            lexer->current = lexer->last;
            lexer->line = lexer->last_line;
            return 1;
        }
    }
    return 0;
}

static int
lxr_parse_int(struct lxr_lexer *lexer)
{
    struct lxr_token tok;
    if (!lxr_read(lexer, &tok))
        return 0;
    if (tok.type == LXR_TOKEN_PUNCTUATION && tok.str[0] == '-') {
        lxr_expect_type(lexer, LXR_TOKEN_NUMBER, LXR_TOKEN_INT, &tok);
        return -lxr_token_to_int(&tok);
    } else if (tok.type != LXR_TOKEN_NUMBER || tok.subtype == LXR_TOKEN_FLOAT) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                "expected int value but found float");
        }
        lexer->error = 1;
    }
    return lxr_token_to_int(&tok);
}

static int
lxr_parse_bool(struct lxr_lexer *lexer)
{
    struct lxr_token tok;
    if (!lxr_expect_type(lexer, LXR_TOKEN_NUMBER, 0, &tok)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                "could not read expected boolean");
        }
        lexer->error = 1;
        return 0;
    }
    return (lxr_token_to_int(&tok) != 0);
}

static float
lxr_parse_float(struct lxr_lexer *lexer)
{
    struct lxr_token tok;
    if (!lxr_read(lexer, &tok)) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                "could not read expected float number");
        }
        lexer->error = 1;
        return 0;
    }
    if (tok.type == LXR_TOKEN_PUNCTUATION && tok.str[0] == '-') {
        lxr_expect_type(lexer, LXR_TOKEN_NUMBER, 0, &tok);
        return -(float)tok.value.f;
    } else if (tok.type != LXR_TOKEN_NUMBER) {
        if (lexer->log) {
            lexer->log(lexer->userdata, LXR_ERROR, lexer->line,
                "expected float number is not a number");
        }
        lexer->error = 1;
        return 0;
    }
    return lxr_token_to_float(&tok);
}


/* ---------------------------------------------------------------
 *                          UTIL
 * ---------------------------------------------------------------*/
#define ar_free(a)  ((a) ? free(ar_sbraw(a)),0 : 0)
#define ar_push(a,v)(ar_sbmaybegrow(a,1), (a)[ar_sbn(a)++] = (v))
#define ar_count(a) ((a) ? ar_sbn(a) : 0)
#define ar_add(a,n) (ar_sbmaybegrow(a,n), ar_sbn(a)+=(n), &(a)[ar_sbn(a)-(n)])
#define ar_last(a)  ((a)[ar_sbn(a)-1])

#define ar_sbraw(a) ((int *) (a) - 2)
#define ar_sbm(a)   ar_sbraw(a)[0]
#define ar_sbn(a)   ar_sbraw(a)[1]

#define ar_sbneedgrow(a,n)  ((a)==0 || ar_sbn(a)+(n) >= ar_sbm(a))
#define ar_sbmaybegrow(a,n) (ar_sbneedgrow(a,(n)) ? ar_sbgrow(a,n) : 0)
#define ar_sbgrow(a,n)      ((a) = ar_sbgrowf((a), (n), sizeof(*(a))))

static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

static void*
ar_sbgrowf(void *arr, int increment, int itemsize)
{
   int dbl_cur = arr ? 2*ar_sbm(arr) : 0;
   int min_needed = ar_count(arr) + increment;
   int m = dbl_cur > min_needed ? dbl_cur : min_needed;
   int *p = (int *)realloc(arr ? ar_sbraw(arr) : 0, (size_t)itemsize * (size_t)m + sizeof(int)*2);
   if (p) {
      if (!arr)
         p[1] = 0;
      p[0] = m;
      return p+2;
   } else {
      #ifdef STRETCHY_BUFFER_OUT_OF_MEMORY
        die("Out of memory");
      #endif
      return (void *) (2*sizeof(int));
   }
}

static char*
file_load(const char* path, size_t* siz)
{
    char *buf;
    FILE *fd = fopen(path, "rb");
    if (!fd) die("Failed to open file: %s\n", path);
    fseek(fd, 0, SEEK_END);
    *siz = (size_t)ftell(fd);
    fseek(fd, 0, SEEK_SET);
    buf = (char*)calloc(*siz+1, 1);
    fread(buf, *siz, 1, fd);
    fclose(fd);
    buf[*siz] = '\0';
    return buf;
}

#define zero_struct(s) lxr_zero_size(&s, sizeof(s))
#define zero_array(p,n) lxr_zero_size(p, (n) * sizeof((p)[0]))
LXR_INTERN void
zero_size(void *ptr, size_t size)
{
    memset(ptr, 0, size);
}

/* ---------------------------------------------------------------
 *
 *                          PARSE
 *
 * ---------------------------------------------------------------*/
struct meta_type {
    int index;
    char *name;
};

struct meta_value {
    int meta_id;
    char *name;
    int int_value;
    char *str_value;
};

struct meta_enum {
    int index;
    char *name;
    struct meta_value *values;
};

enum meta_flags {
    META_FLAG_POINTER = 0x01,
    META_FLAG_ARRAY = 0x02
};

struct meta_member {
    int type;
    char *name;
    int count;
    unsigned int flags;
};

struct meta_struct {
    int type;
    char *name;
    struct meta_member *members;
};

enum meta_func_visibility {
    META_FUNCTION_STATIC,
    META_FUNCTION_EXTERN
};

struct meta_argument {
    int type;
    char *name;
};

struct meta_function {
    char *name;
    const char *file;
    int line;
    int visibility;
    int ret;
    struct meta_argument *args;
};

struct meta_slot {
    int index;
    char *id;
    char *values;
};

struct meta_table {
    int index;
    char *name;
    char *storage;
    char *format;
    int element_count;
    struct meta_slot *slots;
};

struct meta {
    struct meta_type *types;
    struct meta_struct *structs;
    struct meta_enum *enums;
    struct meta_function *functions;
    struct meta_table *tables;
};

static void
parser_log(void *userdata, enum lxr_log_level lvl, size_t line, const char *msg, ...)
{
    int l = (int)line;
    va_list va;
    va_start(va, msg);
    (void)userdata;
    if (lvl == LXR_WARNING) {
        fprintf(stdout, "WARNING: %d ", l);
        vfprintf(stdout, msg, va);
    } else if (lvl == LXR_ERROR) {
        fprintf(stderr, "ERROR: %d ", l);
        vfprintf(stderr, msg, va);
    }
    fprintf(stdout, "\n");
    va_end(va);
}

static int
parse_add_type(struct meta *meta, struct lxr_token *tok)
{
    int i, cnt = ar_count(meta->types);
    for (i = 0; i < cnt; ++i) {
        if (!lxr_token_cmp(tok, meta->types[i].name))
            break;
    }
    if (i == cnt) {
        struct meta_type type;
        zero_struct(type);
        type.index = i;
        type.name = lxr_token_dup(tok);
        ar_push(meta->types, type);
    }
    return i;
}

static int
parse_member(struct meta *meta, struct meta_struct *meta_struct,
    struct lxr_lexer *lexer, int *concat)
{
    struct meta_member member;
    struct lxr_token tok;
    zero_struct(member);

    if (!*concat) {
        /* first member with current type */
        if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
        if (!lxr_token_cmp(&tok, "struct") || !lxr_token_cmp(&tok, "enum"))
            lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok);
        member.type = parse_add_type(meta, &tok);
    } else member.type = meta_struct->members[ar_count(meta_struct->members)-1].type;

    /* check if pointer */
    if (lxr_check_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_MUL, &tok))
        member.flags |= META_FLAG_POINTER;
    if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;

    member.name = lxr_token_dup(&tok);

    /* check if array */
    if (lxr_check_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_BRACKET_OPEN, &tok)) {
        member.flags |= META_FLAG_ARRAY;
        lxr_expect_type(lexer, LXR_TOKEN_NUMBER, 0, &tok);
        member.count = lxr_token_to_int(&tok);
        lxr_expect_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_BRACKET_CLOSE, &tok);
    } else member.count = 1;


    lxr_expect_any(lexer, &tok);
    if (tok.type == LXR_TOKEN_PUNCTUATION && tok.subtype == LXR_PUNCT_SEMICOLON)
        *concat = 0;
    else if (tok.type == LXR_TOKEN_PUNCTUATION && tok.subtype == LXR_PUNCT_COMMA)
        *concat = 1;
    ar_push(meta_struct->members, member);
    return 1;
}

static int
parse_value(struct meta_enum *meta, struct lxr_lexer *lexer)
{
    struct meta_value value;
    struct lxr_token tok;
    if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
    value.meta_id = 0;
    value.name = lxr_token_dup(&tok);
    if (lxr_check_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_ASSIGN, &tok)) {
        /* normal int enumerator */
        if (!lxr_expect_type(lexer, LXR_TOKEN_NUMBER, 0, &tok)) return 0;
        value.str_value = lxr_token_dup(&tok);
        value.int_value = lxr_token_to_int(&tok);
        meta->index = value.int_value;
    } else if (lxr_check_string(lexer, "as")) {
        /* special string enumerator */
        if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
        if (!lxr_expect_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_PARENTHESE_OPEN, &tok))
            return 0;
        if (!lxr_expect_type(lexer, LXR_TOKEN_STRING, 0, &tok)) return 0;
        value.str_value = lxr_token_dup(&tok);
        value.int_value = meta->index++;
        if (!lxr_expect_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_PARENTHESE_CLOSE, &tok))
            return 0;
    } else {
        /* default index */
        char buffer[1024];
        value.int_value = meta->index++;
        sprintf(buffer, "%d", value.int_value);
        value.str_value = calloc(strlen(buffer)+1, 1);
        strcpy(value.str_value, buffer);
    }
    lxr_check_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_COMMA, &tok);
    ar_push(meta->values, value);
    return 1;
}

static int
parse_argument(struct meta *meta, struct meta_function *meta_fun, struct lxr_lexer *lexer)
{
    struct meta_argument arg;
    struct lxr_token tok;
    if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
    arg.type = parse_add_type(meta, &tok);
    if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
    arg.name = lxr_token_dup(&tok);
    lxr_check_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_COMMA, &tok);
    ar_push(meta_fun->args, arg);
    return 1;
}

static int
parse_introspectable(struct meta *meta, struct lxr_lexer *lexer, const char *file)
{
    struct lxr_token tok;
    if (!lxr_expect_any(lexer, &tok)) return 0;

    if (!lxr_token_cmp(&tok, "typedef"))
        if (!lxr_expect_any(lexer, &tok)) return 0;

    if (!lxr_token_cmp(&tok, "struct")) {
        /* read struct */
        int concat = 0;
        struct meta_struct new_struct;
        zero_struct(new_struct);
        if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
        new_struct.name = lxr_token_dup(&tok);
        new_struct.type = parse_add_type(meta, &tok);

        if (!lxr_expect_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_BRACE_OPEN, &tok)) return 0;
        while (1) {
            parse_member(meta, &new_struct, lexer, &concat);
            if (lxr_check_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_BRACE_CLOSE, &tok))
                break;
        }
        ar_push(meta->structs, new_struct);
    } else if (!lxr_token_cmp(&tok, "enum")) {
        /* read enum */
        struct meta_enum new_enum;
        zero_struct(new_enum);
        if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
        new_enum.name = lxr_token_dup(&tok);
        if (!lxr_expect_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_BRACE_OPEN, &tok)) return 0;
        while (1) {
            parse_value(&new_enum, lexer);
            if (lxr_check_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_BRACE_CLOSE, &tok))
                break;
        }
        ar_push(meta->enums, new_enum);
    } else {
        /* read function */
        struct meta_function meta_fun;
        zero_struct(meta_fun);
        if (!lxr_token_cmp(&tok, "static")) {
            meta_fun.visibility = META_FUNCTION_STATIC;
            if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
        }
        else if (!lxr_token_cmp(&tok, "extern")) {
            meta_fun.visibility = META_FUNCTION_EXTERN;
            if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
        }
        meta_fun.ret = parse_add_type(meta, &tok);
        if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
        meta_fun.name = lxr_token_dup(&tok);
        meta_fun.file = file;
        meta_fun.line = (int)tok.line;
        if (!lxr_expect_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_PARENTHESE_OPEN, &tok)) return 0;
        while (1) {
            if (lxr_check_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_PARENTHESE_CLOSE, &tok))
                break;
            parse_argument(meta, &meta_fun, lexer);
        }
        ar_push(meta->functions, meta_fun);
    }
    return 1;
}

static int
parse_slot(struct meta_table *db, struct lxr_lexer *lexer)
{
    size_t i = 0;
    struct lxr_token tok;
    struct meta_slot slot;

    zero_struct(slot);
    lxr_expect_string(lexer, "meta_slot");
    if (!lxr_expect_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_PARENTHESE_OPEN, &tok)) return 0;
    if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
    slot.index = db->index++;
    slot.id = lxr_token_dup(&tok);
    if (!lxr_expect_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_COMMA, &tok)) return 0;
    if (!lxr_read_until(lexer, ")", &tok)) return 0;
    slot.values = lxr_token_dup(&tok);
    for (i = 0; i < tok.len; ++i) {
        if (slot.values[i] == ';')
            slot.values[i] = ',';
    }
    if (!lxr_check_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_COMMA, &tok)) return 0;
    ar_push(db->slots, slot);
    return 1;
}

static int
parse_table(struct meta *meta, struct lxr_lexer *lexer)
{
    size_t i = 0;
    struct lxr_token tok;
    struct meta_table db;

    zero_struct(db);
    if (!lxr_expect_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_PARENTHESE_OPEN, &tok)) return 0;
    if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
    db.storage = lxr_token_dup(&tok);
    if (!lxr_expect_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_COMMA, &tok)) return 0;
    if (!lxr_read_until(lexer, ")", &tok)) return 0;
    db.format = lxr_token_dup(&tok);
    for (i = 0; i < tok.len; ++i) {
        if (tok.str[i] == ';')
            db.element_count++;
    }
    db.element_count++;
    if (!lxr_expect_type(lexer, LXR_TOKEN_NAME, 0, &tok)) return 0;
    db.name = lxr_token_dup(&tok);

    if (!lxr_expect_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_BRACE_OPEN, &tok)) return 0;
    while (1) {
        if (lxr_check_type(lexer, LXR_TOKEN_PUNCTUATION, LXR_PUNCT_BRACE_CLOSE, &tok))
            break;
        parse_slot(&db, lexer);
    }
    ar_push(meta->tables, db);
    return 1;
}

static void
parse_file(struct meta *meta, const char *file_data, size_t size, const char *file)
{
    struct lxr_lexer lexer;
    lxr_init(&lexer, file_data, size, 0, parser_log, 0);
    while (!lexer.error) {
        struct lxr_token tok;
        if (!lxr_read(&lexer, &tok))
            break;

        if (tok.type == LXR_TOKEN_NAME) {
            if (!lxr_token_icmp(&tok, "meta_introspect"))
                parse_introspectable(meta, &lexer, file);
            else if (!lxr_token_icmp(&tok, "meta_table"))
                parse_table(meta, &lexer);
        } else if (tok.type == LXR_TOKEN_EOS) break;;
    }
}

/* ---------------------------------------------------------------
 *
 *                          GENERATE
 *
 * ---------------------------------------------------------------*/
static void
generate_header(FILE *out, struct meta *meta)
{
    int i = 0;
    fprintf(out, "#ifndef META_H_\n");
    fprintf(out, "#define META_H_\n\n");
    fprintf(out, "#define as\n");
    fprintf(out, "#define meta(x)\n");
    fprintf(out, "#define meta_slot(n,x) n\n");
    fprintf(out, "#define meta_introspect\n");
    fprintf(out, "#define meta_table(n,x) enum\n\n");
    fprintf(out, "#ifdef META_STATIC\n");
    fprintf(out, "#define META_API static\n");
    fprintf(out, "#else\n");
    fprintf(out, "#define META_API extern\n");
    fprintf(out, "#endif\n");
    fprintf(out, "enum meta_type {\n");
    for (i = 0; i < ar_count(meta->types); ++i)
        fprintf(out, "    META_TYPE_%s,\n", meta->types[i].name);
    fprintf(out, "};\n\n");
    fprintf(out, "enum meta_member_flags {\n");
    fprintf(out, "    META_MEMBER_FLAG_POINTER   = 0x01,\n");
    fprintf(out, "    META_MEMBER_FLAG_ARRAY     = 0x02\n");
    fprintf(out, "};\n\n");
    fprintf(out, "struct meta_member {\n");
    fprintf(out, "    enum meta_type type;\n");
    fprintf(out, "    const char *name;\n");
    fprintf(out, "    int count;\n");
    fprintf(out, "    unsigned int flags;\n");
    fprintf(out, "    unsigned int offset;\n");
    fprintf(out, "};\n\n");
    fprintf(out, "struct meta_struct {\n");
    fprintf(out, "   enum meta_type type;\n");
    fprintf(out, "   const char *name;\n");
    fprintf(out, "   size_t size;\n");
    fprintf(out, "   int member_count;\n");
    fprintf(out, "   const struct meta_member *def;\n");
    fprintf(out, "};\n\n");
    fprintf(out, "struct meta_enum_value {\n");
    fprintf(out, "   int id;\n");
    fprintf(out, "   const char *name;\n");
    fprintf(out, "   int int_value;\n");
    fprintf(out, "   const char *str_value;\n");
    fprintf(out, "};\n\n");
    fprintf(out, "struct meta_enum {\n");
    fprintf(out, "   const char *name;\n");
    fprintf(out, "   int max_id;\n");
    fprintf(out, "   int value_count;\n");
    fprintf(out, "   const struct meta_enum_value *values;\n");
    fprintf(out, "};\n\n");
    fprintf(out, "enum meta_function_visbility {\n");
    fprintf(out, "   META_FUNCTION_STATIC,\n");
    fprintf(out, "   META_FUNCTION_EXTERN\n");
    fprintf(out, "};\n\n");
    fprintf(out, "struct meta_argument {\n");
    fprintf(out, "   enum meta_type type;\n");
    fprintf(out, "   const char *name;\n");
    fprintf(out, "};\n\n");
    fprintf(out, "struct meta_function {\n");
    fprintf(out, "   const char *name;\n");
    fprintf(out, "   const char *file;\n");
    fprintf(out, "   int line;\n");
    fprintf(out, "   enum meta_function_visbility visbility;\n");
    fprintf(out, "   enum meta_type return_type;\n");
    fprintf(out, "   void *function;\n");
    fprintf(out, "   int argc;\n");
    fprintf(out, "   const struct meta_argument *args;\n");
    fprintf(out, "};\n\n");
    fprintf(out, "struct meta_table {\n");
    fprintf(out, "   const char *name;\n");
    fprintf(out, "   const char *type;\n");
    fprintf(out, "   int slot_count;\n");
    fprintf(out, "   const void *slots;\n");
    fprintf(out, "};\n\n");
    for (i = 0; i < ar_count(meta->tables); ++i) {
        fprintf(out, "struct %s {\n", meta->tables[i].storage);
        fprintf(out, "    int index;%s;\n", meta->tables[i].format);
        fprintf(out, "};\n\n");
    }

    fprintf(out, "META_API const struct meta_struct *meta_struct_from_name(const char*);\n");
    fprintf(out, "META_API const struct meta_member *meta_member_from_name(const char*, const char*);\n");
    fprintf(out, "META_API const struct meta_struct *meta_struct_from_id(enum meta_type);\n");
    fprintf(out, "META_API const struct meta_member *meta_member_from_id(enum meta_type, const char*);\n");
    fprintf(out, "META_API const struct meta_enum *meta_enum_from_string(const char *enumerator);\n");
    fprintf(out, "META_API void *meta_member_ptr_from_name(void *obj, const char *type, const char *member);\n");
    fprintf(out, "META_API void *meta_member_ptr_from_id(void *obj, enum meta_type, const char *member);\n");
    fprintf(out, "META_API int meta_enum_value_from_string(const char *enumerator, const char *id);\n\n");

    fprintf(out, "#define meta_enum_str(x,v) meta_enum_values_of_##x[v].str_value\n");
    fprintf(out, "#define meta_enum_name(x,v) meta_enum_values_of_##x[v].name\n");
    fprintf(out, "#define meta_query(x,v) &meta_table_slots_of_##x[v]\n\n");

    for (i = 0; i < ar_count(meta->structs); ++i)
        fprintf(out, "META_API const struct meta_member meta_members_of_%s[%d];\n",
            meta->structs[i].name, ar_count(meta->structs[i].members)+1);
    for (i = 0; i < ar_count(meta->enums); ++i)
        fprintf(out, "META_API const struct meta_enum_value meta_enum_values_of_%s[%d];\n",
            meta->enums[i].name, ar_count(meta->enums[i].values)+1);
    for (i = 0; i < ar_count(meta->functions); ++i)
        fprintf(out, "META_API const struct meta_argument meta_function_args_of_%s[%d];\n",
            meta->functions[i].name, ar_count(meta->functions[i].args)+1);
    for (i = 0; i < ar_count(meta->tables); ++i)
        fprintf(out, "META_API const struct %s meta_table_slots_of_%s[%d];\n",
            meta->tables[i].storage, meta->tables[i].name, ar_count(meta->tables[i].slots));
    fprintf(out, "META_API const struct meta_struct meta_structs[%d];\n", ar_count(meta->structs)+1);
    fprintf(out, "META_API const struct meta_enum meta_enums[%d];\n", ar_count(meta->enums)+1);
    fprintf(out, "META_API const struct meta_function meta_functions[%d];\n", ar_count(meta->functions)+1);
    fprintf(out, "META_API const struct meta_table meta_tables[%d];\n", ar_count(meta->tables)+1);

    fprintf(out, "#endif\n\n");
}

static void
generate_meta_members(FILE *out, struct meta_struct *meta, struct meta_type *types)
{
    int i = 0;
    for (i = 0; i < ar_count(meta->members); ++i) {
        fprintf(out, "    {META_TYPE_%s, \"%s\", ", types[meta->members[i].type].name, meta->members[i].name);
        fprintf(out, "%d, ", meta->members[i].count);
        if (!meta->members[i].flags)
            fprintf(out, "0,");
        else {
            if (meta->members[i].flags & META_FLAG_POINTER) {
                if (meta->members[i].flags & META_FLAG_ARRAY)
                    fprintf(out, "META_MEMBER_FLAG_POINTER|META_MEMBER_FLAG_ARRAY, ");
                else fprintf(out, "META_MEMBER_FLAG_POINTER, ");
            }
            else if (meta->members[i].flags & META_FLAG_ARRAY)
                fprintf(out, "META_MEMBER_FLAG_ARRAY, ");
        }
        fprintf(out, "(size_t)(&((struct %s*)0)->%s)", meta->name, meta->members[i].name);
        fprintf(out, "},\n");
    }
}

static void
generate_meta_enum_values(FILE *out, struct meta_enum *meta)
{
    int i = 0;
    for (i = 0; i < ar_count(meta->values); ++i) {
        fprintf(out, "    {%s, \"%s\", %d, \"%s\"},\n",
            meta->values[i].name, meta->values[i].name,
            meta->values[i].int_value, meta->values[i].str_value);
    }
}

static void
generate_meta_function_args(FILE *out, struct meta_function *meta, struct meta_type *types)
{
    int i = 0;
    for (i = 0; i < ar_count(meta->args); ++i)
        fprintf(out, "    {META_TYPE_%s, \"%s\"},\n",
            types[meta->args[i].type].name, meta->args[i].name);
}

static void
generate_meta_table_slots(FILE *out, struct meta_table *meta)
{
    int i = 0;
    for (i = 0; i < ar_count(meta->slots); ++i)
        fprintf(out, "    {%s, %s},\n",
            meta->slots[i].id, meta->slots[i].values);
}


static void
generate_meta_data(FILE *out, struct meta *meta)
{
    int i = 0;
    for (i = 0; i < ar_count(meta->structs); ++i) {
        fprintf(out, "const struct meta_member meta_members_of_%s[] = {\n", meta->structs[i].name);
        generate_meta_members(out, &meta->structs[i], meta->types);
        fprintf(out, "    {0,0,0,0,0}\n");
        fprintf(out, "};\n");
    }
    fprintf(out, "\n");

    for (i = 0; i < ar_count(meta->enums); ++i) {
        fprintf(out, "const struct meta_enum_value meta_enum_values_of_%s[] = {\n", meta->enums[i].name);
        generate_meta_enum_values(out, &meta->enums[i]);
        fprintf(out, "    {0,0,0,0}\n");
        fprintf(out, "};\n");
    }
    fprintf(out, "\n");

    for (i = 0; i < ar_count(meta->functions); ++i) {
        fprintf(out, "const struct meta_argument meta_function_args_of_%s[] = {\n", meta->functions[i].name);
        generate_meta_function_args(out, &meta->functions[i], meta->types);
        fprintf(out, "    {0,0}\n");
        fprintf(out, "};\n");
    }
    fprintf(out, "\n");

    for (i = 0; i < ar_count(meta->tables); ++i) {
        fprintf(out, "const struct %s meta_table_slots_of_%s[] = {\n",
            meta->tables[i].storage, meta->tables[i].name);
        generate_meta_table_slots(out, &meta->tables[i]);
        fprintf(out, "};\n");
    }
    fprintf(out, "\n");

    fprintf(out, "const struct meta_struct meta_structs[] = {\n");
    for (i = 0; i < ar_count(meta->structs); ++i) {
        fprintf(out, "    {META_TYPE_%s, \"%s\", sizeof(struct %s), %d, &meta_members_of_%s[0]},\n",
            meta->types[meta->structs[i].type].name, meta->structs[i].name,
            meta->structs[i].name, ar_count(meta->structs[i].members), meta->structs[i].name);
    }
    fprintf(out, "    {0,0,0,0,0}\n");
    fprintf(out, "};\n\n");

    fprintf(out, "const struct meta_enum meta_enums[] = {\n");
    for (i = 0; i < ar_count(meta->enums); ++i) {
        fprintf(out, "    {\"%s\", %d, %d, &meta_enum_values_of_%s[0]},\n",
            meta->enums[i].name, meta->enums[i].index, ar_count(meta->enums[i].values), meta->enums[i].name);
    }
    fprintf(out, "    {0,0,0,0}\n");
    fprintf(out, "};\n\n");

    fprintf(out, "const struct meta_function meta_functions[] = {\n");
    for (i = 0; i < ar_count(meta->functions); ++i) {
        fprintf(out, "    {\"%s\", ", meta->functions[i].name);
        fprintf(out, "\"%s\", ", meta->functions[i].file);
        fprintf(out, "%d, ", meta->functions[i].line);
        fprintf(out, "META_FUNCTION_%s, ",
            (meta->functions[i].visibility == META_FUNCTION_STATIC)? "STATIC": "EXTERN");
        fprintf(out, "META_TYPE_%s, ", meta->types[meta->functions[i].ret].name);
        fprintf(out, "%s, ", meta->functions[i].name);
        fprintf(out, "%d, ", ar_count(meta->functions[i].args));
        fprintf(out, "&meta_function_args_of_%s[0]},\n", meta->functions[i].name);
    }
    fprintf(out, "    {0,0,0,0,0,0,0}\n");
    fprintf(out, "};\n\n");

    fprintf(out, "const struct meta_table meta_tables[] = {\n");
    for (i = 0; i < ar_count(meta->tables); ++i) {
        fprintf(out, "    {\"%s\", \"%s\", %d, &meta_table_slots_of_%s[0]},\n",
            meta->tables[i].name, meta->tables[i].storage,
            ar_count(meta->tables[i].slots), meta->tables[i].name);
    }
    fprintf(out, "    {0,0,0}\n");
    fprintf(out, "};\n\n");

}

static void
generate_meta_functions(FILE *out)
{
    fprintf(out, "const struct meta_struct*\n");
    fprintf(out, "meta_struct_from_name(const char *name)\n");
    fprintf(out, "{\n");
    fprintf(out, "    const struct meta_struct *iter = &meta_structs[0];\n");
    fprintf(out, "    while (iter->name) {\n");
    fprintf(out, "        if (!strcmp(iter->name, name))\n");
    fprintf(out, "            return iter;\n");
    fprintf(out, "        iter++;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
    fprintf(out, "const struct meta_struct*\n");
    fprintf(out, "meta_struct_from_id(enum meta_type type)\n");
    fprintf(out, "{\n");
    fprintf(out, "    const struct meta_struct *iter = &meta_structs[0];\n");
    fprintf(out, "    while (iter->name) {\n");
    fprintf(out, "        if (iter->type == type)\n");
    fprintf(out, "            return iter;\n");
    fprintf(out, "        iter++;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
    fprintf(out, "const struct meta_member*\n");
    fprintf(out, "meta_member_from_name(const char *str, const char *member)\n");
    fprintf(out, "{\n");
    fprintf(out, "    const struct meta_member *iter;\n");
    fprintf(out, "    const struct meta_struct *struct_def = meta_struct_from_name(str);\n");
    fprintf(out, "    if (!struct_def) return 0;\n");
    fprintf(out, "    iter = struct_def->def;\n");
    fprintf(out, "    while (iter->name) {\n");
    fprintf(out, "        if (!strcmp(iter->name, member))\n");
    fprintf(out, "            return iter;\n");
    fprintf(out, "        iter++;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
    fprintf(out, "const struct meta_member*\n");
    fprintf(out, "meta_member_from_id(enum meta_type type, const char *member)\n");
    fprintf(out, "{\n");
    fprintf(out, "    const struct meta_member *iter;\n");
    fprintf(out, "    const struct meta_struct *struct_def = meta_struct_from_id(type);\n");
    fprintf(out, "    if (!struct_def) return 0;\n");
    fprintf(out, "    iter = struct_def->def;\n");
    fprintf(out, "    while (iter->name) {\n");
    fprintf(out, "        if (!strcmp(iter->name, member))\n");
    fprintf(out, "            return iter;\n");
    fprintf(out, "        iter++;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
    fprintf(out, "void*\n");
    fprintf(out, "meta_member_ptr_from_name(void *obj, const char *type, const char *name)\n");
    fprintf(out, "{\n");
    fprintf(out, "    const struct meta_member *member = meta_member_from_name(type, name);\n");
    fprintf(out, "    if (!member) return 0;\n");
    fprintf(out, "    return (unsigned char*)obj + member->offset;\n");
    fprintf(out, "}\n");
    fprintf(out, "void*\n");
    fprintf(out, "meta_member_ptr_from_id(void *obj, enum meta_type type, const char *id)\n");
    fprintf(out, "{\n");
    fprintf(out, "    const struct meta_member *member = meta_member_from_id(type, id);\n");
    fprintf(out, "    if (!member) return 0;\n");
    fprintf(out, "    return (unsigned char*)obj + member->offset;\n");
    fprintf(out, "}\n");
    fprintf(out, "META_API const struct meta_enum *meta_enum_from_string(const char *enumerator)\n");
    fprintf(out, "{\n");
    fprintf(out, "    const struct meta_enum *iter = &meta_enums[0];\n");
    fprintf(out, "    while (iter->name) {\n");
    fprintf(out, "        if (!strcmp(iter->name, enumerator))\n");
    fprintf(out, "            return iter;\n");
    fprintf(out, "        iter++;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
    fprintf(out, "META_API int meta_enum_value_from_string(const char *enums, const char *id)\n\n");
    fprintf(out, "{\n");
    fprintf(out, "    const struct meta_enum_value *iter;\n");
    fprintf(out, "    const struct meta_enum *e = meta_enum_from_string(enums);\n");
    fprintf(out, "    if (!e) return -1;\n");
    fprintf(out, "    iter = e->values;\n");
    fprintf(out, "    while (iter->name) {\n");
    fprintf(out, "        if (!strcmp(iter->name, id))\n");
    fprintf(out, "            return iter->id;\n");
    fprintf(out, "        iter++;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return -1;\n");
    fprintf(out, "}\n\n");
}

static void
generate_implementation(FILE *out, struct meta *meta)
{
    fprintf(out, "#ifdef META_IMPLEMENTATION\n\n");
    generate_meta_data(out, meta);
    generate_meta_functions(out);
    fprintf(out, "#endif\n\n");
}

static void generate_output(FILE *out, struct meta *meta)
{
    generate_header(out, meta);
    generate_implementation(out, meta);
}

int
main(int argc, char **argv)
{
    int i = 0;
    FILE *output;
    char *file;
    size_t size;
    struct meta meta;
    const char *out = 0;
    if (argc < 2)
        die("usage: %s <files>\n", argv[0]);

    zero_struct(meta);
    for (i = 1; i < argc; ++i) {
        file = file_load(argv[i], &size);
        parse_file(&meta, file, size, argv[i]);
        free(file);
    }

    output = fopen("meta.h", "w");
    if (!output) die("failed to open the output file\n");
    generate_output(output, &meta);
    fclose(output);
    return 0;
}

