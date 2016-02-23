/*
    mm_meta.h - zlib - Micha Mettke

ABOUT:
    This is a source code meta data generator which parses source file(s) and
    processes and stores source code information and output a single header file
    which if included allows some extensive features for C (like reflection).

QUICK:
    To use this file do:
    #define MM_META_IMPLEMENTATION
    before you include this file in *one* C or C++ file to create the implementation

    If you want to keep the implementation in that file you have to do
    #define MM_META_STATIC before including this file

    If you want to use asserts to add validation add
    #define MM_META_ASSERT before including this file

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
    MM_META_IMPLEMENTATION
        Generates the implementation of the library into the included file.
        If not provided the library is in header only mode and can be included
        in other headers or source files without problems. But only ONE file
        should hold the implementation.

    MM_META_STATIC
        The generated implementation will stay private inside implementation
        file and all internal symbols and functions will only be visible inside
        that file.

    MM_META_ASSERT
    MM_META_USE_ASSERT
        If you define MM_META_USE_ASSERT without defining MM_ASSERT mm_meta.h
        will use assert.h and assert(). Otherwise it will use your assert
        method. If you do not define MM_META_USE_ASSERT no additional checks
        will be added. This is the only C standard library function used
        by mm_meta.

    MM_META_MEMSET
        You can define this to 'memset' or your own memset replacement.
        If not mm_meta uses a naive (maybe inefficent) implementation.

    MM_META_MEMCPY
        You can define this to 'memcpy' or your own memset replacement.
        If not mm_meta uses a naive (maybe inefficent) implementation.

EXAMPLES:*/
#if 0
/* ---------------------------- meta_test_file.c ---------------------------*/
#include "meta.h" /* generated */

meta_introspect struct test {
    char c; short s;
    int i; long l;
    int *pi; long al[16];
};
meta_introspect enum state {
    STATE_WAITING,
    STATE_RUNNING,
    STATE_HALTED
};
meta_introspect enum punctuation {
    PUNCT_STAR as meta("*"),
    PUNCT_COMMA as meta(","),
    PUNCT_PLUS as meta("+")
};
meta_table(weapon, const char *name; int damage; int weight; int munition) weapons {
    meta_slot(WEAPON_PISTOL, "Pistol"; 2; 10; 200),
    meta_slot(WEAPON_SHOTGUN,  "Shotgun"; 4; 20; 100),
    meta_slot(WEAPON_PLASMA, "Plasma"; 5; 25; 250),
    meta_slot(WEAPON_RAILGUN, "Railgun"; 10; 26; 100),
    meta_slot(WEAPON_MAX, NULL; 0; 0; 0)
};
meta_introspect static int function(int i, char c, long l){return i + c + l;}
int main(void)
{
    /* reflection */
    int *i;
    struct test t;
    void *void_ptr = &t;
    t.i = 5;
    fprintf(stdout, "i: %d\n", t.i);
    i = meta_member_ptr_from_name(void_ptr, "test", "i");
    fprintf(stdout, "i: %d\n", *i);
    *i = 10;
    fprintf(stdout, "i: %d\n", t.i);
}

/*-------------------------------- parser.c ----------------------------*/
static void test_log(void*, enum mm_meta_log_level, int, const char*, ...);
struct mm_meta_info meta;
mm_meta_init(&meta, test_log, 0);
mm_meta_load(&meta, "meta_test_file.c");
mm_meta_generate("meta.h", &meta);
mm_meta_free(&meta);
return 0;

#endif
 /* ===============================================================
 *
 *                          HEADER
 *
 * =============================================================== */
#ifndef MM_META_H_
#define MM_META_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifdef MM_META_STATIC
#define MM_META_API static
#else
#define MM_META_API extern
#endif

#ifndef MM_META_BUFFER_SIZE
#define MM_META_BUFFER_SIZE (64 * 1024)
#endif

struct mm_meta_type {
    int index;
    char *name;
};

struct mm_meta_value {
    int id;
    char *name;
    int int_value;
    char *str_value;
};

struct mm_meta_enum {
    int index;
    char *name;
    struct mm_meta_value *values;
};

enum mm_meta_flags {
    META_FLAG_POINTER = 0x01,
    META_FLAG_ARRAY = 0x02
};

struct mm_meta_member {
    int type;
    char *name;
    int count;
    unsigned int flags;
};

struct mm_meta_struct {
    int type;
    char *name;
    struct mm_meta_member *members;
};

enum mm_meta_func_visibility {
    META_FUNCTION_STATIC,
    META_FUNCTION_EXTERN
};

struct mm_meta_argument {
    int type;
    char *name;
};

struct mm_meta_function {
    struct mm_meta_argument *args;
    char *name;
    const char *file;
    int line;
    int visibility;
    int ret;
};

struct mm_meta_slot {
    int index;
    char *id;
    char *values;
};

struct mm_meta_table {
    int index;
    char *name;
    char *storage;
    char *format;
    int element_count;
    struct mm_meta_slot *slots;
};

enum mm_meta_log_level {MM_META_WARNING,MM_META_ERROR};
typedef void(*mm_meta_log_f)(void*, enum mm_meta_log_level,
                            int line, const char *msg, ...);

struct mm_meta_buffer {
    struct mm_meta_buffer *next;
    char memory[MM_META_BUFFER_SIZE];
    int capacity;
    int size;
};

struct mm_meta_info {
    struct mm_meta_type *types;
    struct mm_meta_struct *structs;
    struct mm_meta_enum *enums;
    struct mm_meta_function *functions;
    struct mm_meta_table *tables;
    struct mm_meta_buffer *memory;
    mm_meta_log_f log;
    void *userdata;
};

MM_META_API void mm_meta_init(struct mm_meta_info*, mm_meta_log_f, void *userdata);
MM_META_API int mm_meta_load_from_memory(struct mm_meta_info*, const char*, const char*, int);
MM_META_API int mm_meta_load(struct mm_meta_info*, const char*);
MM_META_API void mm_meta_free(struct mm_meta_info *meta);

MM_META_API int mm_meta_generate(const char *file, const struct mm_meta_info*);
MM_META_API void mm_meta_generate_to(FILE*, const struct mm_meta_info*);

#ifdef __cplusplus
}
#endif
#endif /* MM_META_H_ */

/* ===============================================================
 *
 *                      IMPLEMENTATION
 *
 * ===============================================================*/
#ifdef MM_META_IMPLEMENTATION
#define MM_META_LEN(a) (sizeof(a)/sizeof((a)[0]))
#define MM_META_UNUSED(a) ((void)(a))

#ifdef MM_META_USE_ASSERT
#ifndef MM_META_ASSERT
#include <assert.h>
#define MM_META_ASSERT(expr) assert(expr)
#endif
#else
#define MM_META_ASSERT(expr)
#endif

/* Pointer to Integer type conversion for pointer alignment */
#if defined(__PTRDIFF_TYPE__) /* This case should work for GCC*/
# define MM_META_UINT_TO_PTR(x) ((void*)(__PTRDIFF_TYPE__)(x))
# define MM_META_PTR_TO_UINT(x) (__PTRDIFF_TYPE__)(x)
#elif !defined(__GNUC__) /* works for compilers other than LLVM */
# define MM_META_UINT_TO_PTR(x) ((void*)&((char*)0)[x])
# define MM_META_PTR_TO_UINT(x) ((unsigned int)(((char*)x)-(char*)0))
#elif defined(MM_META_USE_FIXED_TYPES) /* used if we have <stdint.h> */
# define MM_META_UINT_TO_PTR(x) ((void*)(uintptr_t)(x))
# define MM_META_PTR_TO_UINT(x) ((uintptr_t)(x))
#else /* generates warning but works */
# define MM_META_UINT_TO_PTR(x) ((void*)(x))
# define MM_META_PTR_TO_UINT(x) ((unsigned int)(x))
#endif

#define MM_META_INTERN static
#define MM_META_GLOBAL static
#define MM_META_STORAGE static

#ifndef MM_META_MEMCPY
#define MM_META_MEMCPY mm_meta_memcpy
#endif
#ifndef MM_META_MEMSET
#define MM_META_MEMSET mm_meta_memset
#endif
#ifndef MM_META_MEMCPY
#define MM_META_MEMCPY mm_meta_memcpy
#endif
#ifndef MM_META_MALLOC
#include <stdlib.h>
#define MM_META_MALLOC(sz) malloc(sz)
#define MM_META_REALLOC(p,sz) realloc(p, sz)
#define MM_META_FREE(p) free(p)
#endif

