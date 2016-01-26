/*
    mm_sched.h - zlib - Doug Binks, Micha Mettke

ABOUT:
    This is a permissively licensed ANSI C Task Scheduler for
    creating parallel programs. Note - this is a pure ANSI C single header
    conversion of Doug Binks enkiTS library (https://github.com/dougbinks/enkiTS).

    Project Goals
    - ANSI C: Designed to be easy to embed into other languages
    - Embeddable: Designed as a single header library to be easy to embed into your code. 
    - Lightweight: Designed to be lean so you can use it anywhere easily, and understand it.
    - Fast, then scalable: Designed for consumer devices first, so performance on a low number of threads is important, followed by scalability.
    - Braided parallelism: Can issue tasks from another task as well as from the thread which created the Task System.
    - Up-front Allocation friendly: Designed for zero allocations during scheduling.

DEFINE:
    MM_SCHED_IMPLEMENTATION
        Generates the implementation of the library into the included file.
        If not provided the library is in header only mode and can be included
        in other headers or source files without problems. But only ONE file
        should hold the implementation.

    MM_SCHED_STATIC
        The generated implementation will stay private inside the implementation
        file and all internal symbols and functions will only be visible inside
        that file.

    MM_SCHED_ASSERT
    MM_SCHED_USE_ASSERT
        If you define MM_SCHED_USE_ASSERT without defining MM_ASSERT mm_sched.h
        will use assert.h and assert(). Otherwise it will use your assert
        method. If you do not define MM_SCHED_USE_ASSERT no additional checks
        will be added. This is the only C standard library function used
        by mm_sched.

    MM_SCHED_MEMSET
        You can define this to 'memset' or your own memset replacement.
        If not, mm_sched.h uses a naive (maybe inefficent) implementation.

    MM_SCHED_INT32
    MM_SCHED_UINT32
    MM_SCHED_UINT_PTR
        If your compiler is C99 you do not need to define this.
        Otherwise, mm_sched will try default assignments for them
        and validate them at compile time. If they are incorrect, you will
        get compile errors and will need to define them yourself.

    MM_SCHED_SPIN_COUNT_MAX
        You can change this to set the maximum number of spins for worker
        threads to stop looking for work and go into a sleeping state.

    MM_SCHED_PIPE_SIZE_LOG2
        You can change this to set the size of each worker thread pipe.
        The value is in power of two and needs to smaller than 32 otherwise
        the atomic integer type will overflow.


LICENSE: (zlib)
    Copyright (c) 2016 Doug Binks, Micha Mettke

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

CONTRIBUTORS:
    Doug Binks (implementation)
    Micha Mettke (single header ANSI C conversion)

EXAMPLES:*/
#if 0
    static void parallel_task(void *pArg, struct mm_scheduler *s, mm_sched_uint begin, mm_sched_uint end, mm_sched_uint thread) {
        /* Do something here, cann issue additional tasks into the scheduler */
    }

    int main(int argc, const char **argv)
    {
        void *memory;
        mm_sched_size needed_memory;

        struct mm_scheduler sched;
        mm_scheduler_init(&sched, &needed_memory, MM_SCHED_DEFAULT, 0);
        memory = calloc(needed_memory, 1);
        mm_scheduler_start(&sched, memory);
        {
            struct mm_sched_task task;
            mm_scheduler_add(&sched, &task, parallel_task, 0, 1);
            mm_scheduler_join(&sched, &task);
        }
        mm_scheduler_stop(&sched);
        free(memory);
    }
#endif
 /* ===============================================================
 *
 *                          HEADER
 *
 * =============================================================== */
#ifndef MM_SCHED_H_
#define MM_SCHED_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MM_SCHED_STATIC
#define MM_SCHED_API static
#else
#define MM_SCHED_API extern
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 19901L)
#include <stdint.h>
#ifndef MM_SCHED_UINT32
#define MM_SCHED_UINT32 uint32_t
#endif
#ifndef MM_SCHED_INT32
#define MM_SCHED_INT32 int32_t
#endif
#ifndef MM_SCHED_UINT_PTR
#define MM_SCHED_UINT_PTR uintptr_t
#endif
#else
#ifndef MM_SCHED_UINT32
#define MM_SCHED_UINT32 unsigned int
#endif
#ifndef MM_SCHED_INT32
#define MM_SCHED_INT32 int
#endif
#ifndef MM_SCHED_UINT_PTR
#define MM_SCHED_UINT_PTR unsigned long
#endif
#endif

typedef unsigned char mm_sched_byte;
typedef MM_SCHED_UINT32 mm_sched_uint;
typedef MM_SCHED_INT32 mm_sched_int;
typedef MM_SCHED_UINT_PTR mm_sched_size;
typedef MM_SCHED_UINT_PTR mm_sched_ptr;

struct mm_scheduler;
typedef void(*mm_sched_run)(void*, struct mm_scheduler*, unsigned int begin,
    unsigned int end, unsigned int thread_num);

