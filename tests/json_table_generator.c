#include <stdio.h>

enum json_parser_states {
    JSON_STATE_FAILED,
    JSON_STATE_LOOP,
    JSON_STATE_SEP,
    JSON_STATE_UP,
    JSON_STATE_DOWN,
    JSON_STATE_QUP,
    JSON_STATE_QDOWN,
    JSON_STATE_ESC,
    JSON_STATE_UNESC,
    JSON_STATE_BARE,
    JSON_STATE_UNBARE,
    JSON_STATE_UTF8_2,
    JSON_STATE_UTF8_3,
    JSON_STATE_UTF8_4,
    JSON_STATE_UTF8_NEXT,
    JSON_STATE_MAX
};
enum json_nuber_states {
    JSON_STATE_NUM_FAILED,
    JSON_STATE_NUM_LOOP,
    JSON_STATE_NUM_FLT,
    JSON_STATE_NUM_EXP,
    JSON_STATE_NUM_BREAK,
    JSON_STATE_NUM_MAX
};

/* global parser jump tables */
static char json_go_struct[256];
static char json_go_bare[256];
static char json_go_string[256];
static char json_go_utf8[256];
static char json_go_esc[256];
static char json_go_num[256];


void fill(void)
{
    int i;
    for (i = 48; i <= 57; ++i)
        json_go_struct[i] = JSON_STATE_BARE;
    json_go_struct['\t'] = JSON_STATE_LOOP;
    json_go_struct['\r'] = JSON_STATE_LOOP;
    json_go_struct['\n'] = JSON_STATE_LOOP;
    json_go_struct[' '] = JSON_STATE_LOOP;
    json_go_struct['"'] = JSON_STATE_QUP;
    json_go_struct[':'] = JSON_STATE_SEP;
    json_go_struct['='] = JSON_STATE_SEP;
    json_go_struct[','] = JSON_STATE_LOOP;
    json_go_struct['['] = JSON_STATE_UP;
    json_go_struct[']'] = JSON_STATE_DOWN;
    json_go_struct['{'] = JSON_STATE_UP;
    json_go_struct['}'] = JSON_STATE_DOWN;
    json_go_struct['-'] = JSON_STATE_BARE;
    json_go_struct['t'] = JSON_STATE_BARE;
    json_go_struct['f'] = JSON_STATE_BARE;
    json_go_struct['n'] = JSON_STATE_BARE;

    for (i = 32; i <= 126; ++i)
        json_go_bare[i] = JSON_STATE_LOOP;
    json_go_bare['\t'] = JSON_STATE_UNBARE;
    json_go_bare['\r'] = JSON_STATE_UNBARE;
    json_go_bare['\n'] = JSON_STATE_UNBARE;
    json_go_bare[','] = JSON_STATE_UNBARE;
    json_go_bare[']'] = JSON_STATE_UNBARE;
    json_go_bare['}'] = JSON_STATE_UNBARE;
    json_go_bare[' '] = JSON_STATE_UNBARE;

    for (i = 32; i <= 126; ++i)
        json_go_string[i] = JSON_STATE_LOOP;
    for (i = 192; i <= 223; ++i)
        json_go_string[i] = JSON_STATE_UTF8_2;
    for (i = 224; i <= 239; ++i)
        json_go_string[i] = JSON_STATE_UTF8_3;
    for (i = 240; i <= 247; ++i)
        json_go_string[i] = JSON_STATE_UTF8_4;
    json_go_string['\\'] = JSON_STATE_ESC;
    json_go_string['"'] = JSON_STATE_QDOWN;
    for (i = 128; i <= 191; ++i)
        json_go_utf8[i] = JSON_STATE_UTF8_NEXT;

    json_go_esc['"'] = JSON_STATE_UNESC;
    json_go_esc['\\'] = JSON_STATE_UNESC;
    json_go_esc['/'] = JSON_STATE_UNESC;
    json_go_esc['b'] = JSON_STATE_UNESC;
    json_go_esc['f'] = JSON_STATE_UNESC;
    json_go_esc['n'] = JSON_STATE_UNESC;
    json_go_esc['r'] = JSON_STATE_UNESC;
    json_go_esc['t'] = JSON_STATE_UNESC;
    json_go_esc['u'] = JSON_STATE_UNESC;

    for (i = 48; i <= 57; ++i)
        json_go_num[i] = JSON_STATE_LOOP;
    json_go_num['-'] = JSON_STATE_NUM_LOOP;
    json_go_num['+'] = JSON_STATE_NUM_LOOP;
    json_go_num['.'] = JSON_STATE_NUM_FLT;
    json_go_num['e'] = JSON_STATE_NUM_EXP;
    json_go_num['E'] = JSON_STATE_NUM_EXP;
    json_go_num[' '] = JSON_STATE_NUM_BREAK;
    json_go_num['\n'] = JSON_STATE_NUM_BREAK;
    json_go_num['\t'] = JSON_STATE_NUM_BREAK;
    json_go_num['\r'] = JSON_STATE_NUM_BREAK;
}
int main(int argc, char *argv[])
{
    int i = 0;
    static const char *states[] = {
        "JSON_STATE_FAILED",
        "JSON_STATE_LOOP",
        "JSON_STATE_SEP",
        "JSON_STATE_UP",
        "JSON_STATE_DOWN",
        "JSON_STATE_QUP",
        "JSON_STATE_QDOWN",
        "JSON_STATE_ESC",
        "JSON_STATE_UNESC",
        "JSON_STATE_BARE",
        "JSON_STATE_UNBARE",
        "JSON_STATE_UTF8_2",
        "JSON_STATE_UTF8_3",
        "JSON_STATE_UTF8_4",
        "JSON_STATE_UTF8_NEXT",
    };
    static const char *num[] = {
        "JSON_STATE_NUM_FAILED",
        "JSON_STATE_NUM_LOOP",
        "JSON_STATE_NUM_FLT",
        "JSON_STATE_NUM_EXP",
        "JSON_STATE_NUM_BREAK",
    };
    fill();
    printf("static char json_go_struct[256] = {");
    for (i = 0; i < 256; ++i) {
        if (!(i & 3)) printf("\n");
        printf("\t%s,", states[json_go_struct[i]]);
    } printf("};\n");
    printf("static char json_go_bare[256] = {");
    for (i = 0; i < 256; ++i) {
        if (!(i & 3)) printf("\n");
        printf("\t%s,", states[json_go_bare[i]]);
    } printf("};\n");
    printf("static char json_go_string[256] = {");
    for (i = 0; i < 256; ++i) {
        if (!(i & 3)) printf("\n");
        printf("\t%s,", states[json_go_string[i]]);
    } printf("};\n");
    printf("static char json_go_esc[256] = {");
    for (i = 0; i < 256; ++i) {
        if (!(i & 3)) printf("\n");
        printf("\t%s,", states[json_go_esc[i]]);
    } printf("};\n");
    printf("static char json_go_utf8[256] = {");
    for (i = 0; i < 256; ++i) {
        if (!(i & 3)) printf("\n");
        printf("\t%s,", states[json_go_utf8[i]]);
    } printf("};\n");
    printf("static char json_go_num[256] = {");
    for (i = 0; i < 256; ++i) {
        if (!(i & 3)) printf("\n");
        printf("\t%s,", num[json_go_num[i]]);
    } printf("};\n");
    return 0;
}