/* ---------------------------------------------------------------
 *
 *                          UTIL
 *
 * ---------------------------------------------------------------*/
MM_META_INTERN char
mm_meta_char_upper(char c)
{
    if (c >= 'a' && c <= 'z')
        return (char)('A' + (c - 'a'));
    return c;
}

MM_META_INTERN void*
mm_meta_memcpy(void *dst0, const void *src0, int length)
{
    int t;
    typedef int word;
    char *dst = dst0;
    const char *src = src0;
    if (length == 0 || dst == src)
        goto done;

    #define wsize ((int)sizeof(word))
    #define wmask (wsize-1)
    #define TLOOP(s) if (t) TLOOP1(s)
    #define TLOOP1(s) do { s; } while (--t)

    if (dst < src) {
        t = (int)src; /* only need low bits */
        if ((t | (int)dst) & wmask) {
            if ((t ^ (int)dst) & wmask || length < wsize)
                t = length;
            else
                t = wsize - (t & wmask);
            length -= t;
            TLOOP1(*dst++ = *src++);
        }
        t = length / wsize;
        TLOOP(*(word*)(void*)dst = *(const word*)(const void*)src;
            src += wsize; dst += wsize);
        t = length & wmask;
        TLOOP(*dst++ = *src++);
    } else {
        src += length;
        dst += length;
        t = (int)src;
        if ((t | (int)dst) & wmask) {
            if ((t ^ (int)dst) & wmask || length <= wsize)
                t = length;
            else
                t &= wmask;
            length -= t;
            TLOOP1(*--dst = *--src);
        }
        t = length / wsize;
        TLOOP(src -= wsize; dst -= wsize;
            *(word*)(void*)dst = *(const word*)(const void*)src);
        t = length & wmask;
        TLOOP(*--dst = *--src);
    }
    #undef wsize
    #undef wmask
    #undef TLOOP
    #undef TLOOP1
done:
    return (dst0);
}

