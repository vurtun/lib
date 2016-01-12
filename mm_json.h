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
    #define MM_JSON_IMPLEMENTATION
    before you include this file in *one* C or C++ file to create the implementation

    If you want to keep the implementation in that file you have to do
    #define MM_JSON_STATIC before including this file

    If you want to use asserts to add validation add
    #define MM_JSON_ASSERT before including this file

    To overwrite the default seperator character used inside
    the query functions
    #define MM_JSON_DELIMITER (character) before including this file

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
    to call `mm_json_init` once before using.

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

USAGE:
    This file behaves differently depending on what symbols you define
    before including it.

    Header-File mode:
    If you do not define MM_JSON_IMPLEMENTATION before including this file, it
    will operate in header only mode. In this mode it declares all used structs
    and the API of the library without including the implementation of the library.

    Implementation mode:
    If you define MM_JSON_IMPLEMENTATIOn before including this file, it will
    compile the implementation of the JSON parser. To specify the visibility
    as private and limit all symbols inside the implementation file
    you can define MM_JSON_STATIC before including this file.
    Make sure that you only include this file implementation in *one* C or C++ file
    to prevent collisions.

EXAMPLES:*/
#if 0
    /* Lexer example */
    const char *json = "{a:"test",b:5, c:[0,1,2,4,5]}";
    mm_json_size len = strlen(json);

    /* create iterator  */
    struct mm_json_iter iter;
    iter = mm_json_begin(json, len);

    /* read token pair */
    struct mm_json_pair pair;
    iter = mm_json_parse(&pair, &iter);
    assert(!mm_json_cmp(&pair.name, "a"));
    assert(!mm_json_cmp(&pair.value, "test"));
    assert(pair.value.type == MM_JSON_STRING);

    /* convert token to number */
    mm_json_number num;
    iter = mm_json_parse(&pair, &iter);
    mm_json_test_assert(mm_json_convert(&num, &pair.value) == MM_JSON_NUMBER);
    assert(num == 5.0);

    /* read subobject (array/objects) */
    iter = mm_json_parse(&pair, &iter);
    mm_json_test_assert(pair.value.type == MM_JSON_ARRAY);

    struct mm_json_iter array = mm_json_begin(pair.value.str, pair.value.len);
    iter = mm_json_read(&tok, &array);
    while (array.src) {
        /* read single token */
        array = mm_json_read(&tok, &array);
    }