struct mm_sched_task {
    void *userdata;
    /* custum userdata to use in callback userdata */
    mm_sched_run exec;
    /* function working on the task owner structure */
    mm_sched_uint size;
    /* number of elements inside the set */
    volatile mm_sched_int run_count;
    /* INTERNAL ONLY */
};
#define mm_sched_task_done(t) (!(t)->run_count)

typedef void (*mm_sched_profiler_callback_f)(void*, mm_sched_uint thread_id);
struct mm_sched_profiling {
    void *userdata;
    /* from the user provided data used in each callback */
    mm_sched_profiler_callback_f thread_start;
    /* callback called as soon as a thread starts working */
    mm_sched_profiler_callback_f thread_stop;
    /* callback called when as a thread is finished */
    mm_sched_profiler_callback_f wait_start;
    /* callback called if a thread begins waiting */
    mm_sched_profiler_callback_f wait_stop;
    /* callback called if a thread is woken up */
};

struct mm_sched_event;
struct mm_sched_thread_args;
struct mm_sched_pipe;

struct mm_scheduler {
    struct mm_sched_pipe *pipes;
    /* pipe for every worker thread */
    unsigned int threads_num;
    /* number of worker threads */
    struct mm_sched_thread_args *args;
    /* data used in the os thread callback */
    void *threads;
    /* os threads array  */
    volatile mm_sched_int running;
    /* flag whether the scheduler is running  */
    volatile mm_sched_int thread_running;
    /* number of thread that are currently running */
    volatile mm_sched_int thread_active;
    /* number of thread that are currently active */
    unsigned partitions_num;
    /* divider for the array handled by a task */
    struct mm_sched_event *event;
    /* os event to signal work */
    mm_sched_int have_threads;
    /* flag whether the os threads have been created */
    struct mm_sched_profiling profiling;
    /* profiling callbacks  */
    mm_sched_size memory;
    /* memory size */
};

#define MM_SCHED_DEFAULT (-1)
MM_SCHED_API void mm_scheduler_init(struct mm_scheduler*, mm_sched_size *needed_memory,
                                mm_sched_int thread_count, const struct mm_sched_profiling*);
/*  this function clears the scheduler and calculates the needed memory to run
    Input:
    -   number of os threads to create inside the scheduler (or MM_SCHED_DEFAULT for number of cpu cores)
    -   optional profiling callbacks for profiler (NULL if not wanted)
    Output:
    -   needed memory for the scheduler to run
*/
MM_SCHED_API void mm_scheduler_start(struct mm_scheduler*, void *memory);
/*  this function starts running the scheduler and creates the previously set
 *  number of threads-1, which is sufficent to fill the system by
 *  including the main thread. Start can be called multiple times - it will wait
 *  for the completion before re-initializing.
    Input:
    -   previously allocated memory to run the scheduler with
*/
MM_SCHED_API void mm_scheduler_add(struct mm_sched_task*, struct mm_scheduler*, mm_sched_run func, void *pArg, mm_sched_uint size);
/*  this function adds a task into the scheduler to execute and directly returns
 *  if the pipe is not full. Otherwise the task is run directly. Should only be
 *  called from main thread or within task handler.
    Input:
    -   function to execute to process the task
    -   userdata to call the execution function with
    -   array size that will be divided over multible threads
    Output:
    -   task handle used to wait for the task to finish or check if done. Needs
        to be persistent over the process of the task
*/
MM_SCHED_API void mm_scheduler_join(struct mm_scheduler*, struct mm_sched_task*);
/*  this function waits for a previously started task to finish. Should only be
 *  called from thread which created the task scheduler, or within a task
 *  handler. if called with NULL it will try to run task and return if none
 *  available.
    Input:
    -   previously started task to wait until it is finished
*/
MM_SCHED_API void mm_scheduler_wait(struct mm_scheduler*);
/*  this function waits for all task inside the scheduler to finish. Not
 *  guaranteed to work unless we know we are in a situation where task aren't
 *  being continuosly added. */
MM_SCHED_API void mm_scheduler_stop(struct mm_scheduler*);
/*  this function waits for all task inside the scheduler to finish and stops
 *  all threads and shuts the scheduler down. Not guaranteed to work unless we
 *  are in a situation where task aren't being continuosly added. */
#ifdef __cplusplus
}
#endif
#endif /* MM_SCHED_H_ */

/* ===============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================*/
#ifdef MM_SCHED_IMPLEMENTATION

/* windows requires Windows.h even if you use mingw */
#if defined(_WIN32) || (defined(__MINGW32__) || defined(__MINGW64__))
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

/* make sure atomic and pointer types have correct size */
typedef int mm_sched__check_ptr_size[(sizeof(void*) == sizeof(MM_SCHED_UINT_PTR)) ? 1 : -1];
typedef int mm_sched__check_ptr_uint32[(sizeof(mm_sched_uint) == 4) ? 1 : -1];
typedef int mm_sched__check_ptr_int32[(sizeof(mm_sched_int) == 4) ? 1 : -1];

#ifdef MM_SCHED_USE_ASSERT
#ifndef MM_SCHED_ASSERT
#include <assert.h>
#define MM_SCHED_ASSERT(expr) assert(expr)
#endif
#else
#define MM_SCHED_ASSERT(expr)
#endif