MM_META_INTERN void
mm_meta_memset(void *ptr, int c0, int size)
{
    #define word unsigned
    #define wsize ((int)sizeof(word))
    #define wmask (wsize - 1)
    unsigned char *dst = (unsigned char*)ptr;
    unsigned c = 0;
    int t = 0;

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
    if ((t = (int)dst & wmask) != 0) {
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

#define mm_meta_zero_struct(s) mm_meta_zero_size(&s, sizeof(s))
#define mm_meta_zero_array(p,n) mm_meta_zero_size(p, (n) * sizeof((p)[0]))
MM_META_INTERN void
mm_meta_zero_size(void *ptr, int size)
{
    MM_META_MEMSET(ptr, 0, size);
}

/* ---------------------------------------------------------------
 *
 *                          ARRAY
 *
 * ---------------------------------------------------------------*/
#define mm_ar_free(a)  ((a) ? MM_META_FREE(mm_ar_sbraw(a)),0 : 0)
#define mm_ar_push(a,v)(mm_ar_sbmaybegrow(a,1), (a)[mm_ar_sbn(a)++] = (v))
#define mm_ar_count(a) ((a) ? mm_ar_sbn(a) : 0)
#define mm_ar_add(a,n) (mm_ar_sbmaybegrow(a,n), mm_ar_sbn(a)+=(n), &(a)[mm_ar_sbn(a)-(n)])
#define mm_ar_last(a)  ((a)[mm_ar_sbn(a)-1])

#define mm_ar_sbraw(a) ((int *) (a) - 2)
#define mm_ar_sbm(a)   mm_ar_sbraw(a)[0]
#define mm_ar_sbn(a)   mm_ar_sbraw(a)[1]

#define mm_ar_sbneedgrow(a,n)  ((a)==0 || mm_ar_sbn(a)+(n) >= mm_ar_sbm(a))
#define mm_ar_sbmaybegrow(a,n) (mm_ar_sbneedgrow(a,(n)) ? mm_ar_sbgrow(a,n) : 0)
#define mm_ar_sbgrow(a,n)      ((a) = mm_ar_sbgrowf((a), (n), sizeof(*(a))))

MM_META_INTERN void*
mm_ar_sbgrowf(void *arr, int increment, int itemsize)
{
   int dbl_cur = arr ? 2*mm_ar_sbm(arr) : 0;
   int min_needed = mm_ar_count(arr) + increment;
   int m = dbl_cur > min_needed ? dbl_cur : min_needed;
   int *p = (int *)MM_META_REALLOC(arr ? mm_ar_sbraw(arr) : 0,
        ((int)itemsize * (int)m + (int)(sizeof(int)*2)));

   if (p) {
      if (!arr)
         p[1] = 0;
      p[0] = m;
      return p+2;
   } else {
      #ifdef MM_STRETCHY_BUFFER_OUT_OF_MEMORY
        MM_META_ASSERT(!"Out of memory");
      #endif
      return (void *) (2*sizeof(int));
   }
}

/* ---------------------------------------------------------------
 *
 *                          LEXER
 *
 * ---------------------------------------------------------------*/
#define MM_META_DEFAULT_PUNCTION_MAP(PUNCTUATION)\
    PUNCTUATION(">>=",  MM_META_PUNCT_RSHIFT_ASSIGN)\
    PUNCTUATION("<<=",  MM_META_PUNCT_LSHIFT_ASSIGN)\
    PUNCTUATION("...",  MM_META_PUNCT_PARAMS)\
    PUNCTUATION("&&",   MM_META_PUNCT_LOGIC_AND)\
    PUNCTUATION("||",   MM_META_PUNCT_LOGIC_OR)\
    PUNCTUATION(">=",   MM_META_PUNCT_LOGIC_GEQ)\
    PUNCTUATION("<=",   MM_META_PUNCT_LOGIC_LEQ)\
    PUNCTUATION("==",   MM_META_PUNCT_LOGIC_EQ)\
    PUNCTUATION("!=",   MM_META_PUNCT_LOGIC_UNEQ)\
    PUNCTUATION("*=",   MM_META_PUNCT_MUL_ASSIGN)\
    PUNCTUATION("/=",   MM_META_PUNCT_DIV_ASSIGN)\
    PUNCTUATION("%=",   MM_META_PUNCT_MOD_ASSIGN)\
    PUNCTUATION("+=",   MM_META_PUNCT_ADD_ASSIGN)\
    PUNCTUATION("-=",   MM_META_PUNCT_SUB_ASSIGN)\
    PUNCTUATION("++",   MM_META_PUNCT_INC)\
    PUNCTUATION("--",   MM_META_PUNCT_DEC)\
    PUNCTUATION("&=",   MM_META_PUNCT_BIN_AND_ASSIGN)\
    PUNCTUATION("|=",   MM_META_PUNCT_BIN_OR_ASSIGN)\
    PUNCTUATION("^=",   MM_META_PUNCT_BIN_XOR_ASSIGN)\
    PUNCTUATION(">>",   MM_META_PUNCT_RSHIFT)\
    PUNCTUATION("<<",   MM_META_PUNCT_LSHIFT)\
    PUNCTUATION("->",   MM_META_PUNCT_POINTER)\
    PUNCTUATION("::",   MM_META_PUNCT_CPP1)\
    PUNCTUATION(".*",   MM_META_PUNCT_CPP2)\
    PUNCTUATION("*",    MM_META_PUNCT_MUL)\
    PUNCTUATION("/",    MM_META_PUNCT_DIV)\
    PUNCTUATION("%",    MM_META_PUNCT_MOD)\
    PUNCTUATION("+",    MM_META_PUNCT_ADD)\
    PUNCTUATION("-",    MM_META_PUNCT_SUB)\
    PUNCTUATION("=",    MM_META_PUNCT_ASSIGN)\
    PUNCTUATION("&",    MM_META_PUNCT_BIN_AND)\
    PUNCTUATION("|",    MM_META_PUNCT_BIN_OR)\
    PUNCTUATION("^",    MM_META_PUNCT_BIN_XOR)\
    PUNCTUATION("~",    MM_META_PUNCT_BIN_NOT)\
    PUNCTUATION("!",    MM_META_PUNCT_LOGIC_NOT)\
    PUNCTUATION(">",    MM_META_PUNCT_LOGIC_GREATER)\
    PUNCTUATION("<",    MM_META_PUNCT_LOGIC_LESS)\
    PUNCTUATION(".",    MM_META_PUNCT_REF)\
    PUNCTUATION(",",    MM_META_PUNCT_COMMA)\
    PUNCTUATION(";",    MM_META_PUNCT_SEMICOLON)\
    PUNCTUATION(":",    MM_META_PUNCT_COLON)\
    PUNCTUATION("?",    MM_META_PUNCT_QUESTIONMARK)\
    PUNCTUATION("(",    MM_META_PUNCT_PARENTHESE_OPEN)\
    PUNCTUATION(")",    MM_META_PUNCT_PARENTHESE_CLOSE)\
    PUNCTUATION("{",    MM_META_PUNCT_BRACE_OPEN)\
    PUNCTUATION("}",    MM_META_PUNCT_BRACE_CLOSE)\
    PUNCTUATION("[",    MM_META_PUNCT_BRACKET_OPEN)\
    PUNCTUATION("]",    MM_META_PUNCT_BRACKET_CLOSE)\
    PUNCTUATION("\\",   MM_META_PUNCT_BACKSLASH)\
    PUNCTUATION("#",    MM_META_PUNCT_PRECOMPILER)\
    PUNCTUATION("$",    MM_META_PUNCT_DOLLAR)

enum mm_meta_default_punctuation_ids {
#define MM_META_PUNCTUATION(chars, id) id,
    MM_META_DEFAULT_PUNCTION_MAP(MM_META_PUNCTUATION)
#undef MM_META_PUNCTUATION
    MM_META_PUNCT_MAX
};

struct mm_meta_punctuation {
    const char *string;
    int id;
};

enum mm_meta_token_type {
    MM_META_TOKEN_STRING,
    MM_META_TOKEN_LITERAL,
    MM_META_TOKEN_NUMBER,
    MM_META_TOKEN_NAME,
    MM_META_TOKEN_PUNCTUATION,
    MM_META_TOKEN_EOS
};

/* token subtype flags */
enum mm_meta_token_flags {
    MM_META_TOKEN_INT           = 0x00001,
    MM_META_TOKEN_DEC           = 0x00002,
    MM_META_TOKEN_HEX           = 0x00004,
    MM_META_TOKEN_OCT           = 0x00008,
    MM_META_TOKEN_BIN           = 0x00010,
    MM_META_TOKEN_LONG          = 0x00020,
    MM_META_TOKEN_UNSIGNED      = 0x00040,
    MM_META_TOKEN_FLOAT         = 0x00080,
    MM_META_TOKEN_SINGLE_PREC   = 0x00100,
    MM_META_TOKEN_DOUBLE_PREC   = 0x00200,
    MM_META_TOKEN_INFINITE      = 0x00400,
    MM_META_TOKEN_INDEFINITE    = 0x00800,
    MM_META_TOKEN_NAN           = 0x01000,
    MM_META_TOKEN_VALIDVAL      = 0x02000
};

struct mm_meta_token {
    enum mm_meta_token_type type;
    unsigned int subtype;
    int line;
    int line_crossed;
    struct {unsigned long i; double f;} value;
    const char *str;
    int len;
};

struct mm_meta_lexer {
    int error;
    const char *buffer;
    const char *current;
    const char *last;
    const char *end;
    int length;
    int line;
    int last_line;
    const struct mm_meta_punctuation *puncts;
    mm_meta_log_f logging;
    void *userdata;
};

/* library intern default punctuation map */
MM_META_GLOBAL const struct mm_meta_punctuation
mm_meta_default_punctuations[] = {
#define PUNCTUATION(chars, id) {chars, id},
    MM_META_DEFAULT_PUNCTION_MAP(PUNCTUATION)
#undef PUNCTUATION
    {0, 0}
};

/* ---------------------------------------------------------------
 *                          TOKEN
 * ---------------------------------------------------------------*/
MM_META_INTERN void
mm_meta_token_clear(struct mm_meta_token *tok)
{
    tok->line_crossed = 0;
}

MM_META_INTERN int
mm_meta_token_cpy(char *dst, int max, const struct mm_meta_token* tok)
{
    int i = 0;
    int ret;
    int siz;

    MM_META_ASSERT(dst);
    MM_META_ASSERT(tok);
    if (!dst || !max || !tok)
        return 0;

    ret = (max < (tok->len + 1)) ? max : tok->len;
    siz = (max < (tok->len + 1)) ? max-1 : tok->len;
    for (i = 0; i < siz; i++)
        dst[i] = tok->str[i];
    dst[siz] = '\0';
    return ret;
}

MM_META_INTERN int
mm_meta_token_icmp(const struct mm_meta_token* tok, const char* str)
{
    int i;
    MM_META_ASSERT(tok);
    MM_META_ASSERT(str);
    if (!tok || !str) return 1;
    for (i = 0; (i < tok->len && *str); i++, str++){
        if (mm_meta_char_upper(tok->str[i]) != mm_meta_char_upper(*str))
            return 1;
    }
    if (*str != 0 || i < tok->len)
        return 1;
    return 0;
}

MM_META_INTERN int
mm_meta_token_cmp(const struct mm_meta_token* tok, const char* str)
{
    int i;
    MM_META_ASSERT(tok);
    MM_META_ASSERT(str);
    if (!tok || !str) return 1;
    for (i = 0; (i < tok->len && *str); i++, str++){
        if (tok->str[i] != *str)
            return 1;
    }
    if (*str != 0 || i < tok->len)
        return 1;
    return 0;
}

MM_META_INTERN double
mm_meta_token_parse_double(const char *p, int length)
{
    int i, div, pow;
    double m;
    int len = 0;
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

MM_META_INTERN unsigned long
mm_meta_token_parse_int(const char *p, int length)
{
    unsigned long i = 0;
    int len = 0;
    while (len < length) {
        i = i * 10 + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

MM_META_INTERN unsigned long
mm_meta_token_parse_oct(const char *p, int length)
{
    unsigned long i = 0;
    int len = 1;
    while (len < length) {
        i = (i << 3) + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

MM_META_INTERN unsigned long
mm_meta_token_parse_hex(const char *p, int length)
{
    unsigned long i = 0;
    int len = 2;
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

MM_META_INTERN unsigned long
mm_meta_token_parse_bin(const char *p, int length)
{
    unsigned long i = 0;
    int len = 2;
    while (len < length) {
        i = (i << 1) + (unsigned long)(p[len] - '0');
        len++;
    }
    return i;
}

MM_META_INTERN void
mm_meta_token_number_value(struct mm_meta_token *tok)
{
    int i, pow, c;
    const char *p;
    MM_META_ASSERT(tok->type == MM_META_TOKEN_NUMBER);
    tok->value.i = 0;
    tok->value.f = 0;

    p = tok->str;
    if (tok->subtype & MM_META_TOKEN_FLOAT) {
        if (tok->subtype & ((MM_META_TOKEN_INFINITE|MM_META_TOKEN_INDEFINITE|MM_META_TOKEN_NAN))) {
            /* special real number constants */
            union {float f; unsigned int u;} convert;
            if (tok->subtype & MM_META_TOKEN_INFINITE) {
                convert.u = 0x7f800000;
                tok->value.f = (double)convert.f;
            } else if (tok->subtype & MM_META_TOKEN_INDEFINITE) {
                convert.u = 0xffc00000;
                tok->value.f = (double)convert.f;
            } else if (tok->subtype & MM_META_TOKEN_NAN) {
                convert.u = 0x7fc00000;
                tok->value.f = (double)convert.f;
            }
        } else tok->value.f = mm_meta_token_parse_double(tok->str, tok->len);
        tok->value.i = (unsigned long)tok->value.f;
    } else if (tok->subtype & MM_META_TOKEN_DEC) {
        /* paser decimal number */
        tok->value.i = mm_meta_token_parse_int(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & MM_META_TOKEN_OCT) {
        /* parse octal number */
        tok->value.i = mm_meta_token_parse_oct(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & MM_META_TOKEN_HEX) {
        /* parse hex number */
        tok->value.i = mm_meta_token_parse_hex(p, tok->len);
        tok->value.f = (double)tok->value.i;
    } else if (tok->subtype & MM_META_TOKEN_BIN) {
        /* parse binary number */
        tok->value.i = mm_meta_token_parse_bin(p, tok->len);
        tok->value.f = (double)tok->value.i;
    }
    tok->subtype |= MM_META_TOKEN_VALIDVAL;
}

MM_META_INTERN double
mm_meta_token_to_double(struct mm_meta_token *tok)
{
    if (tok->type != MM_META_TOKEN_NUMBER)
        return 0.0;
    if (!(tok->subtype & MM_META_TOKEN_VALIDVAL))
        mm_meta_token_number_value(tok);
    if (!(tok->subtype & MM_META_TOKEN_VALIDVAL))
        return 0.0;
    return tok->value.f;
}

MM_META_INTERN unsigned long
mm_meta_token_to_unsigned_long(struct mm_meta_token *tok)
{
    if (tok->type != MM_META_TOKEN_NUMBER)
        return 0;
    if (!(tok->subtype & MM_META_TOKEN_VALIDVAL))
        mm_meta_token_number_value(tok);
    if (!(tok->subtype & MM_META_TOKEN_VALIDVAL))
        return 0;
    return tok->value.i;
}

MM_META_INTERN int
mm_meta_token_to_int(struct mm_meta_token *tok)
{
    return (int)mm_meta_token_to_unsigned_long(tok);
}

MM_META_INTERN float
mm_meta_token_to_float(struct mm_meta_token *tok)
{
    double d = mm_meta_token_to_double(tok);
    float f = (float)d;;
    return f;
}

/* ---------------------------------------------------------------
 *
 *                          LEXER
 *
 * ---------------------------------------------------------------*/
MM_META_INTERN void
mm_meta_lexer_init(struct mm_meta_lexer *lexer, const char *ptr, int len,
    const struct mm_meta_punctuation *punct, mm_meta_log_f log, void *userdata)
{
    mm_meta_zero_struct(*lexer);
    lexer->buffer = ptr;
    lexer->current = ptr;
    lexer->last = ptr;
    lexer->end = ptr + len;
    lexer->length = len;
    lexer->line = lexer->last_line = 1;
    if (!punct)
        lexer->puncts = mm_meta_default_punctuations;
    else lexer->puncts = punct;
    lexer->logging = log;
    lexer->userdata = userdata;
}

MM_META_INTERN void
mm_meta_lexer_reset(struct mm_meta_lexer *lexer)
{
    lexer->current = lexer->buffer;
    lexer->last = lexer->buffer;
    lexer->line = lexer->last_line = 1;
}

MM_META_INTERN int
mm_meta_lexer_read_white_space(struct mm_meta_lexer *lexer, int current_line)
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
                        if (*(lexer->current+1) == '*' && lexer->logging) {
                            lexer->logging(lexer->userdata, MM_META_WARNING, lexer->line,
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

MM_META_INTERN int
mm_meta_lexer_read_esc_chars(struct mm_meta_lexer *lexer, char *ch)
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
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_WARNING, lexer->line,
                    "to large value in esc char: %d", val);
            }
            val = 0xFF;
        }
        c = val;
        break;
    } break;
    default: {
        if (*lexer->current < '0' || *lexer->current > '9') {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
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
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_WARNING, lexer->line,
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

MM_META_INTERN int
mm_meta_lexer_read_string(struct mm_meta_lexer *lexer,
    struct mm_meta_token *token, int quote)
{
    int tmpline;
    const char *tmp;
    char ch;
    if (quote == '\"')
        token->type = MM_META_TOKEN_STRING;
    else token->type = MM_META_TOKEN_LITERAL;
    lexer->current++;
    if (lexer->current >= lexer->end)
        return 0;

    token->len = 0;
    token->str = lexer->current;
    while (1) {
        if (*lexer->current == '\\') {
            if (!mm_meta_lexer_read_esc_chars(lexer, &ch))
                return 0;
        } else if (*lexer->current == quote) {
            lexer->current++;
            if (lexer->current >= lexer->end)
                return 0;

            tmp = lexer->current;
            tmpline = lexer->line;
            if (!mm_meta_lexer_read_white_space(lexer, 0)) {
                lexer->current = tmp;
                lexer->line = tmpline;
                break;
            }
            if (*lexer->current == '\0') {
                if (lexer->logging) {
                        lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                            "expecting string after '\' terminated line");
                }
                lexer->error = 1;
                return 0;
            }
            break;
        } else {
            if (*lexer->current == '\0') {
                if (lexer->logging) {
                    lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                        "missing trailing quote");
                }
                lexer->error = 1;
                return 0;
            }
            if (*lexer->current == '\n') {
                if (lexer->logging) {
                    lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                        "newline inside string");
                }
                lexer->error = 1;
                return 0;
            }
            lexer->current++;
        }
    }
    if (token->str) {
        token->len = (int)(lexer->current - token->str) - 1;
        if (token->type == MM_META_TOKEN_LITERAL)
            token->subtype = (unsigned int)token->str[0];
        else token->subtype = (unsigned int)token->len;
    }
    return 1;
}

MM_META_INTERN int
mm_meta_lexer_read_name(struct mm_meta_lexer *lexer, struct mm_meta_token *token)
{
    char c;
    token->type = MM_META_TOKEN_NAME;
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

MM_META_INTERN int
mm_meta_lexer_check_str(const struct mm_meta_lexer *lexer, const char *str, int len)
{
    int i;
    for (i = 0; i < len && (&lexer->current[i] < lexer->end); ++i) {
        if (lexer->current[i] != str[i])
            return 0;
    }
    if (i < len) return 0;
    return 1;
}

MM_META_INTERN int
mm_meta_lexer_read_number(struct mm_meta_lexer *lexer, struct mm_meta_token *token)
{
    int i;
    int dot;
    char c, c2;

    token->type = MM_META_TOKEN_NUMBER;
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
            token->subtype = MM_META_TOKEN_HEX | MM_META_TOKEN_INT;
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
            token->subtype = MM_META_TOKEN_BIN | MM_META_TOKEN_INT;
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
            token->subtype = MM_META_TOKEN_OCT | MM_META_TOKEN_INT;
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
            token->subtype = MM_META_TOKEN_DEC | MM_META_TOKEN_FLOAT;
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
                if (mm_meta_lexer_check_str(lexer, "INF", 3))
                    token->subtype  |= MM_META_TOKEN_INFINITE;
                else if (mm_meta_lexer_check_str(lexer, "IND", 3))
                    token->subtype  |= MM_META_TOKEN_INDEFINITE;
                else if (mm_meta_lexer_check_str(lexer, "NAN", 3))
                    token->subtype  |= MM_META_TOKEN_NAN;
                else if (mm_meta_lexer_check_str(lexer, "QNAN", 4)) {
                    token->subtype  |= MM_META_TOKEN_NAN; c2++;
                } else if (mm_meta_lexer_check_str(lexer, "SNAN", 4)) {
                    token->subtype  |= MM_META_TOKEN_NAN; c2++;
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
            token->subtype = MM_META_TOKEN_DEC | MM_META_TOKEN_INT;
        }
    }

    if (token->subtype & MM_META_TOKEN_FLOAT) {
        /* float point precision subtype */
        if (c > ' ') {
            if (c == 'f' || c == 'F') {
                token->subtype |= MM_META_TOKEN_SINGLE_PREC;
                lexer->current++;
            } else token->subtype |= MM_META_TOKEN_DOUBLE_PREC;
        } else token->subtype |= MM_META_TOKEN_DOUBLE_PREC;
    } else if (token->subtype & MM_META_TOKEN_INT) {
        /* integer subtype */
        if (c > ' '){
            for (i = 0; i < 2; ++i) {
                if (c == 'l' || c == 'L')
                    token->subtype |= MM_META_TOKEN_LONG;
                else if (c == 'u' || c == 'U')
                    token->subtype |= MM_META_TOKEN_UNSIGNED;
                else break;
                if (lexer->current+1 >= lexer->end) break;
                c = *(++lexer->current);
            }
        }
    }
    return 1;
}

MM_META_INTERN int
mm_meta_lexer_read_punctuation(struct mm_meta_lexer *lexer, struct mm_meta_token *token)
{
    const struct mm_meta_punctuation *punc;
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
            token->len += (int)l;
            lexer->current += l;
            token->type = MM_META_TOKEN_PUNCTUATION;
            token->subtype = (unsigned int)punc->id;
            return 1;
        }
    }
    return 0;
}

MM_META_INTERN int
mm_meta_lexer_read(struct mm_meta_lexer *lexer, struct mm_meta_token *token)
{
    int c;
    if (!lexer->current || lexer->current >= lexer->end || lexer->error)
        token->type = MM_META_TOKEN_EOS;

    mm_meta_zero_struct(*token);
    lexer->last = lexer->current;
    lexer->last_line = lexer->line;
    lexer->error = 0;
    if (!mm_meta_lexer_read_white_space(lexer, 0))
        return 0;

    token->line = lexer->line;
    token->line_crossed = (lexer->line - lexer->last_line) ? 1 : 0;

    c = *lexer->current;
    if ((c >= '0' && c <= '9') ||
        (c == '.' && (*(lexer->current + 1)) >= '0' &&
        (c == '.' && (*(lexer->current + 1)) <= '9'))) {
        if (!mm_meta_lexer_read_number(lexer, token))
            return 0;
    } else if (c == '\"' || c == '\'') {
        if (!mm_meta_lexer_read_string(lexer, token, c))
            return 0;
    } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        if (!mm_meta_lexer_read_name(lexer, token))
            return 0;
    } else if ((c == '/' || c == '\\') || c == '.') {
        if (!mm_meta_lexer_read_name(lexer, token))
            return 0;
    } else if (!mm_meta_lexer_read_punctuation(lexer, token)) {
        if (lexer->logging && lexer->current != lexer->end) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "unkown punctuation: %c", c);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

MM_META_INTERN int
mm_meta_lexer_read_on_line(struct mm_meta_lexer *lexer, struct mm_meta_token *token)
{
    struct mm_meta_token tok;
    if (!mm_meta_lexer_read(lexer, &tok)) {
        lexer->current = lexer->last;
        lexer->line = lexer->last_line;
    }
    if (!tok.line_crossed) {
        *token = tok;
        return 1;
    }
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    mm_meta_token_clear(token);
    return 0;
}

MM_META_INTERN int
mm_meta_lexer_expect_string(struct mm_meta_lexer *lexer, const char *string)
{
    struct mm_meta_token token;
    if (!mm_meta_lexer_read(lexer, &token)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to read expected token: %s", string);
        }
        lexer->error = 1;
        return 0;
    }
    if (mm_meta_token_cmp(&token, string)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "read token is not expected string: %s", string);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

MM_META_INTERN int
mm_meta_lexer_expect_type(struct mm_meta_lexer *lexer, enum mm_meta_token_type type,
    unsigned int subtype, struct mm_meta_token *token)
{
    if (!mm_meta_lexer_read(lexer, token)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "could not read expected token with type: %d", type);
        }
        lexer->error = 1;
        return 0;
    }
    if (token->type != type) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "read token has type %s instead of expected type: %d", token->type, type);
        }
        lexer->error = 1;
        return 0;
    }
    if ((token->subtype & subtype) != subtype) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "read token has subtype %d instead of expected subtype %d",
                token->subtype, type);
        }
        lexer->error = 1;
        return 0;
    }
    return 1;
}

