/// @submodule system
#include <lua.h>
#include <lauxlib.h>

#ifdef _WIN32
#include <float.h>
#include <limits.h>
#include <windows.h>
#else
#include <time.h>
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <AvailabilityMacros.h>

#ifndef MAC_OS_X_VERSION_10_12
#define MAC_OS_X_VERSION_10_12 101200
#endif

#define HAVE_CLOCK_GETTIME   (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12 || defined(CLOCK_MONOTONIC))

#if !(HAVE_CLOCK_GETTIME)
#include "time_osx.h"
#endif
#endif

#include "compat.h"

/*-------------------------------------------------------------------------
 * Gets time in s, relative to January 1, 1970 (UTC)
 * Returns
 *   time in s.
 *-------------------------------------------------------------------------*/
#ifdef _WIN32
static double time_gettime(void) {
    FILETIME ft;
    double t;
    GetSystemTimeAsFileTime(&ft);
    /* Windows file time (time since January 1, 1601 (UTC)) */
    t  = ft.dwLowDateTime*1.0e-7 + ft.dwHighDateTime*(4294967296.0*1.0e-7);
    /* convert to Unix Epoch time (time since January 1, 1970 (UTC)) */
    return (t - 11644473600.0);
}
#else
static double time_gettime(void) {
    struct timeval v;
    gettimeofday(&v, (struct timezone *) NULL);
    /* Unix Epoch time (time since January 1, 1970 (UTC)) */
    return v.tv_sec + v.tv_usec*1.0e-6;
}
#endif



#ifdef _WIN32
WINBASEAPI ULONGLONG WINAPI GetTickCount64(VOID);

static double time_monotime(void) {
    ULONGLONG ms = GetTickCount64();
    return ms*1.0e-3;
}
#else
static double time_monotime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec*1.0e-9;
}
#endif



/***
Get Unix time.
The time is returned as the seconds since the Unix epoch (1 January 1970 00:00:00).
The return value cannot be used with `os.date` on Windows.
@function unixtime
@treturn number seconds (fractional)
*/
static int time_lua_nix_gettime(lua_State *L)
{
    lua_pushnumber(L, time_gettime());
    return 1;
}



/***
Get Windows time.
The time is returned as the seconds since the Windows epoch (1 January 1601 00:00:00).
The return value cannot be used with `os.date` on Unix.
@function windowstime
@treturn number seconds (fractional)
*/
static int time_lua_win_gettime(lua_State *L)
{
    lua_pushnumber(L, time_gettime() + 11644473600.0);
    return 1;
}



/***
Get system time.
The time is returned as the seconds since the epoch, which differs between Unix
and Windows. The return value can be used with `os.date` on both Unix and Windows.
@function time
@treturn number seconds (fractional)
*/
static int time_lua_sys_gettime(lua_State *L)
{
#ifdef _WIN32
    return time_lua_win_gettime(L);
#else
    return time_lua_nix_gettime(L);
#endif
}



/***
Get monotonic time.
The time is returned as the seconds since system start.
@function monotime
@treturn number seconds (fractional)
*/
static int time_lua_monotime(lua_State *L)
{
    lua_pushnumber(L, time_monotime());
    return 1;
}



/***
Sleep without a busy loop.
This function will sleep, without doing a busy-loop and wasting CPU cycles.
@function sleep
@tparam number seconds seconds to sleep (fractional).
@return nothing
*/
#ifdef _WIN32
static int time_lua_sleep(lua_State *L)
{
    double n = luaL_checknumber(L, 1);
    if (n < 0.0) n = 0.0;
    if (n < DBL_MAX/1000.0) n *= 1000.0;
    if (n > INT_MAX) n = INT_MAX;
    Sleep((int)n);
    return 0;
}
#else
static int time_lua_sleep(lua_State *L)
{
    double n = luaL_checknumber(L, 1);
    struct timespec t, r;
    if (n < 0.0) n = 0.0;
    if (n > INT_MAX) n = INT_MAX;
    t.tv_sec = (int) n;
    n -= t.tv_sec;
    t.tv_nsec = (int) (n * 1000000000);
    if (t.tv_nsec >= 1000000000) t.tv_nsec = 999999999;
    while (nanosleep(&t, &r) != 0) {
        t.tv_sec = r.tv_sec;
        t.tv_nsec = r.tv_nsec;
    }
    return 0;
}
#endif

static luaL_Reg func[] = {
    { "gettime", time_lua_nix_gettime },  // deprecated: backward compatibility
    { "time", time_lua_sys_gettime },
    { "unixtime", time_lua_nix_gettime },
    { "windowstime", time_lua_win_gettime },
    { "monotime", time_lua_monotime },
    { "sleep", time_lua_sleep },
    { NULL, NULL }
};

/*-------------------------------------------------------------------------
 * Initializes module
 *-------------------------------------------------------------------------*/
void time_open(lua_State *L) {
    luaL_setfuncs(L, func, 0);
}