#define MM_SCHED_INTERN static
#define MM_SCHED_GLOBAL static
#define MM_SCHED_STORAGE static

#ifdef __cplusplus
/* C++ hates the C align of makro form so have to resort to templates */
template<typename T> struct mm_sched_alignof;
template<typename T, int size_diff> struct mm_sched_helper{enum {value = size_diff};};
template<typename T> struct mm_sched_helper<T,0>{enum {value = mm_sched_alignof<T>::value};};
template<typename T> struct mm_sched_alignof{struct Big {T x; char c;}; enum {
    diff = sizeof(Big) - sizeof(T), value = mm_sched_helper<Big, diff>::value};};
#define MM_SCHED_ALIGNOF(t) (mm_sched_alignof<t>::value);
#else
#define MM_SCHED_ALIGNOF(t) ((char*)(&((struct {char c; t _h;}*)0)->_h) - (char*)0)
#endif

/* Pointer to Integer type conversion for pointer alignment */
#if defined(__PTRDIFF_TYPE__) /* This case should work for GCC*/
# define MM_SCHED_UINT_TO_PTR(x) ((void*)(__PTRDIFF_TYPE__)(x))
# define MM_SCHED_PTR_TO_UINT(x) ((mm_sched_size)(__PTRDIFF_TYPE__)(x))
#elif !defined(__GNUC__) /* works for compilers other than LLVM */
# define MM_SCHED_UINT_TO_PTR(x) ((void*)&((char*)0)[x])
# define MM_SCHED_PTR_TO_UINT(x) ((mm_sched_size)(((char*)x)-(char*)0))
#elif defined(MM_SCHED_USE_FIXED_TYPES) /* used if we have <stdint.h> */
# define MM_SCHED_UINT_TO_PTR(x) ((void*)(uintptr_t)(x))
# define MM_SCHED_PTR_TO_UINT(x) ((uintptr_t)(x))
#else /* generates warning but works */
# define MM_SCHED_UINT_TO_PTR(x) ((void*)(x))
# define MM_SCHED_PTR_TO_UINT(x) ((mm_sched_size)(x))
#endif

/* Pointer math*/
#define MM_SCHED_PTR_ADD(t, p, i) ((t*)((void*)((mm_sched_byte*)(p) + (i))))
#define MM_SCHED_ALIGN_PTR(x, mask)\
    (MM_SCHED_UINT_TO_PTR((MM_SCHED_PTR_TO_UINT((mm_sched_byte*)(x) + (mask-1)) & ~(mask-1))))

/* Helper */
#define MM_SCHED_UNUSED(x) ((void)x)
#define MM_SCHED_MIN(a,b) (((a)<(b))?(a):(b))
#define MM_SCHED_MAX(a,b) (((a)>(b))?(a):(b))

#ifndef MM_SCHED_MEMSET
#define MM_SCHED_MEMSET mm_sched_memset
#endif

