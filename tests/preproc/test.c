#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "meta.h"

introspect struct rectangle {
    float x, y, w, h;
};

introspect struct test {
    struct rectangle rect;
    char c;
    short s;
    int i;
    long l;
    int *pi;
    long al[16];
};

introspect enum state {
    STATE_WAITING,
    STATE_RUNNING,
    STATE_HALTED
};

introspect enum string_enum {
    PUNCT_STAR as meta("x"),
    PUNCT_COMMA as meta(","),
    PUNCT_PLUS as meta("+")
};

introspect int
function(int i, char c, long l)
{
    return i + c + l;
}

#define META_IMPLEMENTATION
#include "meta.h"

int main(void)
{

    return 0;
}