#endif
#if 0
    /* Parser example */
    const char *json = "{...}";
    mm_json_size len = strlen(json);

    /* load content into token array */
    mm_json_size read = 0;
    mm_json_size num = mm_json_num(json, len);
    struct mm_json_token *toks = calloc(num, sizeof(struct mm_json_token));
    mm_json_load(toks, num, &read, json, len);

    /* query token */
    struct mm_json_token * t0 = mm_json_query(toks, num, "map.entity[4].position");
    struct mm_json_token * t1 = mm_json_query_del(toks, num, "map:entity[4]:position", ':');

    /* query string */
    char buffer[64];
    mm_json_size size;
    mm_json_query_string(buffer, 64, &size, toks, num, "map.entity[4].name");
    mm_json_query_string_del(buffer, 64, &size, toks, num, "map%entity[9]%name", '%');

    /* query number */
    mm_json_number num;
    mm_json_query_number(&num, toks, num, "map.soldier[2].position.x");
    mm_json_query_number_del(&num, toks, num, "map/soldier[2]/position.x", "/");

    /* query type */
    mm_json_int type0 = mm_json_query_number(toks, num, "map.soldier[2]);
    mm_json_int type1 = mm_json_query_number_del(toks, num, "map_soldier[2], '_');

    /* subqueries */
    json_token *entity = mm_json_query(toks, num, "map.entity[4]");
    json_token *position = mm_json_query(entity, entity->sub, "position");
    json_token *rotation = mm_json_query(entity, entity->sub, "rotation");
#endif

 /* ===============================================================
 *
 *                          HEADER
 *
 * =============================================================== */
#ifndef MM_JSON_H_
#define MM_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MM_JSON_STATIC
#define MM_JSON_API static
#else
#define MM_JSON_API extern
#endif

typedef int mm_json_int;
typedef char mm_json_char;
typedef unsigned long mm_json_size;
typedef double mm_json_number;

/* default delimiter used in the `mm_json_query` function to seperate elements path */
#ifndef MM_JSON_DELIMITER
#define MM_JSON_DELIMITER '.'
#endif

enum mm_json_token_type {
    MM_JSON_NONE,      /* invalid token */
    MM_JSON_OBJECT,    /* subobject */
    MM_JSON_ARRAY,     /* subarray */
    MM_JSON_NUMBER,    /* float point number token */
    MM_JSON_STRING,    /* string text token */
    MM_JSON_TRUE,      /* true constant token */
    MM_JSON_FALSE,     /* false constant token*/
    MM_JSON_NULL,      /* null constant token */
    MM_JSON_MAX
};

struct mm_json_token {
    /*A Token represents a section in a string containing a valid json DOM value.
    Since the token only references the json object string, no actual memory needs
    to be allocated from the library to hold each token. Therefore the library does not
    represent a classical datastructure representing the json object as a tree, instead
    it is only responsible for parsing the string. In addition it is to note that the string
    inside of the token is not terminated with '\0' since it would require write
    access to the json string which is not always wanted. */
    const mm_json_char *str;
    /* pointer to the beginning of the token */
    mm_json_size len;
    /* length of the token */
    mm_json_size children;
    /* number of direct child tokens */
    mm_json_size sub;
    /* total number of subtokens (note: not pairs). Used for querying inside
     * previsouly queried tokens */
    enum mm_json_token_type type;
    /* token type */
};

struct mm_json_pair {
    /* A Pair represents two tokens with name and value token. Every Object, Array and
    Element in a Object except values inside a Array are pairs. So it is possible
    to read in two tokens directly at once while still getting correct results. */
    struct mm_json_token name;
    /* token representing the name of the pair */
    struct mm_json_token value;
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
    mm_json_find   -- queries a token array with a path to a key and returns the value
    mm_json_cpy    -- copies a prev read token into a user provided buffer and returns the len
    mm_json_cmp    -- compares a prev read token with a user provided string
    mm_json_convert-- converts a previously parsed token into a number
    mm_json_init   -- initilizes the parser look up tables only needs to be called
                    before the first use and if the library is used with multithreading
*/
MM_JSON_API mm_json_int mm_json_cmp(const struct mm_json_token*, const mm_json_char*);
/*  this function compares a previously read token with a user provieded string
    Input:
    - previously parsed token that needs to be compared against
    - user provided string
    Output:
    - if both are equal 0 otherwise 1
*/
MM_JSON_API mm_json_size mm_json_cpy(mm_json_char*, mm_json_size, const struct mm_json_token*);
/*  this function copies a previously read token into a user provided buffer
    Input:
    - max fill size of the buffer
    - previously parsed token that will be copied into the buffer
    Output:
    - buffer containing the token
*/
MM_JSON_API mm_json_int mm_json_convert(mm_json_number *, const struct mm_json_token*);
/*  this function converts a previously read token into a number
    Input:
    - previously parsed token that needs to be converted
    Output:
    - number that has been converted from the token
    - MM_JSON_NUMBER on success or mm_json_NOME on failure
*/
MM_JSON_API void mm_json_init(void);
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
    `mm_json_begin`. After that the iterator can be used to iterate over the first
    depth of the DOM-Tree. This is done by either using `mm_json_parse` for pairs
    or `mm_json_read` or single tokens which is useful for array values.

    Tokenzier API (based around an iterator)
    mm_json_begin  -- creates a parsing lexer iterator from a json string and a string length
    mm_json_read   -- parses a token and returns the iterator to the next token
    mm_json_parse  -- parses a token pair and returns the iterator to the next token
*/
struct mm_json_iter {
    /* The lexer iterator controlls the parsing procedure and is similar to the
    container iterator found in the C++ STL library. Important to note is that
    the iterator only works up to the first depth of a JSON DOM tree, but a
    new iterator can be created with the parsed output. */
    mm_json_int err;
    /* flag indicating if a parsing error or EOF took place */
    mm_json_int depth;
    /* current depth of the parsed DOM JSON tree iterator */
    const char *go;
    /* current jump table used in the parser */
    const mm_json_char *src;
    /* current pointer to the parsed string */
    mm_json_size len;
    /* curent length of the current parsed string */
};

MM_JSON_API struct mm_json_iter mm_json_begin(const mm_json_char *json, mm_json_size length);
/*  this function initializes an lexer iterator
    Input:
    - json is a string containing a json DOM object that needs to be parsed
    - length descripes the number of bytes that makeup the DOM object
    Output:
    - new lexer iterator ready to parse the specified string
*/
MM_JSON_API struct mm_json_iter mm_json_read(struct mm_json_token*, const struct mm_json_iter*);
/*  this function uses the lexer to read in one single token and returns a
    a iterator pointing to the next token.
    Input:
    - lexer iterator pointing to a valid json string object
    Output:
    - lexer iterator ready to parse the specified string
    - token that will be read from the lexer iterator
*/
MM_JSON_API struct mm_json_iter mm_json_parse(struct mm_json_pair*, const struct mm_json_iter*);
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
    The parser has to function. The first function `mm_json_num` calculates
    the needed number of tokens, while the `mm_json_load` function reads in all tokens.

    Retain mode parsing API (based on preallocated token array)
    mm_json_num    -- returns the total number of tokens inside a json DOM tree
    mm_json_load   -- loads all tokens in a json DOM tree into tokens
*/
enum mm_json_status {
    MM_JSON_OK = 0,
    /* The parsing process was successfull and no problem were found */
    MM_JSON_INVAL,
    /* A invalid value has been passed into the function */
    MM_JSON_OUT_OF_TOKEN,
    /* Not enough tokens have been passed so the parsing process had to be stopped*/
    MM_JSON_PARSING_ERROR
    /* A invalid token or wrong json string has been passed */
};

MM_JSON_API mm_json_size mm_json_num(const mm_json_char *json, mm_json_size length);
/*  this function calculates the number of tokens inside the JSON DOM object
    Input:
    - json is a string containing a json DOM object that needs to be parsed
    - length descripes the number of bytes that makeup the DOM object
    Output:
    - number of tokens inside the json string
*/
MM_JSON_API enum mm_json_status mm_json_load(struct mm_json_token *toks, mm_json_size max,
                                            mm_json_size *read, const mm_json_char *json,
                                            mm_json_size length);
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
    by a delemiter. The delimiter is defined in macro MM_JSON_DELIMITER but
    each query function provides a additional function with delimiter argument.

    USAGE
    ------------------------
    To query for a token or token content you first have to load the content
    of the json string into a token array by using the parser API. After that
    you can call each query function on the token array.

    Query API (based on parser token array output)
    mm_json_query          -- finds a token at a given path in the token array
    mm_json_query_number   -- finds and converts a token at a given path to a number
    mm_json_query_string   -- finds and buffers a string token at a given path
    mm_json_query_type     -- returns the type of a token at a given path in the token array
*/
MM_JSON_API struct mm_json_token *mm_json_query(struct mm_json_token *toks, mm_json_size count,
                                                const mm_json_char *path);
MM_JSON_API struct mm_json_token *mm_json_query_del(struct mm_json_token *toks, mm_json_size count,
                                                    const mm_json_char *path, mm_json_char del);
/*  this function trys to find a certain node in the JSON-DOM tree by path
    Input:
    - toks is a token array that has to be previously parsed
    - count is the number of tokens inside the array
    - path is a sequence of node names to a destiniation inside the tree seperated
        by '.' eg.: map.entity[0].position.x (case sensitive) and [i] for arrays
    [-del is the delemiter between each key name in the path or if not set MM_JSON_DELIMITER]
    Output:
    - the requested token if it exist otherwise NULL
*/
MM_JSON_API mm_json_int mm_json_query_number(mm_json_number*, struct mm_json_token *toks,
                                            mm_json_size count, const mm_json_char *path);
MM_JSON_API mm_json_int mm_json_query_number_del(mm_json_number*, struct mm_json_token *toks,
                                                mm_json_size count, const mm_json_char *path,
                                                mm_json_char del);
/*  this function trys to find and convert a certain token by path to number
    Input:
    - toks is a token array that has to be previously parsed
    - count is the number of tokens inside the array
    - path is a sequence of node names to a destiniation inside the tree seperated
        by '.' eg.: map.entity[0].position.x (case sensitive) and [i] for arrays
    [-del is the delemiter between each key name in the path or if not set MM_JSON_DELIMITER]
    Output:
    - number that has been converted from the token
    - MM_JSON_NUMBER on success, MM_JSON_NONE on not found, token type otherwise
*/
MM_JSON_API mm_json_int mm_json_query_string(mm_json_char*, mm_json_size max, mm_json_size *size,
                                            struct mm_json_token *toks, mm_json_size count,
                                            const mm_json_char *path);
MM_JSON_API mm_json_int mm_json_query_string_del(mm_json_char*, mm_json_size max, mm_json_size *size,
                                            struct mm_json_token *toks, mm_json_size count,
                                            const mm_json_char *path, mm_json_char del);
/*  this function trys to find and buffer a certain string token by path
    Input:
    - maximum number of bytes inside the buffer
    - toks is a token array that has to be previously parsed
    - count is the number of tokens inside the array
    - path is a sequence of node names to a destiniation inside the tree seperated
        by '.' eg.: map.entity[0].position.x (case sensitive) and [i] for arrays
    [-del is the delemiter between each key name in the path or if not set MM_JSON_DELIMITER]
    Output:
    - buffer containing the token
    - size of the buffer in bytes
    - MM_JSON_STRING on success, MM_JSON_NONE on not found, token type otherwise
*/
MM_JSON_API mm_json_int mm_json_query_type(struct mm_json_token *toks, mm_json_size count,
                                            const mm_json_char *path);
MM_JSON_API mm_json_int mm_json_query_type_del(struct mm_json_token *toks, mm_json_size count,
                                            const mm_json_char *path, mm_json_char del);
/*  this function trys to find a token and return its type
    Input:
    - toks is a token array that has to be previously parsed
    - count is the number of tokens inside the array
    - path is a sequence of node names to a destiniation inside the tree seperated
        by '.' eg.: map.entity[0].position.x (case sensitive) and [i] for arrays
    [-del is the delemiter between each key name in the path or if not set MM_JSON_DELIMITER]
    Output:
    - MM_JSON_NONE if not found, token type otherwise
*/
#ifdef __cplusplus
}
#endif
#endif /* MM_JSON_H_ */

/*===============================================================
 *
 *                          IMPLEMENTATION
 *
 * =============================================================== */
#ifdef MM_JSON_IMPLEMENTATION
/* this flag inserts the <assert.h> header into the json.c file and adds
 assert call to every function in DEBUG mode. If activated then
 the clib will be used. So if you want to compile without clib then
 deactivate this flag. */
#ifdef MM_JSON_USE_ASSERT
#ifndef MM_JSON_ASSERT
#include <assert.h>
#define MM_JSON_ASSERT(expr) assert(expr)
#endif
#else
#define MM_JSON_ASSERT(expr)
#endif

#define MM_JSON_INTERN static
#define MM_JSON_GLOBAL static

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

/* Main token parsing function states */
enum mm_json_parser_states {
    MM_JSON_STATE_FAILED,
    MM_JSON_STATE_LOOP,
    MM_JSON_STATE_SEP,
    MM_JSON_STATE_UP,
    MM_JSON_STATE_DOWN,
    MM_JSON_STATE_QUP,
    MM_JSON_STATE_QDOWN,
    MM_JSON_STATE_ESC,
    MM_JSON_STATE_UNESC,
    MM_JSON_STATE_BARE,
    MM_JSON_STATE_UNBARE,
    MM_JSON_STATE_UTF8_2,
    MM_JSON_STATE_UTF8_3,
    MM_JSON_STATE_UTF8_4,
    MM_JSON_STATE_UTF8_NEXT,
    MM_JSON_STATE_MAX
};

/* Main token to number conversion states */
enum mm_json_nuber_states {
    MM_JSON_STATE_NUM_FAILED,
    MM_JSON_STATE_NUM_LOOP,
    MM_JSON_STATE_NUM_FLT,
    MM_JSON_STATE_NUM_EXP,
    MM_JSON_STATE_NUM_BREAK,
    MM_JSON_STATE_NUM_MAX
};

/* global parser jump tables */
MM_JSON_GLOBAL char mm_json_go_struct[256];
MM_JSON_GLOBAL char mm_json_go_bare[256];
MM_JSON_GLOBAL char mm_json_go_string[256];
MM_JSON_GLOBAL char mm_json_go_utf8[256];
MM_JSON_GLOBAL char mm_json_go_esc[256];
MM_JSON_GLOBAL char mm_json_go_num[256];
MM_JSON_GLOBAL const struct mm_json_iter MM_JSON_ITER_NULL = {0,0,0,0,0};
MM_JSON_GLOBAL const struct mm_json_token MM_JSON_TOKEN_NULL = {0,0,0,0,MM_JSON_NONE};
MM_JSON_GLOBAL int mm_json_is_initialized;

/*--------------------------------------------------------------------------
                                HELPER
  -------------------------------------------------------------------------*/
/* initializes the parser jump tables */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wchar-subscripts"
#endif

MM_JSON_API void
mm_json_init(void)
{
    int i;
    if (mm_json_is_initialized) return;
    mm_json_is_initialized = 1;
    for (i = 48; i <= 57; ++i)
        mm_json_go_struct[i] = MM_JSON_STATE_BARE;
    mm_json_go_struct['\t'] = MM_JSON_STATE_LOOP;
    mm_json_go_struct['\r'] = MM_JSON_STATE_LOOP;
    mm_json_go_struct['\n'] = MM_JSON_STATE_LOOP;
    mm_json_go_struct[' '] = MM_JSON_STATE_LOOP;
    mm_json_go_struct['"'] = MM_JSON_STATE_QUP;
    mm_json_go_struct[':'] = MM_JSON_STATE_SEP;
    mm_json_go_struct['='] = MM_JSON_STATE_SEP;
    mm_json_go_struct[','] = MM_JSON_STATE_LOOP;
    mm_json_go_struct['['] = MM_JSON_STATE_UP;
    mm_json_go_struct[']'] = MM_JSON_STATE_DOWN;
    mm_json_go_struct['{'] = MM_JSON_STATE_UP;
    mm_json_go_struct['}'] = MM_JSON_STATE_DOWN;
    mm_json_go_struct['-'] = MM_JSON_STATE_BARE;
    mm_json_go_struct['t'] = MM_JSON_STATE_BARE;
    mm_json_go_struct['f'] = MM_JSON_STATE_BARE;
    mm_json_go_struct['n'] = MM_JSON_STATE_BARE;

    for (i = 32; i <= 126; ++i)
        mm_json_go_bare[i] = MM_JSON_STATE_LOOP;
    mm_json_go_bare['\t'] = MM_JSON_STATE_UNBARE;
    mm_json_go_bare['\r'] = MM_JSON_STATE_UNBARE;
    mm_json_go_bare['\n'] = MM_JSON_STATE_UNBARE;
    mm_json_go_bare[','] = MM_JSON_STATE_UNBARE;
    mm_json_go_bare[']'] = MM_JSON_STATE_UNBARE;
    mm_json_go_bare['}'] = MM_JSON_STATE_UNBARE;

    for (i = 32; i <= 126; ++i)
        mm_json_go_string[i] = MM_JSON_STATE_LOOP;
    for (i = 192; i <= 223; ++i)
        mm_json_go_string[i] = MM_JSON_STATE_UTF8_2;
    for (i = 224; i <= 239; ++i)
        mm_json_go_string[i] = MM_JSON_STATE_UTF8_3;
    for (i = 240; i <= 247; ++i)
        mm_json_go_string[i] = MM_JSON_STATE_UTF8_4;
    mm_json_go_string['\\'] = MM_JSON_STATE_ESC;
    mm_json_go_string['"'] = MM_JSON_STATE_QDOWN;
    for (i = 128; i <= 191; ++i)
        mm_json_go_utf8[i] = MM_JSON_STATE_UTF8_NEXT;

    mm_json_go_esc['"'] = MM_JSON_STATE_UNESC;
    mm_json_go_esc['\\'] = MM_JSON_STATE_UNESC;
    mm_json_go_esc['/'] = MM_JSON_STATE_UNESC;
    mm_json_go_esc['b'] = MM_JSON_STATE_UNESC;
    mm_json_go_esc['f'] = MM_JSON_STATE_UNESC;
    mm_json_go_esc['n'] = MM_JSON_STATE_UNESC;
    mm_json_go_esc['r'] = MM_JSON_STATE_UNESC;
    mm_json_go_esc['t'] = MM_JSON_STATE_UNESC;
    mm_json_go_esc['u'] = MM_JSON_STATE_UNESC;

    for (i = 48; i <= 57; ++i)
        mm_json_go_num[i] = MM_JSON_STATE_LOOP;
    mm_json_go_num['-'] = MM_JSON_STATE_NUM_LOOP;
    mm_json_go_num['+'] = MM_JSON_STATE_NUM_LOOP;
    mm_json_go_num['.'] = MM_JSON_STATE_NUM_FLT;
    mm_json_go_num['e'] = MM_JSON_STATE_NUM_EXP;
    mm_json_go_num['E'] = MM_JSON_STATE_NUM_EXP;
    mm_json_go_num[' '] = MM_JSON_STATE_NUM_BREAK;
    mm_json_go_num['\n'] = MM_JSON_STATE_NUM_BREAK;
    mm_json_go_num['\t'] = MM_JSON_STATE_NUM_BREAK;
    mm_json_go_num['\r'] = MM_JSON_STATE_NUM_BREAK;
}

/* checks and returns the type of a token */
static enum mm_json_token_type
mm_json_type(const struct mm_json_token *tok)
{
    if (!tok || !tok->str || !tok->len)
        return MM_JSON_NONE;
    if (tok->str[0] == '{')
        return MM_JSON_OBJECT;
    if (tok->str[0] == '[')
        return MM_JSON_ARRAY;
    if (tok->str[0] == '\"')
        return MM_JSON_STRING;
    if (tok->str[0] == 't')
        return MM_JSON_TRUE;
    if (tok->str[0] == 'f')
        return MM_JSON_FALSE;
    if (tok->str[0] == 'n')
        return MM_JSON_NULL;
    return MM_JSON_NUMBER;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

/* dequotes a string token */
static void
mm_json_deq(struct mm_json_token *tok)
{
    if (tok->str[0] == '\"') {
        tok->str++; tok->len-=2;
    }
}

/* simple power function for json exponent numbers */
static mm_json_number
mm_json_ipow(int base, unsigned exp)
{
    long res = 1;
    while (exp) {
        if (exp & 1)
            res *= base;
        exp >>= 1;
        base *= base;
    }
    return (mm_json_number)res;
}

/* converts a token containing a integer value into a number */
static mm_json_number
mm_json_stoi(struct mm_json_token *tok)
{
    mm_json_number n = 0;
    mm_json_size i = 0;
    mm_json_size off;
    mm_json_size neg;
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
static mm_json_number
mm_json_stof(struct mm_json_token *tok)
{
    mm_json_size i = 0;
    mm_json_number n = 0;
    mm_json_number f = 0.1;
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
static mm_json_int
mm_json_lcmp(const struct mm_json_token* tok, const mm_json_char* str, mm_json_size len)
{
    mm_json_size i;
    MM_JSON_ASSERT(tok);
    MM_JSON_ASSERT(str);
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
MM_JSON_API mm_json_int
mm_json_convert(mm_json_number *num, const struct mm_json_token *tok)
{
    mm_json_size len;
    const mm_json_char *cur;
    mm_json_number i, f, e, p;
    enum {INT, FLT, EXP, TOKS};
    struct mm_json_token nums[TOKS];
    struct mm_json_token *write = &nums[INT];

    MM_JSON_ASSERT(num);
    MM_JSON_ASSERT(tok);
    if (!num || !tok || !tok->str || !tok->len)
        return MM_JSON_NONE;

    nums[INT] = MM_JSON_TOKEN_NULL;
    nums[FLT] = MM_JSON_TOKEN_NULL;
    nums[EXP] = MM_JSON_TOKEN_NULL;
    len = tok->len;
    write->str = tok->str;

    for (cur = tok->str; len; cur++, len--) {
        char state =  mm_json_go_num[*(const unsigned char*)cur];
        switch (state) {
            case MM_JSON_STATE_NUM_FAILED: {
                return MM_JSON_NONE;
            } break;
            case MM_JSON_STATE_NUM_FLT: {
                if (nums[FLT].str)
                    return MM_JSON_NONE;
                if (nums[EXP].str)
                    return MM_JSON_NONE;
                write->len = (mm_json_size)(cur - write->str);
                write = &nums[FLT];
                write->str = cur + 1;
            } break;
            case MM_JSON_STATE_NUM_EXP: {
                if (nums[EXP].str)
                    return MM_JSON_NONE;
                write->len = (mm_json_size)(cur - write->str);
                write = &nums[EXP];
                write->str = cur + 1;
            } break;
            case MM_JSON_STATE_NUM_BREAK: {
                len = 1;
            } break;
            default: break;
        }
    }
    write->len = (mm_json_size)(cur - write->str);

    i = mm_json_stoi(&nums[INT]);
    f = mm_json_stof(&nums[FLT]);
    e = mm_json_stoi(&nums[EXP]);
    p = mm_json_ipow(10, (unsigned)((e < 0) ? -e : e));
    if (e < 0) p = (1 / p);
    *num = (i + ((i < 0) ? -f : f)) * p;
    return MM_JSON_NUMBER;
}

MM_JSON_API mm_json_size
mm_json_cpy(mm_json_char *dst, mm_json_size max, const struct mm_json_token* tok)
{
    unsigned i = 0;
    mm_json_size ret;
    mm_json_size siz;

    MM_JSON_ASSERT(dst);
    MM_JSON_ASSERT(tok);
    if (!dst || !max || !tok)
        return 0;

    ret = (max < (tok->len + 1)) ? max : tok->len;
    siz = (max < (tok->len + 1)) ? max-1 : tok->len;
    for (i = 0; i < siz; i++)
        dst[i] = tok->str[i];
    dst[siz] = '\0';
    return ret;
}

MM_JSON_API mm_json_int
mm_json_cmp(const struct mm_json_token* tok, const mm_json_char* str)
{
    mm_json_size i;
    MM_JSON_ASSERT(tok);
    MM_JSON_ASSERT(str);
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
MM_JSON_API struct mm_json_iter
mm_json_begin(const mm_json_char *str, mm_json_size len)
{
    struct mm_json_iter iter = MM_JSON_ITER_NULL;
    mm_json_init();
    iter.src = str;
    iter.len = len;
    return iter;
}

MM_JSON_API struct mm_json_iter
mm_json_read(struct mm_json_token *obj, const struct mm_json_iter* prev)
{
    struct mm_json_iter iter;
    mm_json_size len;
    const mm_json_char *cur;
    mm_json_int utf8_remain = 0;

    MM_JSON_ASSERT(obj);
    MM_JSON_ASSERT(prev);
    if (!prev || !obj || !prev->src || !prev->len || prev->err) {
        /* case either invalid iterator or eof  */
        struct mm_json_iter it = MM_JSON_ITER_NULL;
        *obj = MM_JSON_TOKEN_NULL;
        it.err = 1;
        return it;
    }

    iter = *prev;
    *obj = MM_JSON_TOKEN_NULL;
    iter.err = 0;
    if (!iter.go) /* begin of parsing process */
        iter.go = mm_json_go_struct;

    len = iter.len;
    for (cur = iter.src; len; cur++, len--) {
        const unsigned char *tbl = (const unsigned char*)iter.go;
        unsigned char c = (unsigned char)*cur;
        if (c == '\0') {
            iter.src = 0;
            iter.len = 0;
        }
        switch (tbl[c]) {
        case MM_JSON_STATE_FAILED: {
            iter.err = 1;
            return iter;
        } break;
        case MM_JSON_STATE_LOOP: break;
        case MM_JSON_STATE_SEP: {
            if (iter.depth == 2)
                obj->children--;
        } break;
        case MM_JSON_STATE_UP: {
            if (iter.depth > 1) {
                if (iter.depth == 2)
                    obj->children++;
                obj->sub++;
            }
            if (iter.depth++ == 1)
                obj->str = cur;
        } break;
        case MM_JSON_STATE_DOWN: {
            if (--iter.depth == 1) {
                obj->len = (mm_json_size)(cur - obj->str) + 1;
                if (iter.depth != 1 || !obj->str)
                    goto l_loop;
                goto l_yield;
            }
        } break;
        case MM_JSON_STATE_QUP: {
            iter.go = mm_json_go_string;
            if (iter.depth == 1) {
                obj->str = cur;
            } else {
                if (iter.depth == 2)
                    obj->children++;
                obj->sub++;
            }
        } break;
        case MM_JSON_STATE_QDOWN: {
            iter.go = mm_json_go_struct;
            if (iter.depth == 1) {
                obj->len = (mm_json_size)(cur - obj->str) + 1;
                if (iter.depth != 1 || !obj->str)
                    goto l_loop;
                goto l_yield;
            }
        } break;
        case MM_JSON_STATE_ESC: {
            iter.go = mm_json_go_esc;
        } break;
        case MM_JSON_STATE_UNESC: {
            iter.go = mm_json_go_string;
        } break;
        case MM_JSON_STATE_BARE: {
            if (iter.depth == 1) {
                obj->str = cur;
            } else {
                if (iter.depth == 2)
                    obj->children++;
                obj->sub++;
            }
            iter.go = mm_json_go_bare;
        } break;
        case MM_JSON_STATE_UNBARE: {
            iter.go = mm_json_go_struct;
            if (iter.depth == 1) {
                obj->len = (mm_json_size)(cur - obj->str);
                obj->type = (enum mm_json_token_type)mm_json_type(obj);
                if (obj->type == MM_JSON_STRING)
                    mm_json_deq(obj);
                iter.src = cur;
                iter.len = len;
                return iter;
            }
            cur--; len++;
        } break;
        case MM_JSON_STATE_UTF8_2: {
            iter.go = mm_json_go_utf8;
            utf8_remain = 1;
        } break;
        case MM_JSON_STATE_UTF8_3: {
            iter.go = mm_json_go_utf8;
            utf8_remain = 2;
        } break;
        case MM_JSON_STATE_UTF8_4: {
            iter.go = mm_json_go_utf8;
            utf8_remain = 3;
        } break;
        case MM_JSON_STATE_UTF8_NEXT: {
            if (!--utf8_remain)
                iter.go = mm_json_go_string;
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
            obj->len = (mm_json_size)((cur-1) - obj->str);
            obj->type = (enum mm_json_token_type)mm_json_type(obj);
            if (obj->type == MM_JSON_STRING)
                mm_json_deq(obj);
        }
        return iter;
    }
    return iter;

l_yield:
    iter.src = cur + 1;
    iter.len = len - 1;
    obj->type = mm_json_type(obj);
    if (obj->type == MM_JSON_STRING)
        mm_json_deq(obj);
    return iter;
}

MM_JSON_API struct mm_json_iter
mm_json_parse(struct mm_json_pair *p, const struct mm_json_iter* it)
{
    struct mm_json_iter next;
    MM_JSON_ASSERT(p);
    MM_JSON_ASSERT(it);
    next = mm_json_read(&p->name, it);
    if (next.err) return next;
    return mm_json_read(&p->value, &next);
}

/*--------------------------------------------------------------------------
                            PARSER
  -------------------------------------------------------------------------*/
MM_JSON_API mm_json_size
mm_json_num(const mm_json_char *json, mm_json_size length)
{
    struct mm_json_iter iter;
    struct mm_json_token tok;
    mm_json_size count = 0;

    MM_JSON_ASSERT(json);
    MM_JSON_ASSERT(length);
    if (!json || !length)
        return 0;

    iter = mm_json_begin(json, length);
    iter = mm_json_read(&tok, &iter);
    while (!iter.err) {
        count += (1 + tok.sub);
        iter = mm_json_read(&tok, &iter);
    }
    return count;
}

MM_JSON_API enum mm_json_status
mm_json_load(struct mm_json_token *toks, mm_json_size max, mm_json_size *read,
            const mm_json_char *json, mm_json_size length)
{
    enum mm_json_status status = MM_JSON_OK;
    struct mm_json_token tok;
    struct mm_json_iter iter;

    MM_JSON_ASSERT(toks);
    MM_JSON_ASSERT(json);
    MM_JSON_ASSERT(length);
    MM_JSON_ASSERT(max);
    MM_JSON_ASSERT(read);

    if (!toks || !json || !length || !max || !read)
        return MM_JSON_INVAL;
    if (*read >= max)
        return MM_JSON_OUT_OF_TOKEN;

    iter = mm_json_begin(json, length);
    iter = mm_json_read(&tok, &iter);
    if (iter.err && iter.len)
        return MM_JSON_PARSING_ERROR;

    while (iter.len) {
        toks[*read] = tok;
        *read += 1;
        if (*read > max) return MM_JSON_OUT_OF_TOKEN;
        if (toks[*read-1].type == MM_JSON_OBJECT ||  toks[*read-1].type == MM_JSON_ARRAY) {
            status = mm_json_load(toks, max, read, toks[*read-1].str, toks[*read-1].len);
            if (status != MM_JSON_OK) return status;
        }

        iter = mm_json_read(&tok, &iter);
        if (iter.err && iter.src && iter.len)
            return MM_JSON_PARSING_ERROR;
    }
    return status;
}
/*--------------------------------------------------------------------------
                            QUERY
  -------------------------------------------------------------------------*/
static const mm_json_char*
mm_json_strchr(const mm_json_char *str, mm_json_char c, mm_json_int len)
{
    mm_json_int neg = (len < 0) ? 1: 0;
    mm_json_int dec = neg ? 0 : 1;
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


static const mm_json_char*
mm_json_path_parse_name(struct mm_json_token *tok, const mm_json_char *path,
    mm_json_char delimiter)
{
    const mm_json_char *del;
    const mm_json_char *begin, *end;
    if (!path || *path == '\0')
        return NULL;

    tok->str = path;
    del = mm_json_strchr(tok->str, delimiter, -1);
    begin = mm_json_strchr(tok->str, '[', -1);
    end = mm_json_strchr(tok->str, ']', -1);

    /* array part left */
    if (begin && end && begin == tok->str) {
        tok->len = (mm_json_size)((end - begin) + 1);
        if (*(end + 1) == '\0')
            return NULL;
        if (*(end + 1) == '.')
            return(end + 2);
        else return(end + 1);
    }

    /* only array after name */
    if (begin < del) {
        tok->len = (mm_json_size)(begin - tok->str);
        return begin;
    }

    if (!del) return NULL;
    if (*del == '\0') {
        tok->len = (mm_json_size)(del - tok->str);
        return NULL;
    }
    tok->len = (mm_json_size)(del - tok->str);
    return del+1;
}

static mm_json_int
mm_json_path_parse_array(struct mm_json_token *array, const struct mm_json_token *token)
{
    const mm_json_char *begin;
    const mm_json_char *end;

    array->str = token->str;
    begin = mm_json_strchr(array->str, '[', (mm_json_int)token->len);
    if (!begin || ((mm_json_size)(begin - array->str) >= token->len))
        return 0;

    end = mm_json_strchr(begin, ']', (mm_json_int)(token->len - (mm_json_size)(begin - array->str)));
    if (!end || ((mm_json_size)(end - array->str) >= token->len))
        return 0;

    array->str = begin + 1;
    array->len = (mm_json_size)((end-1) - begin);
    return 1;
}

MM_JSON_API struct mm_json_token*
mm_json_query_del(struct mm_json_token *toks, mm_json_size count,
    const mm_json_char *path, mm_json_char delemiter)
{
    mm_json_size i = 0;
    mm_json_int begin = 1;
    struct mm_json_token *iter = NULL;
    /* iterator to step over each token in the toks array */
    struct mm_json_token name;
    /* current segment in the path to search in the tree for */
    struct mm_json_token array;
    /* array token to store the current path segment array index */
    struct mm_json_object {mm_json_size index, size;} obj;
    /* current object iterator with current pair index and total pairs in object */

    MM_JSON_ASSERT(toks);
    MM_JSON_ASSERT(count);
    if (!toks || !count) return iter;
    if (!path) return &toks[i];

    iter = &toks[i];
    array.len = 0;
    array.str = NULL;

    path = mm_json_path_parse_name(&name, path, delemiter);
    while (1) {
        if (iter->type == MM_JSON_OBJECT || iter->type == MM_JSON_ARRAY || begin) {
            /* setup iteration over elements inside a object or array */
            obj.index = 0;
            if (begin) {
                begin = 0;
                obj.size = count;
            } else if (iter->type == MM_JSON_OBJECT) {
                obj.size = iter->children;
                if ((i + 1) > count) return NULL;
                iter = &toks[++i];
            } else {
                mm_json_number n;
                mm_json_size j = 0;

                /* array object so set iterator to array index */
                if (!mm_json_path_parse_array(&array, &name))
                    return NULL;
                if ((i+1) >= count)
                    return NULL;
                if (array.len < 1)
                    return NULL;
                if (mm_json_convert(&n, &array) != MM_JSON_NUMBER)
                    return NULL;
                if ((mm_json_size)n >= iter->children)
                    return NULL;
                array.str = NULL;
                array.len = 0;

                /* iterate over each array element and find the correct index */
                iter++; i++;
                for (j = 0; j < n; ++j) {
                    if (iter->type == MM_JSON_ARRAY || iter->type == MM_JSON_OBJECT) {
                        i = i + (iter->sub) + 1;
                    } else i += 1;
                    if (i > count)
                        return NULL;
                    iter = &toks[i];
                }
                if (!path) return iter;
                path = mm_json_path_parse_name(&name, path, delemiter);
            }
            continue;
        }
        {
            /* check if current table element is equal to the current path  */
            if (!mm_json_lcmp(iter, name.str, name.len)) {
                /* correct token found and end of path */
                if (!path) {
                    if ((i + 1) > count)
                        return NULL;
                    return (iter + 1);
                }
                /* check if path points to invalid token */
                if ((i+1) > count)
                    return NULL;
                if(toks[i+1].type != MM_JSON_OBJECT && toks[i+1].type != MM_JSON_ARRAY)
                    return NULL;

                /* look deeper into child object/array */
                iter = &toks[++i];
                path = mm_json_path_parse_name(&name, path, delemiter);
            } else {
                /* key is not correct iterate until end of object */
                if (++obj.index >= obj.size)
                    return NULL;
                if ((i + 1) >= count)
                    return NULL;
                if (iter[1].type == MM_JSON_ARRAY || iter[1].type == MM_JSON_OBJECT) {
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

MM_JSON_API struct mm_json_token*
mm_json_query(struct mm_json_token *toks, mm_json_size count, const mm_json_char *path)
{return mm_json_query_del(toks, count, path, MM_JSON_DELIMITER);}

MM_JSON_API mm_json_int
mm_json_query_number_del(mm_json_number *num, struct mm_json_token *toks, mm_json_size count,
    const mm_json_char *path, mm_json_char del)
{
    struct mm_json_token *tok;
    MM_JSON_ASSERT(toks);
    MM_JSON_ASSERT(count);
    MM_JSON_ASSERT(num);
    MM_JSON_ASSERT(path);
    if (!toks || !count || !num || !path)
        return MM_JSON_NONE;

    tok = mm_json_query_del(toks, count, path, del);
    if (!tok) return MM_JSON_NONE;
    if (tok->type != MM_JSON_NUMBER)
        return tok->type;
    return mm_json_convert(num, tok);
}

MM_JSON_API mm_json_int
mm_json_query_number(mm_json_number *num, struct mm_json_token *toks, mm_json_size count,
    const mm_json_char *path)
{return mm_json_query_number_del(num, toks, count, path, MM_JSON_DELIMITER);}

MM_JSON_API mm_json_int
mm_json_query_string_del(mm_json_char *buffer, mm_json_size max, mm_json_size *size,
    struct mm_json_token *toks, mm_json_size count, const mm_json_char *path, mm_json_char del)
{
    struct mm_json_token *tok;
    MM_JSON_ASSERT(toks);
    MM_JSON_ASSERT(count);
    MM_JSON_ASSERT(buffer);
    MM_JSON_ASSERT(size);
    MM_JSON_ASSERT(path);
    if (!toks || !count || !buffer || !size || !path)
        return MM_JSON_NONE;

    tok = mm_json_query_del(toks, count, path, del);
    if (!tok) return MM_JSON_NONE;
    if (tok->type != MM_JSON_STRING)
        return tok->type;
    *size = mm_json_cpy(buffer, max, tok);
    return tok->type;
}

MM_JSON_API mm_json_int
mm_json_query_string(mm_json_char *buffer, mm_json_size max, mm_json_size *size,
    struct mm_json_token *toks, mm_json_size count, const mm_json_char *path)
{return mm_json_query_string_del(buffer, max, size, toks, count, path, MM_JSON_DELIMITER);}

MM_JSON_API mm_json_int
mm_json_query_type_del(struct mm_json_token *toks, mm_json_size count, const mm_json_char *path,
    mm_json_char del)
{
    struct mm_json_token *tok;
    MM_JSON_ASSERT(toks);
    MM_JSON_ASSERT(count);
    MM_JSON_ASSERT(path);
    if (!toks || !count || !path)
        return MM_JSON_NONE;

    tok = mm_json_query_del(toks, count, path, del);
    if (!tok) return MM_JSON_NONE;
    return tok->type;
}

MM_JSON_API mm_json_int
mm_json_query_type(struct mm_json_token *toks, mm_json_size count, const mm_json_char *path)
{return mm_json_query_type_del(toks, count, path, MM_JSON_DELIMITER);}

#endif
