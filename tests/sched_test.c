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
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) ((void)x)

#define MM_SCHED_STATIC
#define MM_SCHED_IMPLEMENTATION
#define MM_SCHED_USE_FIXED_TYPES
#define MM_SCHED_USE_ASSERT
#include "../mm_sched.h"

struct parallel_sum_args {
    unsigned long *partial_sums;
    unsigned long parital_sum_num;
};

static struct parallel_sum_args
parallel_sum_arg(unsigned long partial_sums)
{
    struct parallel_sum_args args;
    args.parital_sum_num = partial_sums;
    args.partial_sums = (unsigned long*)calloc(sizeof(unsigned long), args.parital_sum_num);
    assert(args.partial_sums);
    return args;
}

static void
parallel_sum(void *pArgs, struct mm_scheduler *ts, mm_sched_uint start, mm_sched_uint end, mm_sched_uint thread_num)
{
    unsigned long sum = 0, i = 0;
    struct parallel_sum_args args;
    UNUSED(ts);
    args = *(struct parallel_sum_args*)pArgs;
    sum = args.partial_sums[thread_num];
    for (i = start; i < end; ++i)
        sum += i + 1;
    args.partial_sums[thread_num] = sum;
}

static void
parallel_reduction_sum(void *pArgs, struct mm_scheduler *ts, mm_sched_uint start, mm_sched_uint end, mm_sched_uint thread_num)
{
    unsigned long sum = 0, in_max_sum, i = 0;
    struct parallel_sum_args args = parallel_sum_arg(ts->partitions_num);
    UNUSED(start); UNUSED(end); UNUSED(thread_num);
    in_max_sum = *(unsigned long*)pArgs;
    {
        struct mm_sched_task task;
        mm_scheduler_add(&task, ts, parallel_sum, &args, (unsigned int)in_max_sum);
        mm_scheduler_join(ts, &task);
    }
    for (i = 0; i < args.parital_sum_num; ++i)
        sum += args.partial_sums[i];
    free(args.partial_sums);
    *(unsigned long*)pArgs = sum;
}

int
main(void)
{
    void *memory;
    size_t needed_memory;

    struct mm_scheduler ts;
    mm_scheduler_init(&ts, &needed_memory, MM_SCHED_DEFAULT, NULL);
    memory = calloc(needed_memory, 1);
    assert(memory);
    mm_scheduler_start(&ts, memory);
    {
        unsigned long serial_sum = 0;
        unsigned long max = 10 * 1024 * 1024;
        unsigned long in_max_sum = max;

        struct mm_sched_task task;
        mm_scheduler_add(&task, &ts, parallel_reduction_sum, &in_max_sum, 1);
        mm_scheduler_join(&ts, &task);

        fprintf(stdout, "Parallel complete sum:\t%lu\n", in_max_sum);
        {
            unsigned long i = 0;
            for (i = 0; i < max; ++i)
                serial_sum += i + 1;
        }
        fprintf(stdout, "Serial Example complete sum:\t%lu\n", serial_sum);
    }
    mm_scheduler_stop(&ts);
    free(memory);
    return 0;
}

