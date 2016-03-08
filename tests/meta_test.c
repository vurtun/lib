#define META_IMPLEMENTATION
#include "../mm_meta.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static void
test_log(void *pArg, enum meta_log_level type, int line, const char *fmt, ...)
{
    char buffer[1024];
    va_list arglist;
    (void)pArg;
    va_start(arglist, fmt);
    printf("%s(%d):  ", (type == META_WARNING) ? "Warning" : "Error", line);
    vprintf(fmt, arglist);
    va_end(arglist);
}

int main(void)
{
    struct meta_info meta;
    meta_init(&meta, test_log, 0);
    meta_load(&meta, "meta_test_file.c");
    meta_generate("meta.h", &meta);
    meta_free(&meta);
    return 0;
}