MM_META_INTERN int
mm_meta_expect_any(struct mm_meta_lexer *lexer, struct mm_meta_token *token)
{
    if (!mm_meta_lexer_read(lexer, token)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "could not read any expected token");
        }
        return 0;
    }
    return 1;
}

MM_META_INTERN int
mm_meta_lexer_check_string(struct mm_meta_lexer *lexer, const char *string)
{
    struct mm_meta_token token;
    if (!mm_meta_lexer_read(lexer, &token))
        return 0;
    if (!mm_meta_token_cmp(&token, string))
        return 1;

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    return 0;
}

MM_META_INTERN int
mm_meta_lexer_check_type(struct mm_meta_lexer *lexer, enum mm_meta_token_type type,
    unsigned int subtype, struct mm_meta_token *token)
{
    struct mm_meta_token tok;
    if (!mm_meta_lexer_read(lexer, &tok))
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

MM_META_INTERN int
mm_meta_lexer_peek_string(struct mm_meta_lexer *lexer, const char *string)
{
    struct mm_meta_token tok;
    if (!mm_meta_lexer_read(lexer, &tok))
        return 0;

    /* unread token */
    lexer->current = lexer->last;
    lexer->line = lexer->last_line;
    if (!mm_meta_token_cmp(&tok, string))
        return 1;
    return 0;
}

MM_META_INTERN int
mm_meta_lexer_peek_type(struct mm_meta_lexer *lexer, enum mm_meta_token_type type,
    unsigned int subtype, struct mm_meta_token *token)
{
    struct mm_meta_token tok;
    if (!mm_meta_lexer_read(lexer, &tok))
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

MM_META_INTERN int
mm_meta_lexer_read_until(struct mm_meta_lexer *lexer, const char *string, struct mm_meta_token *token)
{
    int begin = 1;
    struct mm_meta_token tok;
    token->str = lexer->current;
    while (mm_meta_lexer_read(lexer, &tok)) {
        if (!mm_meta_token_cmp(&tok, string)) {
            token->len = (int)(tok.str - token->str);
            return 1;
        }
    }
    return 0;
}

MM_META_INTERN int
mm_meta_lexer_skip_until(struct mm_meta_lexer *lexer, const char *string)
{
    struct mm_meta_token tok;
    while (mm_meta_lexer_read(lexer, &tok)) {
        if (!mm_meta_token_cmp(&tok, string))
            return 1;
    }
    return 0;
}

MM_META_INTERN int
mm_meta_lexer_skip_line(struct mm_meta_lexer *lexer)
{
    struct mm_meta_token tok;
    while (mm_meta_lexer_read(lexer, &tok)) {
        if (tok.line_crossed) {
            lexer->current = lexer->last;
            lexer->line = lexer->last_line;
            return 1;
        }
    }
    return 0;
}

MM_META_INTERN int
mm_meta_lexer_parse_int(struct mm_meta_lexer *lexer)
{
    struct mm_meta_token tok;
    if (!mm_meta_lexer_read(lexer, &tok))
        return 0;
    if (tok.type == MM_META_TOKEN_PUNCTUATION && tok.str[0] == '-') {
        mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NUMBER, MM_META_TOKEN_INT, &tok);
        return -mm_meta_token_to_int(&tok);
    } else if (tok.type != MM_META_TOKEN_NUMBER || tok.subtype == MM_META_TOKEN_FLOAT) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "expected int value but found float");
        }
        lexer->error = 1;
    }
    return mm_meta_token_to_int(&tok);
}

