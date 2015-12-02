/*
    mm_json.h - zlib - Micha Mettke

ABOUT:
    This is a single header JSON parser header and implementation without
    any dependencies (even the standard library),
    string memory allocation or complex tree generation.
    Instead this library focuses on parsing tokens from a previously
    loaded in memory JSON file. Each token thereby references the JSON file
    string and limits the allocation to the initial JSON file instead of
    allocating a new string for each token.

QUICK:
    To use this file do:
    #define MMJ_IMPLEMENTATION
    before you include this file in *one* C or C++ file to create the implementation

    If you want to keep the implementation in that file you have to do
    #define MMJ_STATIC before including this file

    If you want to use asserts to add validation add
    #define MMJ_ASSERT before including this file

    To overwrite the default seperator character used inside
    the query functions
    #define MMJ_DELIMITER (character) before including this file

CONCEPT:
    The library contains three different parts.
    Tokenizer
    The tokenizer divides the initial in memory JSON file string into
    smaller interpreterable tokens and does not require any allocations at all.
    This is achieved by using an iterator to walk over each object inside an
    object and parsing each subobject with a new iterator.
    You can use the tokenizer without the parser if you either only want to
    parse a subsection of a JSON object or use a different language like Lua.

    Parser
    The parser uses the tokenizer internally to read in a JSON file into a
    token array. Each token thereby contains information about the JSON
    object tree and you can access the tree by a number of utility functions.

    Utility
    The library has a number of utility functions to access or parse and convert
    a previously read token. In addition if you use the parser a number of functions
    are provided to look for a token inside the parsed array.

    It is important to note that while every valid JSON tree should be parsable
    without problems, there is no guarantee that any invalid input is detected as
    such, since the parser has only minimal validation.
    In addition if you want to use the library with Multithreading make sure
    to call `mmj_init` once before using.

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
    If you do not define MMJ_IMPLEMENTATION before including this file, it
    will operate in header only mode. In this mode it declares all used structs
    and the API of the library without including the implementation of the library.

    Implementation mode:
    If you define MMJ_IMPLEMENTATIOn before including this file, it will
    compile the implementation of the JSON parser. To specify the visibility
    as private and limit all symbols inside the implementation file
    you can define MMJ_STATIC before including this file.
    Make sure that you only include this file implementation in *one* C or C++ file
    to prevent collisions.

EXAMPLES:*/
#if 0
    /* Lexer example */
    const char *json = "{a:"test",b:5, c:[0,1,2,4,5]}";
    mmj_size len = strlen(json);

    /* create iterator  */
    struct mmj_iter iter;
    iter = mmj_begin(json, len);

    /* read token pair */
    struct mmj_pair pair;
    iter = mmj_parse(&pair, &iter);
    assert(!mmj_cmp(&pair.name, "a"));
    assert(!mmj_cmp(&pair.value, "test"));
    assert(pair.value.type == MMJ_STRING);

    /* convert token to number */
    mmj_number num;
    iter = mmj_parse(&pair, &iter);
    mmj_test_assert(mmj_convert(&num, &pair.value) == MMJ_NUMBER);
    assert(num == 5.0);

    /* read subobject (array/objects) */
    iter = mmj_parse(&pair, &iter);
    mmj_test_assert(pair.value.type == MMJ_ARRAY);

    struct mmj_iter array = mmj_begin(pair.value.str, pair.value.len);
    iter = mmj_read(&tok, &array);
    while (array.src) {
        /* read single token */
        array = mmj_read(&tok, &array);
    }
