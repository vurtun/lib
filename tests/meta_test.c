#define MM_META_IMPLEMENTATION
#include "../mm_meta.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static void
test_log(void *pArg, enum mm_meta_log_level type, int line, const char *fmt, ...)
{
    char buffer[1024];
    va_list arglist;
    (void)pArg;
    va_start(arglist, fmt);
    printf("%s(%d):  ", (type == MM_META_WARNING) ? "Warning" : "Error", line);
    vprintf(fmt, arglist);
    va_end(arglist);
}

int main(void)
{
    struct mm_meta_info meta;
    mm_meta_init(&meta, test_log, 0);
    mm_meta_load(&meta, "meta_test_file.c");
    mm_meta_generate("meta.h", &meta);
    mm_meta_free(&meta);
    return 0;
}

