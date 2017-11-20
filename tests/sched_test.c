/*
    Copyright (c) 2015 Doug Binks, Micha Mettke

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
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) ((void)x)

#define SCHED_STATIC
#define SCHED_IMPLEMENTATION
#define SCHED_USE_FIXED_TYPES
#define SCHED_USE_ASSERT
#include "../sched.h"

/* ---------------------------------------------------------------
 *                          PARALLEL SUM TASK
 * ---------------------------------------------------------------*/
struct count {
    uint64_t cnt;
    char cacheline[64];
};
struct parallel_sum_task {
    struct sched_task task;
    struct scheduler *sched;
    struct count *partsums;
    sched_uint size, cnt;
};
static void
parallel_sum_task_init(struct parallel_sum_task *pst, struct scheduler *sched, sched_uint size)
{
    memset(pst, 0, sizeof(*pst));
    pst->size = size;
    pst->sched = sched;
    pst->cnt = sched->threads_num;
    pst->partsums = calloc(pst->cnt, sizeof(struct count));
    memset(pst->partsums, 0, sizeof(struct count) * pst->cnt);
}
static void
parallel_sum_task_destroy(struct parallel_sum_task *pst)
{
    free(pst->partsums);
}
static void
parallel_sum_task_run(void *p, struct scheduler *s,
    struct sched_task_partition range, sched_uint thread_num)
{
    uint64_t i = 0;
    struct parallel_sum_task *t = (struct parallel_sum_task*)p;
    uint64_t sum = t->partsums[thread_num].cnt;
    for (i = range.start; i < range.end; ++i)
        sum += i + 1;
    t->partsums[thread_num].cnt = sum;
}
/* ---------------------------------------------------------------
 *                      PARALLEL REDUCTION TASK
 * ---------------------------------------------------------------*/
struct parallel_sum_reduction_task {
    struct sched_task task;
    struct parallel_sum_task pst;
    uint64_t sum;
};
static void
parallel_sum_reduction_task_init(struct parallel_sum_reduction_task *prt,
    struct scheduler *sched, sched_uint size)
{
    memset(prt, 0, sizeof(*prt));
    parallel_sum_task_init(&prt->pst, sched, size);
    prt->sum = 0;
}
static void
parallel_sum_reduction_task_destroy(struct parallel_sum_reduction_task *ptr)
{
    parallel_sum_task_destroy(&ptr->pst);
}
static void
parallel_sum_reduction_task_run(void *p, struct scheduler *s,
    struct sched_task_partition range, sched_uint thread_num)
{
    sched_uint i;
    struct parallel_sum_reduction_task *t = (struct parallel_sum_reduction_task*)p;
    scheduler_add(s, &t->pst.task, parallel_sum_task_run, &t->pst, t->pst.size, 0);
    scheduler_join(s, &t->pst.task);
    for (i = 0; i < t->pst.cnt; ++i)
        t->sum += t->pst.partsums[i].cnt;
}

/* ---------------------------------------------------------------
 *                              TEST
 * ---------------------------------------------------------------*/
#define WARMUP 10
#define RUNS 10
#define REPEATS (WARMUP+RUNS)

int main(void)
{
    sched_uint i, nthrds = sched_num_hw_threads();
    for (i = 1; i <= nthrds; ++i) {
        void *memory = 0;
        size_t needed_memory = 0;

        struct scheduler ts;
        scheduler_init(&ts, &needed_memory, SCHED_DEFAULT, 0);
        memory = calloc(needed_memory, 1);
        scheduler_start(&ts, memory);
        {
            int run = 0;
            for (run = 0; run < REPEATS; ++run) {
                struct parallel_sum_reduction_task psrt;
                fprintf(stderr, "Run: %d ...\n", run);
                parallel_sum_reduction_task_init(&psrt, &ts, 10*1024*1024);
                scheduler_add(&ts, &psrt.task, parallel_sum_reduction_task_run, &psrt, 0, 0);
                scheduler_join(&ts, &psrt.task);

                {uint64_t n, sum = 0;
                for (n = 0; n < psrt.pst.size; ++n) {
                    sum += n + 1;
                }
                if (sum != psrt.sum) {
                    fprintf(stderr, "ERROR; sum does not match!\n");
                    return -1;
                }
                fprintf(stderr, "\tSuccess: %lu\n", sum);}
                parallel_sum_reduction_task_destroy(&psrt);
            }
        }
        scheduler_stop(&ts, 1);
        free(memory);
    } return 0;
}