#endif
#if 0
    /* Parser example */
    const char *json = "{...}";
    mmj_size len = strlen(json);

    /* load content into token array */
    mmj_size read = 0;
    mmj_size num = mmj_num(json, len);
    struct mmj_token *toks = calloc(num, sizeof(struct mmj_token));
    mmj_load(toks, num, &read, json, len);

    /* query token */
    struct mmj_token * t0 = mmj_query(toks, num, "map.entity[4].position");
    struct mmj_token * t1 = mmj_query_del(toks, num, "map:entity[4]:position", ':');

    /* query string */
    char buffer[64];
    mmj_size size;
    mmj_query_string(buffer, 64, &size, toks, num, "map.entity[4].position");
    mmj_query_string_del(buffer, 64, &size, toks, num, "map%entity[9]%name", '%');

    /* query number */
    mmj_number num;
    mmj_query_number(&num, toks, num, "map.solider[2].position.x");
    mmj_query_number_del(&num, toks, num, "map/solider[2]/position.x", "/");

    /* query type */
    mmj_int typ0 = mmj_query_number(toks, num, "map.solider[2]);
    mmj_int typ1 = mmj_query_number_del(toks, num, "map_solider[2], '_');

    /* subqueries */
    json_token *entity = mmj_query(toks, num, "map.entity[4]");
    json_token *position = mmj_query(entity, entity->sub, "position");
    json_token *rotation = mmj_query(entity, entity->sub, "rotation");
#endif

 /* ===============================================================
 *
 *                          HEADER
 *
 * =============================================================== */
#ifndef MMJ_H_
#define MMJ_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MMJ_STATIC
#define MMJ_API static
#else
#define MMJ_API extern
#endif

typedef int mmj_int;
typedef char mmj_char;
typedef unsigned long mmj_size;
typedef double mmj_number;

/* default delimiter used in the `mmj_query` function to seperate elements path */
#ifndef MMJ_DELIMITER
#define MMJ_DELIMITER '.'
#endif

enum mmj_token_type {
    MMJ_NONE,      /* invalid token */
    MMJ_OBJECT,    /* subobject */
    MMJ_ARRAY,     /* subarray */
    MMJ_NUMBER,    /* float point number token */
    MMJ_STRING,    /* string text token */
    MMJ_TRUE,      /* true constant token */
    MMJ_FALSE,     /* false constant token*/
    MMJ_NULL,      /* null constant token */
    MMJ_MAX
};

struct mmj_token {
    /*A Token represents a section in a string containing a valid json DOM value.
    Since the token only references the json object string, no actual memory needs
    to be allocated from the library to hold each token. Therefore the library does not
    represent a classical datastructure representing the json object as a tree, instead
    it is only responsible for parsing the string. In addition it is to note that the string
    inside of the token is not terminated with '\0' since it would require write
    access to the json string which is not always wanted. */
    const mmj_char *str;
    /* pointer to the beginning of the token */
    mmj_size len;
    /* length of the token */
    mmj_size children;
    /* number of direct child tokens */
    mmj_size sub;
    /* total number of subtokens (note: not pairs). Used for querying inside
     * previsouly queried tokens */
    enum mmj_token_type type;
    /* token type */
};

struct mmj_pair {
    /* A Pair represents two tokens with name and value token. Every Object, Array and
    Element in a Object except values inside a Array are pairs. So it is possible
    to read in two tokens directly at once while still getting correct results. */
    struct mmj_token name;
    /* token representing the name of the pair */
    struct mmj_token value;
    /* token holding the value*/
};
/*--------------------------------------------------------------------------
                            UTILITY
  -------------------------------------------------------------------------*/
/*
    UTILITY
    The utiltiy API provided access to the string inside a previously read token or
    token array. Its main purpose of existence lies in the way tokens are
    handled in this library. Tokens do not allocate memory for every string,
    but instead only reference the source json DOM string. But since a number
    of operation are needed on the string the utility function were added.

    USAGE
    The utility function allow to copy the string of a token into a char buffer,
    converting number tokens into number and compare the string inside the
    token with another string.

    Utility function API (not neccessary for the actual parsing process)
    mmj_find   -- queries a token array with a path to a key and returns the value
    mmj_cpy    -- copies a prev read token into a user provided buffer and returns the len
    mmj_cmp    -- compares a prev read token with a user provided string
    mmj_convert-- converts a previously parsed token into a number
    mmj_init   -- initilizes the parser look up tables only needs to be called
                    before the first use and if the library is used with multithreading
*/
MMJ_API mmj_int mmj_cmp(const struct mmj_token*, const mmj_char*);
/*  this function compares a previously read token with a user provieded string
    Input:
    - previously parsed token that needs to be compared against
    - user provided string
    Output:
    - if both are equal 0 otherwise 1
*/
MMJ_API mmj_size mmj_cpy(mmj_char*, mmj_size, const struct mmj_token*);
/*  this function copies a previously read token into a user provided buffer
    Input:
    - max fill size of the buffer
    - previously parsed token that will be copied into the buffer
    Output:
    - buffer containing the token
*/
MMJ_API mmj_int mmj_convert(mmj_number *, const struct mmj_token*);
/*  this function converts a previously read token into a number
    Input:
    - previously parsed token that needs to be converted
    Output:
    - number that has been converted from the token
    - MMJ_NUMBER on success or mmj_NOME on failure
*/
MMJ_API void mmj_init(void);
/* this function initializes the internal parser lookup tables.
 * Only needed to be called at the beginning if used with multithreading */

/*--------------------------------------------------------------------------
                            TOKENIZER
  -------------------------------------------------------------------------*/
/*
    TOKENIZER
    -----------------------
    The tokenizer is based around an iterator which steps
    over the first depth of an object. Deeper levels can be reached with
    iterators by parsing the resulting tokens. The tokenizer does not
    allocate any memory for tokens instead the string inside the token only
    references the source string and has therefore a very low memory foot print.
    Internally the tokenizer uses a look up table to parse tokens. Which
    costs ~1.5kB of memory which is quite a lot but makes the parsing process
    quite easy and even readable.

    USAGE
    ------------------------
    To use the tokenizer you have to first create an iterator with
    `mmj_begin`. After that the iterator can be used to iterate over the first
    depth of the DOM-Tree. This is done by either using `mmj_parse` for pairs
    or `mmj_read` or single tokens which is useful for array values.

    Tokenzier API (based around an iterator)
    mmj_begin  -- creates a parsing lexer iterator from a json string and a string length
    mmj_read   -- parses a token and returns the iterator to the next token
    mmj_parse  -- parses a token pair and returns the iterator to the next token
*/
struct mmj_iter {
    /* The lexer iterator controlls the parsing procedure and is similar to the
    container iterator found in the C++ STL library. Important to note is that
    the iterator only works up to the first depth of a JSON DOM tree, but a
    new iterator can be created with the parsed output. */
    mmj_int err;
    /* flag indicating if a parsing error or EOF took place */
    mmj_int depth;
    /* current depth of the parsed DOM JSON tree iterator */
    const char *go;
    /* current jump table used in the parser */
    const mmj_char *src;
    /* current pointer to the parsed string */
    mmj_size len;
    /* curent length of the current parsed string */
};

MMJ_API struct mmj_iter mmj_begin(const mmj_char *json, mmj_size length);
/*  this function initializes an lexer iterator
    Input:
    - json is a string containing a json DOM object that needs to be parsed
    - length descripes the number of bytes that makeup the DOM object
    Output:
    - new lexer iterator ready to parse the specified string
*/
MMJ_API struct mmj_iter mmj_read(struct mmj_token*, const struct mmj_iter*);
/*  this function uses the lexer to read in one single token and returns a
    a iterator pointing to the next token.
    Input:
    - lexer iterator pointing to a valid json string object
    Output:
    - lexer iterator ready to parse the specified string
    - token that will be read from the lexer iterator
*/
MMJ_API struct mmj_iter mmj_parse(struct mmj_pair*, const struct mmj_iter*);
/*  this function uses the lexer to read in one token pair and returns a
    a iterator pointing to the next token.
    Input:
    - lexer iterator pointing to a valid json string object
    Output:
    - new lexer iterator ready to parse the specified string
    - pair that will be read from the lexer iterator
*/

/*-------------------------------------------------------------------------
                            PARSER
  -------------------------------------------------------------------------
    PARSER
    -----------------------
    The parser uses the tokenizer internally to fill an array with JSON tokens.
    Each token does not allocate memory, instead they directly reference the
    JSON-DOM source string. The parsed output of the parser can be used to fill
    a tree datastructure or to query against a path.

    USAGE
    ------------------------
    The parser has to function. The first function `mmj_num` calculates
    the needed number of tokens, while the `mmj_load` function reads in all tokens.

    Retain mode parsing API (based on preallocated token array)
    mmj_num    -- returns the total number of tokens inside a json DOM tree
    mmj_load   -- loads all tokens in a json DOM tree into tokens
*/
enum mmj_status {
    MMJ_OK = 0,
    /* The parsing process was successfull and no problem were found */
    MMJ_INVAL,
    /* A invalid value has been passed into the function */
    MMJ_OUT_OF_TOKEN,
    /* Not enough tokens have been passed so the parsing process had to be stopped*/
    MMJ_PARSING_ERROR
    /* A invalid token or wrong json string has been passed */
};

MMJ_API mmj_size mmj_num(const mmj_char *json, mmj_size length);
/*  this function calculates the number of tokens inside the JSON DOM object
    Input:
    - json is a string containing a json DOM object that needs to be parsed
    - length descripes the number of bytes that makeup the DOM object
    Output:
    - number of tokens inside the json string
*/
MMJ_API enum mmj_status mmj_load(struct mmj_token *toks, mmj_size max,
                                    mmj_size *read, const mmj_char *json,
                                    mmj_size length);
/*  this function reads all JSON DOM tree tokens into a token array
    Input:
    - toks is a token array to fill with tokens
    - max is the maximum number of tokens in the array
    - read is the final token count which needs to be initialized to zero
    - json is a string containing a json DOM object that needs to be parsed
    - length descripes the number of bytes that makeup the DOM object
    Output:
    - error status of the operation
*/

/*--------------------------------------------------------------------------
                            QUERY
  -------------------------------------------------------------------------
    QUERY
    -----------------------
    Each query function pair allows either to find and convert, buffer or check
    its type of a token in a previouly parsed token array. The query in itself
    is based around a path to the token. Each segement of the path is seperated
    by a delemiter. The delimiter is defined in macro MMJ_DELIMITER but
    each query function provides a additional function with delimiter argument.

    USAGE
    ------------------------
    To query for a token or token content you first have to load the content
    of the json string into a token array by using the parser API. After that
    you can call each query function on the token array.

    Query API (based on parser token array output)
    mmj_query          -- finds a token at a given path in the token array
    mmj_query_number   -- finds and converts a token at a given path to a number
    mmj_query_string   -- finds and buffers a string token at a given path
    mmj_query_type     -- returns the type of a token at a given path in the token array
*/
MMJ_API struct mmj_token *mmj_query(struct mmj_token *toks, mmj_size count,
                                        const mmj_char *path);
MMJ_API struct mmj_token *mmj_query_del(struct mmj_token *toks, mmj_size count,
                                            const mmj_char *path, mmj_char del);
/*  this function trys to find a certain node in the JSON-DOM tree by path
    Input:
    - toks is a token array that has to be previously parsed
    - count is the number of tokens inside the array
    - path is a sequence of node names to a destiniation inside the tree seperated
        by '.' eg.: map.entity[0].position.x (case sensitive) and [i] for arrays
    [-del is the delemiter between each key name in the path or if not set MMJ_DELIMITER]
    Output:
    - the requested token if it exist otherwise NULL
*/
MMJ_API mmj_int mmj_query_number(mmj_number*, struct mmj_token *toks,
                                    mmj_size count, const mmj_char *path);
MMJ_API mmj_int mmj_query_number_del(mmj_number*, struct mmj_token *toks,
                                        mmj_size count, const mmj_char *path,
                                        mmj_char del);
/*  this function trys to find and convert a certain token by path to number
    Input:
    - toks is a token array that has to be previously parsed
    - count is the number of tokens inside the array
    - path is a sequence of node names to a destiniation inside the tree seperated
        by '.' eg.: map.entity[0].position.x (case sensitive) and [i] for arrays
    [-del is the delemiter between each key name in the path or if not set MMJ_DELIMITER]
    Output:
    - number that has been converted from the token
    - MMJ_NUMBER on success, MMJ_NONE on not found, token type otherwise
*/
MMJ_API mmj_int mmj_query_string(mmj_char*, mmj_size max, mmj_size *size,
                                    struct mmj_token *toks, mmj_size count,
                                    const mmj_char *path);
MMJ_API mmj_int mmj_query_string_del(mmj_char*, mmj_size max, mmj_size *size,
                                        struct mmj_token *toks, mmj_size count,
                                        const mmj_char *path, mmj_char del);
/*  this function trys to find and buffer a certain string token by path
    Input:
    - maximum number of bytes inside the buffer
    - toks is a token array that has to be previously parsed
    - count is the number of tokens inside the array
    - path is a sequence of node names to a destiniation inside the tree seperated
        by '.' eg.: map.entity[0].position.x (case sensitive) and [i] for arrays
    [-del is the delemiter between each key name in the path or if not set MMJ_DELIMITER]
    Output:
    - buffer containing the token
    - size of the buffer in bytes
    - MMJ_STRING on success, MMJ_NONE on not found, token type otherwise
*/
MMJ_API mmj_int mmj_query_type(struct mmj_token *toks, mmj_size count,
                                    const mmj_char *path);
MMJ_API mmj_int mmj_query_type_del(struct mmj_token *toks, mmj_size count,
                                    const mmj_char *path, mmj_char del);
/*  this function trys to find a token and return its type
    Input:
    - toks is a token array that has to be previously parsed
    - count is the number of tokens inside the array
    - path is a sequence of node names to a destiniation inside the tree seperated
        by '.' eg.: map.entity[0].position.x (case sensitive) and [i] for arrays
    [-del is the delemiter between each key name in the path or if not set MMJ_DELIMITER]
    Output:
    - MMJ_NONE if not found, token type otherwise
*/
#ifdef __cplusplus
}
#endif
#endif /* MMJ_H_ */

/*===============================================================
 *
 *                          IMPLEMENTATION
 *
 * =============================================================== */
#ifdef MMJ_IMPLEMENTATION
/* this flag inserts the <assert.h> header into the json.c file and adds
 assert call to every function in DEBUG mode. If activated then
 the clib will be used. So if you want to compile without clib then
 deactivate this flag. */
#ifdef MMJ_USE_ASSERT
#ifndef MMJ_ASSERT
#include <assert.h>
#define MMJ_ASSERT(expr) assert(expr)
#endif
#else
#define MMJ_ASSERT(expr)
#endif

#define MMJ_INTERN static
#define MMJ_GLOBAL static

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

/* Main token parsing function states */
enum mmj_parser_states {
    MMJ_STATE_FAILED,
    MMJ_STATE_LOOP,
    MMJ_STATE_SEP,
    MMJ_STATE_UP,
    MMJ_STATE_DOWN,
    MMJ_STATE_QUP,
    MMJ_STATE_QDOWN,
    MMJ_STATE_ESC,
    MMJ_STATE_UNESC,
    MMJ_STATE_BARE,
    MMJ_STATE_UNBARE,
    MMJ_STATE_UTF8_2,
    MMJ_STATE_UTF8_3,
    MMJ_STATE_UTF8_4,
    MMJ_STATE_UTF8_NEXT,
    MMJ_STATE_MAX
};

/* Main token to number conversion states */
enum mmj_nuber_states {
    MMJ_STATE_NUM_FAILED,
    MMJ_STATE_NUM_LOOP,
    MMJ_STATE_NUM_FLT,
    MMJ_STATE_NUM_EXP,
    MMJ_STATE_NUM_BREAK,
    MMJ_STATE_NUM_MAX
};

/* global parser jump tables */
MMJ_GLOBAL char mmj_go_struct[256];
MMJ_GLOBAL char mmj_go_bare[256];
MMJ_GLOBAL char mmj_go_string[256];
MMJ_GLOBAL char mmj_go_utf8[256];
MMJ_GLOBAL char mmj_go_esc[256];
MMJ_GLOBAL char mmj_go_num[256];
MMJ_GLOBAL const struct mmj_iter MMJ_ITER_NULL = {0,0,0,0,0};
MMJ_GLOBAL const struct mmj_token MMJ_TOKEN_NULL = {0,0,0,0,MMJ_NONE};
MMJ_GLOBAL int mmj_is_initialized;

/*--------------------------------------------------------------------------
                                HELPER
  -------------------------------------------------------------------------*/
/* initializes the parser jump tables */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wchar-subscripts"
#endif

MMJ_API void
mmj_init(void)
{
    int i;
    if (mmj_is_initialized) return;
    mmj_is_initialized = 1;
    for (i = 48; i <= 57; ++i)
        mmj_go_struct[i] = MMJ_STATE_BARE;
    mmj_go_struct['\t'] = MMJ_STATE_LOOP;
    mmj_go_struct['\r'] = MMJ_STATE_LOOP;
    mmj_go_struct['\n'] = MMJ_STATE_LOOP;
    mmj_go_struct[' '] = MMJ_STATE_LOOP;
    mmj_go_struct['"'] = MMJ_STATE_QUP;
    mmj_go_struct[':'] = MMJ_STATE_SEP;
    mmj_go_struct['='] = MMJ_STATE_SEP;
    mmj_go_struct[','] = MMJ_STATE_LOOP;
    mmj_go_struct['['] = MMJ_STATE_UP;
    mmj_go_struct[']'] = MMJ_STATE_DOWN;
    mmj_go_struct['{'] = MMJ_STATE_UP;
    mmj_go_struct['}'] = MMJ_STATE_DOWN;
    mmj_go_struct['-'] = MMJ_STATE_BARE;
    mmj_go_struct['t'] = MMJ_STATE_BARE;
    mmj_go_struct['f'] = MMJ_STATE_BARE;
    mmj_go_struct['n'] = MMJ_STATE_BARE;

    for (i = 32; i <= 126; ++i)
        mmj_go_bare[i] = MMJ_STATE_LOOP;
    mmj_go_bare['\t'] = MMJ_STATE_UNBARE;
    mmj_go_bare['\r'] = MMJ_STATE_UNBARE;
    mmj_go_bare['\n'] = MMJ_STATE_UNBARE;
    mmj_go_bare[','] = MMJ_STATE_UNBARE;
    mmj_go_bare[']'] = MMJ_STATE_UNBARE;
    mmj_go_bare['}'] = MMJ_STATE_UNBARE;

    for (i = 32; i <= 126; ++i)
        mmj_go_string[i] = MMJ_STATE_LOOP;
    for (i = 192; i <= 223; ++i)
        mmj_go_string[i] = MMJ_STATE_UTF8_2;
    for (i = 224; i <= 239; ++i)
        mmj_go_string[i] = MMJ_STATE_UTF8_3;
    for (i = 240; i <= 247; ++i)
        mmj_go_string[i] = MMJ_STATE_UTF8_4;
    mmj_go_string['\\'] = MMJ_STATE_ESC;
    mmj_go_string['"'] = MMJ_STATE_QDOWN;
    for (i = 128; i <= 191; ++i)
        mmj_go_utf8[i] = MMJ_STATE_UTF8_NEXT;

    mmj_go_esc['"'] = MMJ_STATE_UNESC;
    mmj_go_esc['\\'] = MMJ_STATE_UNESC;
    mmj_go_esc['/'] = MMJ_STATE_UNESC;
    mmj_go_esc['b'] = MMJ_STATE_UNESC;
    mmj_go_esc['f'] = MMJ_STATE_UNESC;
    mmj_go_esc['n'] = MMJ_STATE_UNESC;
    mmj_go_esc['r'] = MMJ_STATE_UNESC;
    mmj_go_esc['t'] = MMJ_STATE_UNESC;
    mmj_go_esc['u'] = MMJ_STATE_UNESC;

    for (i = 48; i <= 57; ++i)
        mmj_go_num[i] = MMJ_STATE_LOOP;
    mmj_go_num['-'] = MMJ_STATE_NUM_LOOP;
    mmj_go_num['+'] = MMJ_STATE_NUM_LOOP;
    mmj_go_num['.'] = MMJ_STATE_NUM_FLT;
    mmj_go_num['e'] = MMJ_STATE_NUM_EXP;
    mmj_go_num['E'] = MMJ_STATE_NUM_EXP;
    mmj_go_num[' '] = MMJ_STATE_NUM_BREAK;
    mmj_go_num['\n'] = MMJ_STATE_NUM_BREAK;
    mmj_go_num['\t'] = MMJ_STATE_NUM_BREAK;
    mmj_go_num['\r'] = MMJ_STATE_NUM_BREAK;
}

/* checks and returns the type of a token */
static enum mmj_token_type
mmj_type(const struct mmj_token *tok)
{
    if (!tok || !tok->str || !tok->len)
        return MMJ_NONE;
    if (tok->str[0] == '{')
        return MMJ_OBJECT;
    if (tok->str[0] == '[')
        return MMJ_ARRAY;
    if (tok->str[0] == '\"')
        return MMJ_STRING;
    if (tok->str[0] == 't')
        return MMJ_TRUE;
    if (tok->str[0] == 'f')
        return MMJ_FALSE;
    if (tok->str[0] == 'n')
        return MMJ_NULL;
    return MMJ_NUMBER;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

/* dequotes a string token */
static void
mmj_deq(struct mmj_token *tok)
{
    if (tok->str[0] == '\"') {
        tok->str++; tok->len-=2;
    }
}

/* simple power function for json exponent numbers */
static mmj_number
mmj_ipow(int base, unsigned exp)
{
    long res = 1;
    while (exp) {
        if (exp & 1)
            res *= base;
        exp >>= 1;
        base *= base;
    }
    return (mmj_number)res;
}

/* converts a token containing a integer value into a number */
static mmj_number
mmj_stoi(struct mmj_token *tok)
{
    mmj_number n = 0;
    mmj_size i = 0;
    mmj_size off;
    mmj_size neg;
    if (!tok->str || !tok->len)
        return 0;

    off = (tok->str[0] == '-' || tok->str[0] == '+') ? 1 : 0;
    neg = (tok->str[0] == '-') ? 1 : 0;
    for (i = off; i < tok->len; i++) {
        if ((tok->str[i] >= '0') && (tok->str[i] <= '9'))
            n = (n * 10) + tok->str[i]  - '0';
    }
    return (neg) ? -n : n;
}

/* converts a token containing a real value into a floating point number */
static mmj_number
mmj_stof(struct mmj_token *tok)
{
    mmj_size i = 0;
    mmj_number n = 0;
    mmj_number f = 0.1;
    if (!tok->str || !tok->len) return 0;
    for (i = 0; i < tok->len; i++) {
        if ((tok->str[i] >= '0') && (tok->str[i] <= '9')) {
            n = n + (tok->str[i] - '0') * f;
            f *= 0.1;
        }
    }
    return n;
}

/* compares a size limited string with a string inside a token */
static mmj_int
mmj_lcmp(const struct mmj_token* tok, const mmj_char* str, mmj_size len)
{
    mmj_size i;
    MMJ_ASSERT(tok);
    MMJ_ASSERT(str);
    if (!tok || !str || !len) return 1;
    for (i = 0; (i < tok->len && i < len); i++, str++){
        if (tok->str[i] != *str)
            return 1;
    }
    return 0;
}

/*--------------------------------------------------------------------------
                            UTILITY
  -------------------------------------------------------------------------*/
MMJ_API mmj_int
mmj_convert(mmj_number *num, const struct mmj_token *tok)
{
    mmj_size len;
    const mmj_char *cur;
    mmj_number i, f, e, p;
    enum {INT, FLT, EXP, TOKS};
    struct mmj_token nums[TOKS];
    struct mmj_token *write = &nums[INT];

    MMJ_ASSERT(num);
    MMJ_ASSERT(tok);
    if (!num || !tok || !tok->str || !tok->len)
        return MMJ_NONE;

    nums[INT] = MMJ_TOKEN_NULL;
    nums[FLT] = MMJ_TOKEN_NULL;
    nums[EXP] = MMJ_TOKEN_NULL;
    len = tok->len;
    write->str = tok->str;

    for (cur = tok->str; len; cur++, len--) {
        char state =  mmj_go_num[*(const unsigned char*)cur];
        switch (state) {
            case MMJ_STATE_NUM_FAILED: {
                return MMJ_NONE;
            } break;
            case MMJ_STATE_NUM_FLT: {
                if (nums[FLT].str)
                    return MMJ_NONE;
                if (nums[EXP].str)
                    return MMJ_NONE;
                write->len = (mmj_size)(cur - write->str);
                write = &nums[FLT];
                write->str = cur + 1;
            } break;
            case MMJ_STATE_NUM_EXP: {
                if (nums[EXP].str)
                    return MMJ_NONE;
                write->len = (mmj_size)(cur - write->str);
                write = &nums[EXP];
                write->str = cur + 1;
            } break;
            case MMJ_STATE_NUM_BREAK: {
                len = 1;
            } break;
            default: break;
        }
    }
    write->len = (mmj_size)(cur - write->str);

    i = mmj_stoi(&nums[INT]);
    f = mmj_stof(&nums[FLT]);
    e = mmj_stoi(&nums[EXP]);
    p = mmj_ipow(10, (unsigned)((e < 0) ? -e : e));
    if (e < 0) p = (1 / p);
    *num = (i + ((i < 0) ? -f : f)) * p;
    return MMJ_NUMBER;
}

MMJ_API mmj_size
mmj_cpy(mmj_char *dst, mmj_size max, const struct mmj_token* tok)
{
    unsigned i = 0;
    mmj_size ret;
    mmj_size siz;

    MMJ_ASSERT(dst);
    MMJ_ASSERT(tok);
    if (!dst || !max || !tok)
        return 0;

    ret = (max < (tok->len + 1)) ? max : tok->len;
    siz = (max < (tok->len + 1)) ? max-1 : tok->len;
    for (i = 0; i < siz; i++)
        dst[i] = tok->str[i];
    dst[siz] = '\0';
    return ret;
}

MMJ_API mmj_int
mmj_cmp(const struct mmj_token* tok, const mmj_char* str)
{
    mmj_size i;
    MMJ_ASSERT(tok);
    MMJ_ASSERT(str);
    if (!tok || !str) return 1;
    for (i = 0; (i < tok->len && *str); i++, str++){
        if (tok->str[i] != *str)
            return 1;
    }
    return 0;
}
/*--------------------------------------------------------------------------
                            TOKENIZER
  -------------------------------------------------------------------------*/
MMJ_API struct mmj_iter
mmj_begin(const mmj_char *str, mmj_size len)
{
    struct mmj_iter iter = MMJ_ITER_NULL;
    mmj_init();
    iter.src = str;
    iter.len = len;
    return iter;
}

MMJ_API struct mmj_iter
mmj_read(struct mmj_token *obj, const struct mmj_iter* prev)
{
    struct mmj_iter iter;
    mmj_size len;
    const mmj_char *cur;
    mmj_int utf8_remain = 0;

    MMJ_ASSERT(obj);
    MMJ_ASSERT(prev);
    if (!prev || !obj || !prev->src || !prev->len || prev->err) {
        /* case either invalid iterator or eof  */
        struct mmj_iter it = MMJ_ITER_NULL;
        *obj = MMJ_TOKEN_NULL;
        it.err = 1;
        return it;
    }

    iter = *prev;
    *obj = MMJ_TOKEN_NULL;
    iter.err = 0;
    if (!iter.go) /* begin of parsing process */
        iter.go = mmj_go_struct;

    len = iter.len;
    for (cur = iter.src; len; cur++, len--) {
        const unsigned char *tbl = (const unsigned char*)iter.go;
        unsigned char c = (unsigned char)*cur;
        if (c == '\0') {
            iter.src = 0;
            iter.len = 0;
        }
        switch (tbl[c]) {
        case MMJ_STATE_FAILED: {
            iter.err = 1;
            return iter;
        } break;
        case MMJ_STATE_LOOP: break;
        case MMJ_STATE_SEP: {
            if (iter.depth == 2)
                obj->children--;
        } break;
        case MMJ_STATE_UP: {
            if (iter.depth > 1) {
                if (iter.depth == 2)
                    obj->children++;
                obj->sub++;
            }
            if (iter.depth++ == 1)
                obj->str = cur;
        } break;
        case MMJ_STATE_DOWN: {
            if (--iter.depth == 1) {
                obj->len = (mmj_size)(cur - obj->str) + 1;
                if (iter.depth != 1 || !obj->str)
                    goto l_loop;
                goto l_yield;
            }
        } break;
        case MMJ_STATE_QUP: {
            iter.go = mmj_go_string;
            if (iter.depth == 1) {
                obj->str = cur;
            } else {
                if (iter.depth == 2)
                    obj->children++;
                obj->sub++;
            }
        } break;
        case MMJ_STATE_QDOWN: {
            iter.go = mmj_go_struct;
            if (iter.depth == 1) {
                obj->len = (mmj_size)(cur - obj->str) + 1;
                if (iter.depth != 1 || !obj->str)
                    goto l_loop;
                goto l_yield;
            }
        } break;
        case MMJ_STATE_ESC: {
            iter.go = mmj_go_esc;
        } break;
        case MMJ_STATE_UNESC: {
            iter.go = mmj_go_string;
        } break;
        case MMJ_STATE_BARE: {
            if (iter.depth == 1) {
                obj->str = cur;
            } else {
                if (iter.depth == 2)
                    obj->children++;
                obj->sub++;
            }
            iter.go = mmj_go_bare;
        } break;
        case MMJ_STATE_UNBARE: {
            iter.go = mmj_go_struct;
            if (iter.depth == 1) {
                obj->len = (mmj_size)(cur - obj->str);
                obj->type = (enum mmj_token_type)mmj_type(obj);
                if (obj->type == MMJ_STRING)
                    mmj_deq(obj);
                iter.src = cur;
                iter.len = len;
                return iter;
            }
            cur--; len++;
        } break;
        case MMJ_STATE_UTF8_2: {
            iter.go = mmj_go_utf8;
            utf8_remain = 1;
        } break;
        case MMJ_STATE_UTF8_3: {
            iter.go = mmj_go_utf8;
            utf8_remain = 2;
        } break;
        case MMJ_STATE_UTF8_4: {
            iter.go = mmj_go_utf8;
            utf8_remain = 3;
        } break;
        case MMJ_STATE_UTF8_NEXT: {
            if (!--utf8_remain)
                iter.go = mmj_go_string;
        } break;
        default:
            break;
        }
        l_loop:;
    }

    if (!iter.depth) {
        /* reached eof */
        iter.src = 0;
        iter.len = 0;
        if (obj->str) {
            obj->len = (mmj_size)((cur-1) - obj->str);
            obj->type = (enum mmj_token_type)mmj_type(obj);
            if (obj->type == MMJ_STRING)
                mmj_deq(obj);
        }
        return iter;
    }
    return iter;

l_yield:
    iter.src = cur + 1;
    iter.len = len - 1;
    obj->type = mmj_type(obj);
    if (obj->type == MMJ_STRING)
        mmj_deq(obj);
    return iter;
}

MMJ_API struct mmj_iter
mmj_parse(struct mmj_pair *p, const struct mmj_iter* it)
{
    struct mmj_iter next;
    MMJ_ASSERT(p);
    MMJ_ASSERT(it);
    next = mmj_read(&p->name, it);
    if (next.err) return next;
    return mmj_read(&p->value, &next);
}

/*--------------------------------------------------------------------------
                            PARSER
  -------------------------------------------------------------------------*/
MMJ_API mmj_size
mmj_num(const mmj_char *json, mmj_size length)
{
    struct mmj_iter iter;
    struct mmj_token tok;
    mmj_size count = 0;

    MMJ_ASSERT(json);
    MMJ_ASSERT(length);
    if (!json || !length)
        return 0;

    iter = mmj_begin(json, length);
    iter = mmj_read(&tok, &iter);
    while (!iter.err) {
        count += (1 + tok.sub);
        iter = mmj_read(&tok, &iter);
    }
    return count;
}

MMJ_API enum mmj_status
mmj_load(struct mmj_token *toks, mmj_size max, mmj_size *read,
            const mmj_char *json, mmj_size length)
{
    enum mmj_status status = MMJ_OK;
    struct mmj_token tok;
    struct mmj_iter iter;

    MMJ_ASSERT(toks);
    MMJ_ASSERT(json);
    MMJ_ASSERT(length);
    MMJ_ASSERT(max);
    MMJ_ASSERT(read);

    if (!toks || !json || !length || !max || !read)
        return MMJ_INVAL;
    if (*read >= max)
        return MMJ_OUT_OF_TOKEN;

    iter = mmj_begin(json, length);
    iter = mmj_read(&tok, &iter);
    if (iter.err && iter.len)
        return MMJ_PARSING_ERROR;

    while (iter.len) {
        toks[*read] = tok;
        *read += 1;
        if (*read > max) return MMJ_OUT_OF_TOKEN;
        if (toks[*read-1].type == MMJ_OBJECT ||  toks[*read-1].type == MMJ_ARRAY) {
            status = mmj_load(toks, max, read, toks[*read-1].str, toks[*read-1].len);
            if (status != MMJ_OK) return status;
        }

        iter = mmj_read(&tok, &iter);
        if (iter.err && iter.src && iter.len)
            return MMJ_PARSING_ERROR;
    }
    return status;
}
/*--------------------------------------------------------------------------
                            QUERY
  -------------------------------------------------------------------------*/
static const mmj_char*
mmj_strchr(const mmj_char *str, mmj_char c, mmj_int len)
{
    mmj_int neg = (len < 0) ? 1: 0;
    mmj_int dec = neg ? 0 : 1;
    len = neg ? 0 : len;
    if (!str) return NULL;
    while (*str && len >= 0) {
        if (*str == c)
            return str;
        len -= dec;
        str++;
    }
    if (neg) return str;
    return NULL;
}


static const mmj_char*
mmj_path_parse_name(struct mmj_token *tok, const mmj_char *path,
    mmj_char delimiter)
{
    const mmj_char *del;
    const mmj_char *begin, *end;
    if (!path || *path == '\0')
        return NULL;

    tok->str = path;
    del = mmj_strchr(tok->str, delimiter, -1);
    begin = mmj_strchr(tok->str, '[', -1);
    end = mmj_strchr(tok->str, ']', -1);

    /* array part left */
    if (begin && end && begin == tok->str) {
        tok->len = (mmj_size)((end - begin) + 1);
        if (*(end + 1) == '\0')
            return NULL;
        if (*(end + 1) == '.')
            return(end + 2);
        else return(end + 1);
    }

    /* only array after name */
    if (begin < del) {
        tok->len = (mmj_size)(begin - tok->str);
        return begin;
    }

    if (!del) return NULL;
    if (*del == '\0') {
        tok->len = (mmj_size)(del - tok->str);
        return NULL;
    }
    tok->len = (mmj_size)(del - tok->str);
    return del+1;
}

static mmj_int
mmj_path_parse_array(struct mmj_token *array, const struct mmj_token *token)
{
    const mmj_char *begin;
    const mmj_char *end;

    array->str = token->str;
    begin = mmj_strchr(array->str, '[', (mmj_int)token->len);
    if (!begin || ((mmj_size)(begin - array->str) >= token->len))
        return 0;

    end = mmj_strchr(begin, ']', (mmj_int)(token->len - (mmj_size)(begin - array->str)));
    if (!end || ((mmj_size)(end - array->str) >= token->len))
        return 0;

    array->str = begin + 1;
    array->len = (mmj_size)((end-1) - begin);
    return 1;
}

MMJ_API struct mmj_token*
mmj_query_del(struct mmj_token *toks, mmj_size count,
    const mmj_char *path, mmj_char delemiter)
{
    mmj_size i = 0;
    mmj_int begin = 1;
    struct mmj_token *iter = NULL;
    /* iterator to step over each token in the toks array */
    struct mmj_token name;
    /* current segment in the path to search in the tree for */
    struct mmj_token array;
    /* array token to store the current path segment array index */
    struct mmj_object {mmj_size index, size;} obj;
    /* current object iterator with current pair index and total pairs in object */

    MMJ_ASSERT(toks);
    MMJ_ASSERT(count);
    if (!toks || !count) return iter;
    if (!path) return &toks[i];

    iter = &toks[i];
    array.len = 0;
    array.str = NULL;

    path = mmj_path_parse_name(&name, path, delemiter);
    while (1) {
        if (iter->type == MMJ_OBJECT || iter->type == MMJ_ARRAY || begin) {
            /* setup iteration over elements inside a object or array */
            obj.index = 0;
            if (begin) {
                begin = 0;
                obj.size = count;
            } else if (iter->type == MMJ_OBJECT) {
                obj.size = iter->children;
                if ((i + 1) > count) return NULL;
                iter = &toks[++i];
            } else {
                mmj_number n;
                mmj_size j = 0;

                /* array object so set iterator to array index */
                if (!mmj_path_parse_array(&array, &name))
                    return NULL;
                if ((i+1) >= count)
                    return NULL;
                if (array.len < 1)
                    return NULL;
                if (mmj_convert(&n, &array) != MMJ_NUMBER)
                    return NULL;
                if ((mmj_size)n >= iter->children)
                    return NULL;
                array.str = NULL;
                array.len = 0;

                /* iterate over each array element and find the correct index */
                iter++; i++;
                for (j = 0; j < n; ++j) {
                    if (iter->type == MMJ_ARRAY || iter->type == MMJ_OBJECT) {
                        i = i + (iter->sub) + 1;
                    } else i += 1;
                    if (i > count)
                        return NULL;
                    iter = &toks[i];
                }
                if (!path) return iter;
                path = mmj_path_parse_name(&name, path, delemiter);
            }
            continue;
        }
        {
            /* check if current table element is equal to the current path  */
            if (!mmj_lcmp(iter, name.str, name.len)) {
                /* correct token found and end of path */
                if (!path) {
                    if ((i + 1) > count)
                        return NULL;
                    return (iter + 1);
                }
                /* check if path points to invalid token */
                if ((i+1) > count)
                    return NULL;
                if(toks[i+1].type != MMJ_OBJECT && toks[i+1].type != MMJ_ARRAY)
                    return NULL;

                /* look deeper into child object/array */
                iter = &toks[++i];
                path = mmj_path_parse_name(&name, path, delemiter);
            } else {
                /* key is not correct iterate until end of object */
                if (++obj.index >= obj.size)
                    return NULL;
                if ((i + 1) >= count)
                    return NULL;
                if (iter[1].type == MMJ_ARRAY || iter[1].type == MMJ_OBJECT) {
                    i = i + (iter[1].sub + 2);
                } else i = i + 2;
                if (i >= count)
                    return NULL;
                iter = &toks[i];
            }
        }
    }
    return iter;
}

MMJ_API struct mmj_token*
mmj_query(struct mmj_token *toks, mmj_size count, const mmj_char *path)
{return mmj_query_del(toks, count, path, MMJ_DELIMITER);}

MMJ_API mmj_int
mmj_query_number_del(mmj_number *num, struct mmj_token *toks, mmj_size count,
    const mmj_char *path, mmj_char del)
{
    struct mmj_token *tok;
    MMJ_ASSERT(toks);
    MMJ_ASSERT(count);
    MMJ_ASSERT(num);
    MMJ_ASSERT(path);
    if (!toks || !count || !num || !path)
        return MMJ_NONE;

    tok = mmj_query_del(toks, count, path, del);
    if (!tok) return MMJ_NONE;
    if (tok->type != MMJ_NUMBER)
        return tok->type;
    return mmj_convert(num, tok);
}

MMJ_API mmj_int
mmj_query_number(mmj_number *num, struct mmj_token *toks, mmj_size count,
    const mmj_char *path)
{return mmj_query_number_del(num, toks, count, path, MMJ_DELIMITER);}

MMJ_API mmj_int
mmj_query_string_del(mmj_char *buffer, mmj_size max, mmj_size *size,
    struct mmj_token *toks, mmj_size count, const mmj_char *path, mmj_char del)
{
    struct mmj_token *tok;
    MMJ_ASSERT(toks);
    MMJ_ASSERT(count);
    MMJ_ASSERT(buffer);
    MMJ_ASSERT(size);
    MMJ_ASSERT(path);
    if (!toks || !count || !buffer || !size || !path)
        return MMJ_NONE;

    tok = mmj_query_del(toks, count, path, del);
    if (!tok) return MMJ_NONE;
    if (tok->type != MMJ_STRING)
        return tok->type;
    *size = mmj_cpy(buffer, max, tok);
    return tok->type;
}

MMJ_API mmj_int
mmj_query_string(mmj_char *buffer, mmj_size max, mmj_size *size,
    struct mmj_token *toks, mmj_size count, const mmj_char *path)
{return mmj_query_string_del(buffer, max, size, toks, count, path, MMJ_DELIMITER);}

MMJ_API mmj_int
mmj_query_type_del(struct mmj_token *toks, mmj_size count, const mmj_char *path,
    mmj_char del)
{
    struct mmj_token *tok;
    MMJ_ASSERT(toks);
    MMJ_ASSERT(count);
    MMJ_ASSERT(path);
    if (!toks || !count || !path)
        return MMJ_NONE;

    tok = mmj_query_del(toks, count, path, del);
    if (!tok) return MMJ_NONE;
    return tok->type;
}

MMJ_API mmj_int
mmj_query_type(struct mmj_token *toks, mmj_size count, const mmj_char *path)
{return mmj_query_type_del(toks, count, path, MMJ_DELIMITER);}

#endif