MM_META_INTERN int
mm_meta_lexer_parse_bool(struct mm_meta_lexer *lexer)
{
    struct mm_meta_token tok;
    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NUMBER, 0, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "could not read expected boolean");
        }
        lexer->error = 1;
        return 0;
    }
    return (mm_meta_token_to_int(&tok) != 0);
}

MM_META_INTERN float
mm_meta_lexer_parse_float(struct mm_meta_lexer *lexer)
{
    struct mm_meta_token tok;
    if (!mm_meta_lexer_read(lexer, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "could not read expected float number");
        }
        lexer->error = 1;
        return 0;
    }
    if (tok.type == MM_META_TOKEN_PUNCTUATION && tok.str[0] == '-') {
        mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NUMBER, 0, &tok);
        return -(float)tok.value.f;
    } else if (tok.type != MM_META_TOKEN_NUMBER) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "expected float number is not a number");
        }
        lexer->error = 1;
        return 0;
    }
    return mm_meta_token_to_float(&tok);
}

/* ---------------------------------------------------------------
 *
 *                          PARSER
 *
 * ---------------------------------------------------------------*/
MM_META_INTERN char*
mm_meta_token_dup(struct mm_meta_info *info, struct mm_meta_token *tok)
{
    char *buffer;
    struct mm_meta_buffer *buf = info->memory;
    if (!tok->len) return 0;
    if (!info->memory || info->memory->size + tok->len >= info->memory->capacity) {
        buf = MM_META_MALLOC(sizeof(*buf));
        buf->capacity = MM_META_BUFFER_SIZE;
        buf->size = 0;
        buf->next = info->memory;
        info->memory = buf;
    }
    buffer = &buf->memory[buf->size];
    MM_META_MEMCPY(buffer, tok->str, tok->len);
    buffer[tok->len] = '\0';
    buf->size+= tok->len+1;
    return buffer;
}

