#include <assert.h>

#define SINFL_IMPLEMENTATION
#define SDEFL_IMPLEMENTATION

#include "../sdefl.h"
#include "../sinfl.h"

/* ===============================================================
 *                              Test
 * ===============================================================*/
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <sys/time.h>

static double
profiler_time(struct timespec start, struct timespec stop)
{
  double accum = (double)(stop.tv_sec - start.tv_sec) * 1000.0f;
  accum += (double)(stop.tv_nsec - start.tv_nsec) / 1000000.0;
  return accum;
}
#if 0
static void
profiler_begin(void)
{
  clock_gettime(CLOCK_REALTIME, &start);
}
static void
profiler_end(void)
{
  clock_gettime(CLOCK_REALTIME, &stop);
}
#endif

static struct sdefl sdefl;

int main(int argc, char **argv)
{
    void *comp = 0;
    size_t size = 0;
    struct timespec compr_started;
    struct timespec compr_ended;
    struct timespec decompr_started;
    struct timespec decompr_ended;
    double compr_mbs = 0;
    double decompr_mbs = 0;
    void *decomp;
    double ratio = 0.0f;
    int len = 0, n, lvl;

    void *data = 0;
    {
        FILE* fp = fopen(argv[1], "rb");
        if (!fp) exit(2);
        fseek(fp, 0, SEEK_END);
        size = (size_t)ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = calloc(size, 1);
        fread(data, 1, (size_t)size, fp);
        fclose(fp);
    }

    comp = calloc(size*2, 1);
    decomp = calloc(size, 1);
    for (lvl = SDEFL_LVL_MIN; lvl < SDEFL_LVL_MAX; ++lvl) {
        clock_gettime(CLOCK_REALTIME, &compr_started);
        len = sdeflate(&sdefl, comp, data, (int)size, lvl);
        clock_gettime(CLOCK_REALTIME, &compr_ended);

        clock_gettime(CLOCK_REALTIME, &decompr_started);
        n = sinflate(decomp, comp, len);
        clock_gettime(CLOCK_REALTIME, &decompr_ended);

        double compr_time = profiler_time(compr_started, compr_ended);
        double decompr_time = profiler_time(decompr_started, decompr_ended);

        ratio = (double)len / (double)size * 100.0;
        compr_mbs = ((double)size / 1000000.0f) / (compr_time / 1000.0f);
        decompr_mbs = ((double)size / 1000000.0f) / (decompr_time / 1000.0f);

        printf("lvl: %d size: %d, compr: %.2fms (%.2fMB/s) decompr: %.2fms (%.2fMB/s) ratio: %f%% ", lvl, len, compr_time, compr_mbs, decompr_time, decompr_mbs, ratio);
        if ((size_t)n != size || memcmp(data, decomp, (size_t)size)) {
            FILE *fd = fopen("error.bin", "wb");
            fwrite(decomp, 1, (size_t)n, fd);
            fclose(fd);

            printf("error: %d:%d !\n", n, (int)size);
            break;
        }
        else printf("\n");
    }
    return 0;
}

