#ifndef TIME_OSX_H
#define TIME_OSX_H

/*
 * clock_gettime()
 *
 * OS X doesn't implement the clock_gettime() POSIX interface, but does
 * provide a monotonic clock through mach_absolute_time(). On i386 and
 * x86_64 architectures this clock is in nanosecond units, but not so on
 * other devices. mach_timebase_info() provides the conversion parameters.
 *
 */

#include <time.h>            /* struct timespec */
#include <errno.h>           /* errno EINVAL */
#include <sys/time.h>        /* TIMEVAL_TO_TIMESPEC struct timeval gettimeofday(3) */
#include <mach/mach_time.h>  /* mach_timebase_info_data_t mach_timebase_info() mach_absolute_time() */


#define CLOCK_REALTIME  0
#define CLOCK_VIRTUAL   1
#define CLOCK_PROF      2
#define CLOCK_MONOTONIC 3

static mach_timebase_info_data_t clock_timebase = {
    .numer = 1, .denom = 1,
}; /* clock_timebase */

void clock_gettime_init(void) __attribute__((constructor));

void clock_gettime_init(void) {
    if (mach_timebase_info(&clock_timebase) != KERN_SUCCESS)
        __builtin_abort();
} /* clock_gettime_init() */

static int clock_gettime(int clockid, struct timespec *ts) {
    switch (clockid) {
    case CLOCK_REALTIME: {
        struct timeval tv;

        if (0 != gettimeofday(&tv, 0))
        return -1;

        TIMEVAL_TO_TIMESPEC(&tv, ts);

        return 0;
    }
    case CLOCK_MONOTONIC: {
        unsigned long long abt;

        abt = mach_absolute_time();
        abt = abt * clock_timebase.numer / clock_timebase.denom;

        ts->tv_sec = abt / 1000000000UL;
        ts->tv_nsec = abt % 1000000000UL;

        return 0;
    }
    default:
        errno = EINVAL;

        return -1;
    } /* switch() */
} /* clock_gettime() */

#endif /* TIME_OSX_H */