MM_META_INTERN int
mm_meta_parse_add_type(struct mm_meta_info *meta, struct mm_meta_token *tok)
{
    int i, cnt = mm_ar_count(meta->types);
    for (i = 0; i < cnt; ++i) {
        if (!mm_meta_token_cmp(tok, meta->types[i].name))
            break;
    }
    if (i == cnt) {
        struct mm_meta_type type;
        mm_meta_zero_struct(type);
        type.index = i;
        type.name = mm_meta_token_dup(meta, tok);
        mm_ar_push(meta->types, type);
    }
    return i;
}

MM_META_INTERN int
mm_meta_parse_member(struct mm_meta_info *meta, struct mm_meta_struct *meta_struct,
    struct mm_meta_lexer *lexer, int *concat)
{
    struct mm_meta_member member;
    struct mm_meta_token tok;
    mm_meta_zero_struct(member);

    /* parse type */
    if (!*concat) {
        /* first member with current type */
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse member variable type of %s", meta_struct->name);
            }
            return 0;
        }

        if (!mm_meta_token_cmp(&tok, "struct") || !mm_meta_token_cmp(&tok, "enum")) {
            if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
                if (lexer->logging) {
                    lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                        "failed to parse member variable type of %s", meta_struct->name);
                }
                return 0;
            }
        }
        member.type = mm_meta_parse_add_type(meta, &tok);
    } else member.type = meta_struct->members[mm_ar_count(meta_struct->members)-1].type;

    /* check and parse pointer */
    if (mm_meta_lexer_check_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_MUL, &tok))
        member.flags |= META_FLAG_POINTER;

    /* parse name */
    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse member variable name of %s", meta_struct->name);
        }
        return 0;
    }
    member.name = mm_meta_token_dup(meta, &tok);

    /* check and parse array */
    if (mm_meta_lexer_check_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_BRACKET_OPEN, &tok)) {
        member.flags |= META_FLAG_ARRAY;
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NUMBER, 0, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse %s's member array variable %s size",
                    meta_struct->name, member.name);
            }
            return 0;
        }
        member.count = mm_meta_token_to_int(&tok);
        mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_BRACKET_CLOSE, &tok);
    } else member.count = 1;

    /* check and parse for additional variables in line */
    mm_meta_expect_any(lexer, &tok);
    if (tok.type == MM_META_TOKEN_PUNCTUATION && tok.subtype == MM_META_PUNCT_SEMICOLON)
        *concat = 0;
    else if (tok.type == MM_META_TOKEN_PUNCTUATION && tok.subtype == MM_META_PUNCT_COMMA)
        *concat = 1;
    mm_ar_push(meta_struct->members, member);
    return 1;
}

MM_META_INTERN int
mm_meta_parse_value(struct mm_meta_info *meta, struct mm_meta_enum *enums,
    struct mm_meta_lexer *lexer)
{
    struct mm_meta_value value;
    struct mm_meta_token tok;
    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok))
        return 0;

    value.id = 0;
    value.name = mm_meta_token_dup(meta, &tok);
    if (mm_meta_lexer_check_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_ASSIGN, &tok)) {
        /* normal int enumerator */
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NUMBER, 0, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse %s's member value after '='", enums->name);
            }
            return 0;
        }
        value.str_value = mm_meta_token_dup(meta, &tok);
        value.int_value = mm_meta_token_to_int(&tok);
        enums->index = value.int_value;
    } else if (mm_meta_lexer_check_string(lexer, "as")) {
        /* special string enumerator */
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse string enum %s: missing keyword 'as'", enums->name);
            }
            return 0;
        }
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_PARENTHESE_OPEN, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse string enum %s: missing '(' after as", enums->name);
            }
            return 0;
        }
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_STRING, 0, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse string enum %s: missing string enum value", enums->name);
            }
            return 0;
        }

        value.str_value = mm_meta_token_dup(meta, &tok);
        value.int_value = enums->index++;
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_PARENTHESE_CLOSE, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse string enum %s: missing ')' after string value: %s",
                    enums->name, value.str_value);
            }
            return 0;
        }
    } else {
        /* default index */
        char buffer[1024];
        value.int_value = enums->index++;
        sprintf(buffer, "%d", value.int_value);
        tok.str = buffer;
        tok.len = strlen(buffer);
        value.str_value = mm_meta_token_dup(meta, &tok);
    }
    mm_meta_lexer_check_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_COMMA, &tok);
    mm_ar_push(enums->values, value);
    return 1;
}

MM_META_INTERN int
mm_meta_parse_argument(struct mm_meta_info *meta, struct mm_meta_function *meta_fun,
    struct mm_meta_lexer *lexer)
{
    struct mm_meta_argument arg;
    struct mm_meta_token tok;
    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse argument %d in function %s: missing type",
                mm_ar_count(meta_fun->args)+1, meta_fun->name);
        }
        return 0;
    }

    arg.type = mm_meta_parse_add_type(meta, &tok);
    if (mm_meta_lexer_check_type(lexer, MM_META_TOKEN_NAME, 0, &tok))
        arg.name = mm_meta_token_dup(meta, &tok);
    else arg.name = 0;

    mm_meta_lexer_check_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_COMMA, &tok);
    mm_ar_push(meta_fun->args, arg);
    return 1;
}

MM_META_INTERN int
mm_meta_parse_introspectable(struct mm_meta_info *meta,
    struct mm_meta_lexer *lexer, const char *file)
{
    struct mm_meta_token tok;
    if (!mm_meta_expect_any(lexer, &tok))
        return 0;

    if (!mm_meta_token_cmp(&tok, "typedef")) {
        if (!mm_meta_expect_any(lexer, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse instrospectable: missing type after typedef");
            }
            return 0;
        }
    }

    if (!mm_meta_token_cmp(&tok, "struct")) {
        /* parse struct */
        int concat = 0;
        struct mm_meta_struct new_struct;
        mm_meta_zero_struct(new_struct);
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse instrospectable struct: missing name");
            }
            return 0;
        }
        new_struct.name = mm_meta_token_dup(meta, &tok);
        new_struct.type = mm_meta_parse_add_type(meta, &tok);

        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_BRACE_OPEN, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse instrospectable struct %s: missing '{'", new_struct.name);
            }
            return 0;
        }

        while (1) {
            int res = mm_meta_parse_member(meta, &new_struct, lexer, &concat);
            if (!res) return res;
            if (mm_meta_lexer_check_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_BRACE_CLOSE, &tok))
                break;
        }
        mm_ar_push(meta->structs, new_struct);
    } else if (!mm_meta_token_cmp(&tok, "enum")) {
        /* parse enum */
        struct mm_meta_enum new_enum;
        mm_meta_zero_struct(new_enum);
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse instrospectable enum: missing name");
            }
            return 0;
        }

        new_enum.name = mm_meta_token_dup(meta, &tok);
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_BRACE_OPEN, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse instrospectable enum %s: missing '{'", new_enum.name);
            }
        }
        while (1) {
            int res = mm_meta_parse_value(meta, &new_enum, lexer);
            if (!res) return res;
            if (mm_meta_lexer_check_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_BRACE_CLOSE, &tok))
                break;
        }
        mm_ar_push(meta->enums, new_enum);
    } else {
        /* parse function */
        struct mm_meta_function meta_fun;
        mm_meta_zero_struct(meta_fun);
        if (!mm_meta_token_cmp(&tok, "static")) {
            meta_fun.visibility = META_FUNCTION_STATIC;
            if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
                if (lexer->logging) {
                    lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                        "failed to parse instrospectable function: missing return type after 'static'");
                }
                return 0;
            }
        } else if (!mm_meta_token_cmp(&tok, "extern")) {
            meta_fun.visibility = META_FUNCTION_EXTERN;
            if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
                if (lexer->logging) {
                    lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                        "failed to parse instrospectable function: missing return type after 'extern'");
                }
                return 0;
            }
        }
        meta_fun.ret = mm_meta_parse_add_type(meta, &tok);
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse instrospectable function: missing name");
            }
            return 0;
        }

        meta_fun.name = mm_meta_token_dup(meta, &tok);
        meta_fun.file = file;
        meta_fun.line = (int)tok.line;
        if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_PARENTHESE_OPEN, &tok)) {
            if (lexer->logging) {
                lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                    "failed to parse instrospectable function %s: missing '('", meta_fun.name);
            }
            return 0;
        }

        while (1) {
            int res;
            if (mm_meta_lexer_check_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_PARENTHESE_CLOSE, &tok)) break;
            res = mm_meta_parse_argument(meta, &meta_fun, lexer);
            if (!res) return res;
        }
        mm_ar_push(meta->functions, meta_fun);
    }
    return 1;
}