MM_SCHED_INTERN void
mm_sched_memset(void *ptr, mm_sched_int c0, mm_sched_size size)
{
    #define word unsigned
    #define wsize sizeof(word)
    #define wmask (wsize - 1)
    unsigned char *dst = (unsigned char*)ptr;
    unsigned c = 0;
    mm_sched_size t = 0;

    if ((c = (unsigned char)c0) != 0) {
        c = (c << 8) | c; /* at least 16-bits  */
        if (sizeof(unsigned int) > 2)
            c = (c << 16) | c; /* at least 32-bits*/
        if (sizeof(unsigned int) > 4)
            c = (c << 32) | c; /* at least 64-bits*/
    }

    /* to small of a word count */
    dst = (unsigned char*)ptr;
    if (size < 3 * wsize) {
        while (size--) *dst++ = (unsigned char)c0;
        return;
    }

    /* align destination */
    if ((t = MM_SCHED_PTR_TO_UINT(dst) & wmask) != 0) {
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

#define mm_sched_zero_struct(s) mm_sched_zero_size(&s, sizeof(s))
#define mm_sched_zero_array(p,n) mm_sched_zero_size(p, (n) * sizeof((p)[0]))
MM_SCHED_INTERN void
mm_sched_zero_size(void *ptr, mm_sched_size size)
{
    MM_SCHED_MEMSET(ptr, 0, size);
}

/* ---------------------------------------------------------------
 *                          ATOMIC
 * ---------------------------------------------------------------*/
#if  defined(_WIN32) && !(defined(__MINGW32__) || defined(__MINGW64__))
    #include <intrin.h>
    void _ReadWriteBarrier();
    #pragma intrinsic(_ReadWriteBarrier);
    #pragma intrinsic(_InterlockedCompareExchange);
    #pragma intrinsic(_InterlockedExchangeAdd);
    #define MM_SCHED_BASE_MEMORY_BARRIER_ACQUIRE() _ReadWriteBarrier()
    #define MM_SCHED_BASE_MEMORY_BARRIER_RELEASE() _ReadWriteBarrier()
    #define MM_SCHED_BASE_ALIGN(x) __declspec(align(x))
#else
    #define MM_SCHED_BASE_MEMORY_BARRIER_ACQUIRE() __asm__ __volatile__("": : :"memory")
    #define MM_SCHED_BASE_MEMORY_BARRIER_RELEASE() __asm__ __volatile__("": : :"memory")
    #define MM_SCHED_BASE_ALIGN(x) __attribute__((aligned(x)))
#endif

MM_SCHED_INTERN mm_sched_uint
mm_sched_atomic_cmp_swp(volatile mm_sched_uint *dst, mm_sched_uint swap, mm_sched_uint cmp)
{
/* Atomically performs: if (*dst == swapTp){ *dst = swapTo;}
 * return old *dst (so if sucessfull return cmp) */
#if defined(_WIN32) && !(defined(__MINGW32__) || defined(__MINGW64__))
    /* assumes two's complement - unsigned /signed conversion leads to same bit pattern */
    return _InterlockedCompareExchange((volatile long*)dst, swap, cmp);
#else
    return __sync_val_compare_and_swap(dst, cmp, swap);
#endif
}

MM_SCHED_INTERN mm_sched_int
mm_sched_atomic_add(volatile mm_sched_int *dst, mm_sched_int value)
{
/* Atomically performs: tmp = *dst: *dst += value; return tmp; */
#if defined(_WIN32) && !(defined(__MINGW32__) || defined(__MINGW64__))
    return _InterlockedExchangeAdd((long*)dst, value);
#else
    return (mm_sched_int)__sync_add_and_fetch(dst, value);
#endif
}

/* ---------------------------------------------------------------
 *                          THREAD
 * ---------------------------------------------------------------*/
#if defined(_WIN32) && !(defined(__MINGW32__) || defined(__MINGW64__))
#define MM_SCHED_THREAD_FUNC_DECL DWORD WINAPI
#define MM_SCHED_THREAD_LOCAL __declspec(thread)

typedef HANDLE mm_sched_thread;
struct mm_sched_event {
    HANDLE event;
    mm_sched_int count_waiters;
};
const mm_sched_uint MM_SCHED_INFINITE = INFINITE;

MM_SCHED_INTERN mm_sched_int
mm_sched_thread_create(mm_sched_thread *returnid, DWORD(WINAPI *StartFunc)(void*), void *arg)
{
    DWORD thread;
    *returnid = CreateThread(0,0, StartFunc, arg, 0, &thread);
    return *returnid != NULL;
}

MM_SCHED_INTERN mm_sched_int
mm_sched_thread_term(mm_sched_thread threadid)
{
    return CloseHandle(threadid) == 0;
}

MM_SCHED_INTERN mm_sched_uint
mm_sched_num_hw_threads(void)
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

MM_SCHED_INTERN struct mm_sched_event
mm_sched_event_create(void)
{
    mm_sched_event ret;
    ret.event = CreateEvent(NULL, TRUE, FALSE, NULL);
    ret.count_waiters = 0;
    return ret;
}

MM_SCHED_INTERN void
mm_sched_event_close(struct mm_sched_event *eventid)
{
    CloseHandle(eventid->event);
}

MM_SCHED_INTERN void
mm_sched_event_wait(struct mm_sched_event *eventid, mm_sched_int ms)
{
    DWORD ret_val;
    mm_sched_int prev;
    mm_sched_atomic_add(&eventid->count_waiters, 1);
    ret_val = WaitForSingleObject(eventid->event, ms);
    prev = mm_sched_atomic_add(&eventid->count_waiters, -1);
    if (prev == 1) /* we were the last to awaken, so reset event. */
        ResetEvent(eventid->event);
    MM_SCHED_ASSERT(ret_val != WAIT_FAILED);
    MM_SCHED_ASSERT(prev != 0);
}

MM_SCHED_INTERN void
mm_sched_event_signal(struct mm_sched_event *eventid)
{
    SetEvent(eventid->event);
}

#else
/* POSIX */
#include <pthread.h>
#if !(defined(__MINGW32__) || defined(__MINGW64__))
    #include <unistd.h>
    #include <time.h>
#endif

#define MM_SCHED_THREAD_FUNC_DECL void*
#define MM_SCHED_THREAD_LOCAL __thread

typedef pthread_t mm_sched_thread;
struct mm_sched_event {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
};
const mm_sched_int MM_SCHED_INFINITE = -1;

MM_SCHED_INTERN mm_sched_int
mm_sched_thread_create(mm_sched_thread *returnid, void*(*StartFunc)(void*), void *arg)
{
    mm_sched_int ret_val;
    MM_SCHED_ASSERT(returnid);
    MM_SCHED_ASSERT(StartFunc);
    ret_val = pthread_create(returnid, NULL, StartFunc, arg);
    return(ret_val == 0);
}

MM_SCHED_INTERN mm_sched_int
mm_sched_thread_term(mm_sched_thread threadid)
{
    return (pthread_cancel(threadid) == 0);
}

MM_SCHED_INTERN struct mm_sched_event
mm_sched_event_create(void)
{
    struct mm_sched_event event = {PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};
    return event;
}

MM_SCHED_INTERN void
mm_sched_event_close(struct mm_sched_event *eventid)
{
    /* do not need to close event */
    MM_SCHED_UNUSED(eventid);
}

MM_SCHED_INTERN void
mm_sched_event_wait(struct mm_sched_event *eventid, mm_sched_int ms)
{
    MM_SCHED_ASSERT(eventid);
    pthread_mutex_lock(&eventid->mutex);
    if (ms == MM_SCHED_INFINITE) {
        pthread_cond_wait(&eventid->cond, &eventid->mutex);
    } else {
        struct timespec waittime;
        waittime.tv_sec = ms/1000;
        ms -= (mm_sched_int)waittime.tv_sec*1000;
        waittime.tv_nsec = ms * 1000;
        pthread_cond_timedwait(&eventid->cond, &eventid->mutex, &waittime);
    }
    pthread_mutex_unlock(&eventid->mutex);
}

MM_SCHED_INTERN void
mm_sched_event_signal(struct mm_sched_event *eventid)
{
    MM_SCHED_ASSERT(eventid);
    pthread_mutex_lock(&eventid->mutex);
    pthread_cond_broadcast(&eventid->cond);
    pthread_mutex_unlock(&eventid->mutex);
}

MM_SCHED_INTERN mm_sched_uint
mm_sched_num_hw_threads(void)
{
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
#else
    return (mm_sched_uint)sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

#endif

/* ---------------------------------------------------------------
 *                          PIPE
 * ---------------------------------------------------------------*/
/*  PIPE
    Single writer, multiple reader thread safe pipe using (semi) lockless programming
    Readers can only read from the back of the pipe
    The single writer can write to the front of the pipe, and read from both
    ends (a writer can be a reader) for many of the principles used here,
    see http://msdn.microsoft.com/en-us/library/windows/desktop/ee418650(v=vs.85).aspx
    Note: using log2 sizes so we do not need to clamp (multi-operation)
    Note this is not true lockless as the use of flags as a form of lock state.
*/
/* IMPORTANT: Define this to control the maximum number of elements inside a
 * pipe as a log2 number. Should be smaller than 32 since it would otherwise
 * overflow the atomic integer type.*/
#ifndef MM_SCHED_PIPE_SIZE_LOG2
#define MM_SCHED_PIPE_SIZE_LOG2 8
#endif
#define MM_SCHED_PIPE_SIZE (2 << MM_SCHED_PIPE_SIZE_LOG2)
#define MM_SCHED_PIPE_MASK (MM_SCHED_PIPE_SIZE-1)

/* 32-Bit for compare-and-swap */
#define MM_SCHED_PIPE_INVALID    0xFFFFFFFF
#define MM_SCHED_PIPE_CAN_WRITE  0x00000000
#define MM_SCHED_PIPE_CAN_READ   0x11111111

struct mm_sched_task_partition {
    mm_sched_uint start;
    mm_sched_uint end;
};

struct mm_sched_subset_task {
    struct mm_sched_task *task;
    struct mm_sched_task_partition partition;
};

struct mm_sched_pipe {
    struct mm_sched_subset_task buffer[MM_SCHED_PIPE_SIZE];
    /* read and write index allow fast access to the pipe
     but actual access is controlled by the access flags. */
    volatile mm_sched_uint MM_SCHED_BASE_ALIGN(4) write;
    volatile mm_sched_uint MM_SCHED_BASE_ALIGN(4) read_count;
    volatile mm_sched_uint flags[MM_SCHED_PIPE_SIZE];
    volatile mm_sched_uint MM_SCHED_BASE_ALIGN(4) read;
};

/* utility function, not intended for general use. Should only be used very prudenlty*/
#define mm_sched_pipe_is_empty(p) (((p)->write - (p)->read_count) == 0)

MM_SCHED_INTERN mm_sched_int
mm_sched_pipe_read_back(struct mm_sched_pipe *pipe, struct mm_sched_subset_task *dst)
{
    /* return false if we are unable to read. This is thread safe for both
     * multiple readers and the writer */
    mm_sched_uint to_use;
    mm_sched_uint previous;
    mm_sched_uint actual_read;
    mm_sched_uint read_count;

    MM_SCHED_ASSERT(pipe);
    MM_SCHED_ASSERT(dst);

    read_count = pipe->read_count;
    to_use = read_count;
    while (1) {
        mm_sched_uint write_index = pipe->write;
        mm_sched_uint num_in_pipe = write_index - read_count;
        if (!num_in_pipe)
            return 0;

        /* move back to start */
        if (to_use >= write_index)
            to_use = pipe->read;

        /* power of two sizes ensures we can perform AND for a moduls */
        actual_read = to_use & MM_SCHED_PIPE_MASK;
        /* multiple potential readers means we should check if the data is valid
         * using an atomic compare exchange */
        previous = mm_sched_atomic_cmp_swp(&pipe->flags[actual_read], MM_SCHED_PIPE_INVALID, MM_SCHED_PIPE_CAN_READ);
        if (previous == MM_SCHED_PIPE_CAN_READ)
            break;

        ++to_use;
        /* update known read_count */
        read_count = pipe->read_count;
    }
    mm_sched_atomic_add((volatile mm_sched_int*)&pipe->read_count, 1);
    MM_SCHED_BASE_MEMORY_BARRIER_ACQUIRE();

    /* noew read data, ensuring we do so after above reads & CAS */
    *dst = pipe->buffer[actual_read];
    pipe->flags[actual_read] = MM_SCHED_PIPE_CAN_WRITE;
    return 1;
}

MM_SCHED_INTERN mm_sched_int
mm_sched_pipe_read_front(struct mm_sched_pipe *pipe, struct mm_sched_subset_task *dst)
{
    mm_sched_uint prev;
    mm_sched_uint actual_read = 0;
    mm_sched_uint write_index;
    mm_sched_uint front_read;

    write_index = pipe->write;
    front_read = write_index;

    /* Mutliple potential reads mean we should check if the data is valid,
     * using an atomic compare exchange - which acts as a form of lock */
    prev = MM_SCHED_PIPE_INVALID;
    actual_read = 0;
    while (1) {
        /* power of two ensures we can use a simple cal without modulus */
        mm_sched_uint read_count = pipe->read_count;
        mm_sched_uint num_in_pipe = write_index - read_count;
        if (!num_in_pipe || !front_read) {
            pipe->read = read_count;
            return 0;
        }
        --front_read;
        actual_read = front_read & MM_SCHED_PIPE_MASK;
        prev = mm_sched_atomic_cmp_swp(&pipe->flags[actual_read], MM_SCHED_PIPE_INVALID, MM_SCHED_PIPE_CAN_READ);
        if (prev == MM_SCHED_PIPE_CAN_READ) break;
        else if (pipe->read >= front_read) return 0;
    }

    *dst = pipe->buffer[actual_read];
    pipe->flags[actual_read] = MM_SCHED_PIPE_CAN_WRITE;
    MM_SCHED_BASE_MEMORY_BARRIER_RELEASE();

    /* 32-bit aligned stores are atomic, and writer owns the write index */
    --pipe->write;
    return 1;
}

MM_SCHED_INTERN mm_sched_int
mm_sched_pipe_write(struct mm_sched_pipe *pipe, const struct mm_sched_subset_task *src)
{
    /* returns false if we were to write. This is thread safe for the single
     * writer, but should not be called by readers  */
    mm_sched_uint actual_write;
    mm_sched_uint write_index;
    MM_SCHED_ASSERT(pipe);
    MM_SCHED_ASSERT(src);

    /* The writer 'owns' the write index and readers can only reduce the amout of
     * data in the pipe. We get hold of both values for consistentcy and to
     * reduce false sharing impacting more than one access */
    write_index = pipe->write;

    /* power of two sizes ensures we can perform AND for a modulus*/
    actual_write = write_index & MM_SCHED_PIPE_MASK;
    /* a read may still be reading this item, as there are multiple readers */
    if (pipe->flags[actual_write] != MM_SCHED_PIPE_CAN_WRITE)
        return 0; /* still being read, so have caught up with tail */

    /* as we are the only writer we can update the data without atomics whilst
     * the write index has not been updated. */
    pipe->buffer[actual_write] = *src;
    pipe->flags[actual_write] = MM_SCHED_PIPE_CAN_READ;

    /* we need to ensure the above occur prior to updating the write index,
     * otherwise another thread might read before it's finished */
    MM_SCHED_BASE_MEMORY_BARRIER_RELEASE();
    /* 32-bit aligned stores are atomic, and writer owns the write index */
    ++write_index;
    pipe->write = write_index;
    return 1;
}

/* ---------------------------------------------------------------
 *                          SCHEDULER
 * ---------------------------------------------------------------*/
/* IMPORTANT: Define this to control the maximum number of iterations for a
 * thread to check for work until it is send into a sleeping state */
#ifndef MM_SCHED_SPIN_COUNT_MAX
#define MM_SCHED_SPIN_COUNT_MAX 100
#endif

struct mm_sched_thread_args {
    mm_sched_uint thread_num;
    struct mm_scheduler *scheduler;
};

MM_SCHED_GLOBAL const mm_sched_size mm_sched_pipe_align = MM_SCHED_ALIGNOF(struct mm_sched_pipe);
MM_SCHED_GLOBAL const mm_sched_size mm_sched_arg_align = MM_SCHED_ALIGNOF(struct mm_sched_thread_args);
MM_SCHED_GLOBAL const mm_sched_size mm_sched_thread_align = MM_SCHED_ALIGNOF(mm_sched_thread);
MM_SCHED_GLOBAL const mm_sched_size mm_sched_event_align = MM_SCHED_ALIGNOF(struct mm_sched_event);
MM_SCHED_GLOBAL MM_SCHED_THREAD_LOCAL mm_sched_uint gtl_thread_num = 0;

MM_SCHED_INTERN mm_sched_int
mm_sched_try_running_task(struct mm_scheduler *s, mm_sched_uint thread_num, mm_sched_uint *pipe_hint)
{
    /* check for tasks */
    struct mm_sched_subset_task subtask;
    mm_sched_int have_task = mm_sched_pipe_read_front(&s->pipes[thread_num], &subtask);
    mm_sched_uint thread_to_check = *pipe_hint;
    mm_sched_uint check_count = 0;

    while (!have_task && check_count < s->threads_num) {
        thread_to_check = (*pipe_hint + check_count) % s->threads_num;
        if (thread_to_check != thread_num)
            have_task = mm_sched_pipe_read_back(&s->pipes[thread_to_check], &subtask);
        ++check_count;
    }

    if (have_task) {
        /* update hint, will preserve value unless actually got task from another thread */
        *pipe_hint = thread_to_check;
        /* the task has already been divided up by mm_scheduler_add, so just run */
        subtask.task->exec(subtask.task->userdata, s, subtask.partition.start,
                subtask.partition.end, thread_num);
        mm_sched_atomic_add(&subtask.task->run_count, -1);
    }
    return have_task;
}

MM_SCHED_INTERN void
mm_scheduler_wait_for_work(struct mm_scheduler *s, mm_sched_uint thread_num)
{
    mm_sched_uint i = 0;
    mm_sched_int have_tasks = 0;
    for (i = 0; i < s->threads_num; ++i) {
        if (!mm_sched_pipe_is_empty(&s->pipes[i])) {
            have_tasks = 1;
            break;
        }
    }
    if (!have_tasks) {
        if (s->profiling.wait_start)
            s->profiling.wait_start(s->profiling.userdata, thread_num);
        mm_sched_atomic_add(&s->thread_active, -1);
        mm_sched_event_wait(s->event, MM_SCHED_INFINITE);
        mm_sched_atomic_add(&s->thread_active, +1);
        if (s->profiling.wait_stop)
            s->profiling.wait_stop(s->profiling.userdata, thread_num);
    }
}

MM_SCHED_INTERN MM_SCHED_THREAD_FUNC_DECL
mm_sched_tasking_thread_f(void *pArgs)
{
    mm_sched_uint spin_count = 0, hint_pipe;
    struct mm_sched_thread_args args = *(struct mm_sched_thread_args*)pArgs;
    mm_sched_uint thread_num = args.thread_num;
    struct mm_scheduler *s = args.scheduler;
    gtl_thread_num = args.thread_num;

    mm_sched_atomic_add(&s->thread_active, 1);
    if (s->profiling.thread_start)
        s->profiling.thread_start(s->profiling.userdata, thread_num);

    hint_pipe = thread_num + 1;
    while (s->running) {
        if (!mm_sched_try_running_task(s, thread_num, &hint_pipe)) {
            ++spin_count;
            if (spin_count > MM_SCHED_SPIN_COUNT_MAX)
                mm_scheduler_wait_for_work(s, thread_num);
        } else spin_count = 0;
    }

    mm_sched_atomic_add(&s->thread_running, -1);
    if (s->profiling.thread_stop)
        s->profiling.thread_stop(s->profiling.userdata, thread_num);
    return 0;
}

MM_SCHED_API void
mm_scheduler_init(struct mm_scheduler *s, mm_sched_size *memory,
    mm_sched_int thread_count, const struct mm_sched_profiling *prof)
{
    MM_SCHED_ASSERT(s);
    MM_SCHED_ASSERT(memory);

    mm_sched_zero_struct(*s);
    /* ensure we have sufficent tasks to equally fill either all threads
     * including the main or just the threads we launched, this is outside the
     * first start as we awant to be able to runtime change it.*/
    s->threads_num = (thread_count == MM_SCHED_DEFAULT)?
        mm_sched_num_hw_threads() : (mm_sched_uint)thread_count;
    s->partitions_num = (s->threads_num == 1) ?
        1: (s->threads_num * (s->threads_num - 1));
    if (prof) s->profiling = *prof;

    /* calculate needed memory */
    MM_SCHED_ASSERT(s->threads_num > 0);
    *memory = 0;
    *memory += sizeof(struct mm_sched_pipe) * s->threads_num;
    *memory += sizeof(struct mm_sched_thread_args) * s->threads_num;
    *memory += sizeof(mm_sched_thread) * s->threads_num;
    *memory += sizeof(struct mm_sched_event);
    *memory += mm_sched_pipe_align + mm_sched_arg_align + mm_sched_thread_align + mm_sched_event_align;
    s->memory = *memory;
}

MM_SCHED_API void
mm_scheduler_start(struct mm_scheduler *s, void *memory)
{
    mm_sched_uint i = 0;
    MM_SCHED_ASSERT(s);
    MM_SCHED_ASSERT(memory);
    if (s->have_threads) return;
    mm_scheduler_stop(s);

    /* setup scheduler memory */
    mm_sched_zero_size(memory, s->memory);
    s->pipes = (struct mm_sched_pipe*)MM_SCHED_ALIGN_PTR(memory, mm_sched_pipe_align);
    s->threads = MM_SCHED_ALIGN_PTR(s->pipes + s->threads_num, mm_sched_thread_align);
    s->args = (struct mm_sched_thread_args*) MM_SCHED_ALIGN_PTR(
        MM_SCHED_PTR_ADD(void, s->threads, sizeof(mm_sched_thread) * s->threads_num), mm_sched_arg_align);
    s->event = (struct mm_sched_event*)MM_SCHED_ALIGN_PTR(s->args + s->threads_num, mm_sched_event_align);
    *s->event = mm_sched_event_create();

    /* Create one less thread than thread_num as the main thread count as one */
    s->args[0].thread_num = 0;
    s->args[0].scheduler = s;
#if  defined(_WIN32) && !(defined(__MINGW32__) || defined(__MINGW64__))
    ((mm_sched_thread*)(s->threads)) [0] = 0;
#endif
    s->thread_running = 1;
    s->thread_active = 1;
    s->running = 1;

    /* start hardware threads */
    for (i = 1; i < s->threads_num; ++i) {
        s->args[i].thread_num = i;
        s->args[i].scheduler = s;
        mm_sched_thread_create(&((mm_sched_thread*)(s->threads))[i], mm_sched_tasking_thread_f, &s->args[i]);
        s->thread_running++;
    }
    s->have_threads = 1;
}

MM_SCHED_API void
mm_scheduler_add(struct mm_sched_task *task, struct mm_scheduler *s, mm_sched_run func, void *pArg, mm_sched_uint size)
{
    struct mm_sched_subset_task subtask;
    mm_sched_uint range_to_run;
    mm_sched_uint range_left;
    mm_sched_uint num_added = 0;

    MM_SCHED_ASSERT(s);
    MM_SCHED_ASSERT(task);
    MM_SCHED_ASSERT(func);

    task->userdata = pArg;
    task->exec = func;
    task->size = size;

    subtask.task = task;
    subtask.partition.start = 0;
    subtask.partition.end = task->size;
    task->run_count = -1;

    /* divide task up and add to pipe */
    range_to_run = MM_SCHED_MAX(1, task->size / s->partitions_num);
    range_left = subtask.partition.end - subtask.partition.start;
    num_added = 0;
    while (range_left) {
        if (range_to_run > range_left)
            range_to_run = range_left;

        subtask.partition.start = task->size - range_left;
        subtask.partition.end = subtask.partition.start + range_to_run;
        range_left -= range_to_run;

        /* add partition to pipe */
        ++num_added;
        if (!mm_sched_pipe_write(&s->pipes[gtl_thread_num], &subtask)) {
            /* pipe is full therefore directly call it */
            subtask.task->exec(subtask.task->userdata, s, subtask.partition.start,
                subtask.partition.end, gtl_thread_num);
            --num_added;
        }
    }

    /* increment running count by number added plus one to account for start value */
    mm_sched_atomic_add(&task->run_count, (mm_sched_int)(num_added+1));
    if (s->thread_active < s->thread_running)
        mm_sched_event_signal(s->event);
}

MM_SCHED_API void
mm_scheduler_join(struct mm_scheduler *s, struct mm_sched_task *task)
{
    mm_sched_uint pipe_to_check = gtl_thread_num+1;
    MM_SCHED_ASSERT(s);
    if (task) {
        while (task->run_count)
            mm_sched_try_running_task(s, gtl_thread_num, &pipe_to_check);
    } else {
        mm_sched_try_running_task(s, gtl_thread_num, &pipe_to_check);
    }
}

MM_SCHED_API void
mm_scheduler_wait(struct mm_scheduler *s)
{
    mm_sched_int have_task = 1;
    mm_sched_uint pipe_hint = gtl_thread_num+1;
    MM_SCHED_ASSERT(s);

    while (have_task || s->thread_active > 1) {
        mm_sched_uint i = 0;
        mm_sched_try_running_task(s, gtl_thread_num, &pipe_hint);
        have_task = 0;
        for (i = 0; i < s->threads_num; ++i) {
            if (!mm_sched_pipe_is_empty(&s->pipes[i])) {
                have_task = 1;
                break;
            }
        }
    }
}

MM_SCHED_API void
mm_scheduler_stop(struct mm_scheduler *s)
{
    mm_sched_uint i = 0;
    MM_SCHED_ASSERT(s);
    if (!s->have_threads)
        return;

    /* wait for threads to quit and terminate them */
    s->running = 0;
    mm_scheduler_wait(s);
    while (s->thread_running > 1) {
        /* keep firing event to ensure all threads pick uo state of running*/
        mm_sched_event_signal(s->event);
    }
    for (i = 1; i < s->threads_num; ++i)
        mm_sched_thread_term(((mm_sched_thread*)(s->threads))[i]);

    mm_sched_event_close(s->event);
    s->thread_running = 0;
    s->thread_active = 0;
    s->have_threads = 0;
    s->threads = 0;
    s->pipes = 0;
    s->event = 0;
    s->args = 0;
}

#endif /* MM_SCHED_IMPLEMENTATION */