MM_META_INTERN int
mm_meta_parse_slot(struct mm_meta_info *meta, struct mm_meta_table *db,
    struct mm_meta_lexer *lexer)
{
    int i = 0;
    struct mm_meta_token tok;
    struct mm_meta_slot slot;

    mm_meta_zero_struct(slot);
    if (!mm_meta_lexer_expect_string(lexer, "meta_slot")) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse table %s slot: %d: missing keyword 'meta_slot'",
                db->name, mm_ar_count(db->slots)+1);
        }
        return 0;
    }
    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_PARENTHESE_OPEN, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse table %s slot %d: missing '(' after 'meta_slot'",
                db->name, mm_ar_count(db->slots)+1);
        }
        return 0;
    }
    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse table %s slot %d: missing identifier after '('",
                db->name, mm_ar_count(db->slots)+1);
        }
        return 0;
    }

    slot.index = db->index++;
    slot.id = mm_meta_token_dup(meta, &tok);
    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_COMMA, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse table %s slot %s: missing ',' after identifier '%s'",
                db->name, slot.id, slot.id);
        }
        return 0;
    }
    if (!mm_meta_lexer_read_until(lexer, ")", &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse table %s slot %s: error while reading slot content",
                db->name, slot.id);
        }
        return 0;
    }

    slot.values = mm_meta_token_dup(meta, &tok);
    for (i = 0; i < tok.len; ++i) {
        if (slot.values[i] == ';')
            slot.values[i] = ',';
    }
    if (!mm_meta_lexer_check_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_COMMA, &tok))
        return 1;
    mm_ar_push(db->slots, slot);
    return 1;
}

MM_META_INTERN int
mm_meta_parse_table(struct mm_meta_info *meta, struct mm_meta_lexer *lexer)
{
    int i = 0;
    struct mm_meta_token tok;
    struct mm_meta_table db;

    mm_meta_zero_struct(db);
    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_PARENTHESE_OPEN, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse table: missing '(' after 'meta_table'");
        }
        return 0;
    }
    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse table: missing identifier after '('");
        }
        return 0;
    }
    db.storage = mm_meta_token_dup(meta, &tok);
    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_COMMA, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse table: missing ')' after identifier: '%s'", db.storage);
        }
        return 0;
    }
    if (!mm_meta_lexer_read_until(lexer, ")", &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse table: error while reading table definition");
        }
        return 0;
    }

    db.format = mm_meta_token_dup(meta, &tok);
    for (i = 0; i < tok.len; ++i) {
        if (tok.str[i] == ';')
            db.element_count++;
    }
    db.element_count++;
    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_NAME, 0, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse table: failed to read table name after ')'");
        }
        return 0;
    }
    db.name = mm_meta_token_dup(meta, &tok);

    if (!mm_meta_lexer_expect_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_BRACE_OPEN, &tok)) {
        if (lexer->logging) {
            lexer->logging(lexer->userdata, MM_META_ERROR, lexer->line,
                "failed to parse table %s: missing '{' after identifier: '%s'",
                db.name, db.name);
        }
        return 0;
    }

    while (1) {
        int res;
        if (mm_meta_lexer_check_type(lexer, MM_META_TOKEN_PUNCTUATION, MM_META_PUNCT_BRACE_CLOSE, &tok))
            break;
        res = mm_meta_parse_slot(meta, &db, lexer);
        if (!res) return res;
    }
    mm_ar_push(meta->tables, db);
    return 1;
}

MM_META_API int
mm_meta_load_from_memory(struct mm_meta_info *meta, const char *name,
    const char *buffer, int len)
{
    struct mm_meta_lexer lexer;
    mm_meta_lexer_init(&lexer, buffer, (int)len, 0, meta->log, meta->userdata);
    while (!lexer.error) {
        struct mm_meta_token tok;
        if (!mm_meta_lexer_read(&lexer, &tok))
            break;

        if (tok.type == MM_META_TOKEN_NAME) {
            if (!mm_meta_token_icmp(&tok, "meta_introspect")) {
                int res = mm_meta_parse_introspectable(meta, &lexer, name);
                if (!res) return res;
            } else if (!mm_meta_token_icmp(&tok, "meta_table")) {
                int res = mm_meta_parse_table(meta, &lexer);
                if (!res) return res;
            }
        } else if (tok.type == MM_META_TOKEN_EOS) break;;
    }
    return 1;
}

MM_META_API int
mm_meta_load(struct mm_meta_info *meta, const char *filename)
{
    char *buf;
    int len;
    FILE *fd = fopen(filename, "rb");
    if (!fd) return 0;

    fseek(fd, 0, SEEK_END);
    len = (int)ftell(fd);
    fseek(fd, 0, SEEK_SET);
    buf = (char*)MM_META_MALLOC((size_t)(len+1));
    buf[len] = '\0';
    fread(buf, (size_t)len, 1, fd);
    fclose(fd);
    return mm_meta_load_from_memory(meta, filename, buf, len);
}

/* ---------------------------------------------------------------
 *
 *                          GENERATOR
 *
 * ---------------------------------------------------------------*/
MM_META_INTERN void
mm_meta_generate_members(FILE *out, const struct mm_meta_struct *meta,
    const struct mm_meta_type *types)
{
    int i = 0;
    for (i = 0; i < mm_ar_count(meta->members); ++i) {
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
        fprintf(out, "(int)(&((struct %s*)0)->%s)", meta->name, meta->members[i].name);
        fprintf(out, "},\n");
    }
}

MM_META_INTERN void
mm_meta_generate_enum_values(FILE *out, const struct mm_meta_enum *meta)
{
    int i = 0;
    for (i = 0; i < mm_ar_count(meta->values); ++i) {
        fprintf(out, "    {%s, \"%s\", %d, \"%s\"},\n",
            meta->values[i].name, meta->values[i].name,
            meta->values[i].int_value, meta->values[i].str_value);
    }
}

static void
mm_meta_generate_function_args(FILE *out, const struct mm_meta_function *meta,
    const struct mm_meta_type *types)
{
    int i = 0;
    for (i = 0; i < mm_ar_count(meta->args); ++i)
        fprintf(out, "    {META_TYPE_%s, \"%s\"},\n",
            types[meta->args[i].type].name, meta->args[i].name);
}

static void
mm_meta_generate_table_slots(FILE *out, const struct mm_meta_table *meta)
{
    int i = 0;
    for (i = 0; i < mm_ar_count(meta->slots); ++i)
        fprintf(out, "    {%s, %s},\n", meta->slots[i].id, meta->slots[i].values);
}

MM_META_API void
mm_meta_generate_to(FILE *out, const struct mm_meta_info *meta)
{
    /* ----------------------- header --------------------------*/
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
    fprintf(out, "#endif\n\n");
    fprintf(out, "enum meta_type {\n");
    for (i = 0; i < mm_ar_count(meta->types); ++i)
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
    fprintf(out, "   int size;\n");
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
    for (i = 0; i < mm_ar_count(meta->tables); ++i) {
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

    /* generate definitions */
    for (i = 0; i < mm_ar_count(meta->structs); ++i)
        fprintf(out, "META_API const struct meta_member meta_members_of_%s[%d];\n",
            meta->structs[i].name, mm_ar_count(meta->structs[i].members)+1);
    for (i = 0; i < mm_ar_count(meta->enums); ++i)
        fprintf(out, "META_API const struct meta_enum_value meta_enum_values_of_%s[%d];\n",
            meta->enums[i].name, mm_ar_count(meta->enums[i].values)+1);
    for (i = 0; i < mm_ar_count(meta->functions); ++i)
        fprintf(out, "META_API const struct meta_argument meta_function_args_of_%s[%d];\n",
            meta->functions[i].name, mm_ar_count(meta->functions[i].args)+1);
    for (i = 0; i < mm_ar_count(meta->tables); ++i)
        fprintf(out, "META_API const struct %s meta_table_slots_of_%s[%d];\n",
            meta->tables[i].storage, meta->tables[i].name, mm_ar_count(meta->tables[i].slots));
    fprintf(out, "META_API const struct meta_struct meta_structs[%d];\n", mm_ar_count(meta->structs)+1);
    fprintf(out, "META_API const struct meta_enum meta_enums[%d];\n", mm_ar_count(meta->enums)+1);
    fprintf(out, "META_API const struct meta_function meta_functions[%d];\n", mm_ar_count(meta->functions)+1);
    fprintf(out, "META_API const struct meta_table meta_tables[%d];\n", mm_ar_count(meta->tables)+1);
    fprintf(out, "#endif\n\n");

    /* ----------------------- implementation --------------------------*/
    fprintf(out, "#ifdef META_IMPLEMENTATION\n");
    fprintf(out, "#include <string.h>\n\n");

    /* generate data */
    for (i = 0; i < mm_ar_count(meta->structs); ++i) {
        fprintf(out, "const struct meta_member meta_members_of_%s[] = {\n", meta->structs[i].name);
        mm_meta_generate_members(out, &meta->structs[i], meta->types);
        fprintf(out, "    {0,0,0,0,0}\n");
        fprintf(out, "};\n");
    }
    fprintf(out, "\n");

    for (i = 0; i < mm_ar_count(meta->enums); ++i) {
        fprintf(out, "const struct meta_enum_value meta_enum_values_of_%s[] = {\n", meta->enums[i].name);
        mm_meta_generate_enum_values(out, &meta->enums[i]);
        fprintf(out, "    {0,0,0,0}\n");
        fprintf(out, "};\n");
    }
    fprintf(out, "\n");

    for (i = 0; i < mm_ar_count(meta->functions); ++i) {
        fprintf(out, "const struct meta_argument meta_function_args_of_%s[] = {\n", meta->functions[i].name);
        mm_meta_generate_function_args(out, &meta->functions[i], meta->types);
        fprintf(out, "    {0,0}\n");
        fprintf(out, "};\n");
    }
    fprintf(out, "\n");

    for (i = 0; i < mm_ar_count(meta->tables); ++i) {
        fprintf(out, "const struct %s meta_table_slots_of_%s[] = {\n",
            meta->tables[i].storage, meta->tables[i].name);
        mm_meta_generate_table_slots(out, &meta->tables[i]);
        fprintf(out, "};\n");
    }
    fprintf(out, "\n");

    fprintf(out, "const struct meta_struct meta_structs[] = {\n");
    for (i = 0; i < mm_ar_count(meta->structs); ++i) {
        fprintf(out, "    {META_TYPE_%s, \"%s\", sizeof(struct %s), %d, &meta_members_of_%s[0]},\n",
            meta->types[meta->structs[i].type].name, meta->structs[i].name,
            meta->structs[i].name, mm_ar_count(meta->structs[i].members), meta->structs[i].name);
    }
    fprintf(out, "    {0,0,0,0,0}\n");
    fprintf(out, "};\n\n");

    fprintf(out, "const struct meta_enum meta_enums[] = {\n");
    for (i = 0; i < mm_ar_count(meta->enums); ++i) {
        fprintf(out, "    {\"%s\", %d, %d, &meta_enum_values_of_%s[0]},\n",
            meta->enums[i].name, meta->enums[i].index, mm_ar_count(meta->enums[i].values), meta->enums[i].name);
    }
    fprintf(out, "    {0,0,0,0}\n");
    fprintf(out, "};\n\n");

    fprintf(out, "const struct meta_function meta_functions[] = {\n");
    for (i = 0; i < mm_ar_count(meta->functions); ++i) {
        fprintf(out, "    {\"%s\", ", meta->functions[i].name);
        fprintf(out, "\"%s\", ", meta->functions[i].file);
        fprintf(out, "%d, ", meta->functions[i].line);
        fprintf(out, "META_FUNCTION_%s, ",
            (meta->functions[i].visibility == META_FUNCTION_STATIC)? "STATIC": "EXTERN");
        fprintf(out, "META_TYPE_%s, ", meta->types[meta->functions[i].ret].name);
        fprintf(out, "%s, ", meta->functions[i].name);
        fprintf(out, "%d, ", mm_ar_count(meta->functions[i].args));
        fprintf(out, "&meta_function_args_of_%s[0]},\n", meta->functions[i].name);
    }
    fprintf(out, "    {0,0,0,0,0,0,0}\n");
    fprintf(out, "};\n\n");

    fprintf(out, "const struct meta_table meta_tables[] = {\n");
    for (i = 0; i < mm_ar_count(meta->tables); ++i) {
        fprintf(out, "    {\"%s\", \"%s\", %d, &meta_table_slots_of_%s[0]},\n",
            meta->tables[i].name, meta->tables[i].storage,
            mm_ar_count(meta->tables[i].slots), meta->tables[i].name);
    }
    fprintf(out, "    {0,0,0}\n");
    fprintf(out, "};\n\n");

    /* generate functions */
    fprintf(out, "META_API const struct meta_struct*\n");
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
    fprintf(out, "META_API const struct meta_struct*\n");
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
    fprintf(out, "META_API const struct meta_member*\n");
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
    fprintf(out, "META_API const struct meta_member*\n");
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
    fprintf(out, "META_API void*\n");
    fprintf(out, "meta_member_ptr_from_name(void *obj, const char *type, const char *name)\n");
    fprintf(out, "{\n");
    fprintf(out, "    const struct meta_member *member = meta_member_from_name(type, name);\n");
    fprintf(out, "    if (!member) return 0;\n");
    fprintf(out, "    return (unsigned char*)obj + member->offset;\n");
    fprintf(out, "}\n");
    fprintf(out, "META_API void*\n");
    fprintf(out, "meta_member_ptr_from_id(void *obj, enum meta_type type, const char *id)\n");
    fprintf(out, "{\n");
    fprintf(out, "    const struct meta_member *member = meta_member_from_id(type, id);\n");
    fprintf(out, "    if (!member) return 0;\n");
    fprintf(out, "    return (unsigned char*)obj + member->offset;\n");
    fprintf(out, "}\n");
    fprintf(out, "META_API const struct meta_enum*\n");
    fprintf(out, "meta_enum_from_string(const char *enumerator)\n");
    fprintf(out, "{\n");
    fprintf(out, "    const struct meta_enum *iter = &meta_enums[0];\n");
    fprintf(out, "    while (iter->name) {\n");
    fprintf(out, "        if (!strcmp(iter->name, enumerator))\n");
    fprintf(out, "            return iter;\n");
    fprintf(out, "        iter++;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
    fprintf(out, "META_API int\n");
    fprintf(out, "meta_enum_value_from_string(const char *enums, const char *id)\n");
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
    fprintf(out, "#endif\n\n");
}

MM_META_API int
mm_meta_generate(const char *file, const struct mm_meta_info *meta)
{
    FILE *fd = fopen(file, "wb");
    if (!fd) return 0;
    mm_meta_generate_to(fd, meta);
    return 1;
}
/* ---------------------------------------------------------------
 *
 *                          META
 *
 * ---------------------------------------------------------------*/
MM_META_API void
mm_meta_init(struct mm_meta_info *meta, mm_meta_log_f log, void *userdata)
{
    mm_meta_zero_struct(*meta);
    meta->log = log;
    meta->userdata = userdata;
}

MM_META_API void
mm_meta_free(struct mm_meta_info *meta)
{
    int i;
    struct mm_meta_buffer *iter;
    if (!meta) return;

    for (i = 0; i < mm_ar_count(meta->tables); ++i)
        mm_ar_free(meta->tables[i].slots);
    for (i = 0; i < mm_ar_count(meta->functions); ++i)
        mm_ar_free(meta->functions[i].args);
    for (i = 0; i < mm_ar_count(meta->enums); ++i)
        mm_ar_free(meta->enums[i].values);
    for (i = 0; i < mm_ar_count(meta->structs); ++i)
        mm_ar_free(meta->structs[i].members);

    mm_ar_free(meta->types);
    mm_ar_free(meta->structs);
    mm_ar_free(meta->enums);
    mm_ar_free(meta->functions);
    mm_ar_free(meta->tables);

    iter = meta->memory;
    while (iter) {
        struct mm_meta_buffer *next = iter->next;
        MM_META_FREE(iter);
        iter = next;
    }
}
#endif
