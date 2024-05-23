#ifndef COMPAT_H
#define COMPAT_H

#include <lua.h>
#include <lauxlib.h>

#if LUA_VERSION_NUM == 501
void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup);
void *luaL_testudata(lua_State *L, int ud, const char *tname);
#endif


#ifdef __MINGW32__
#include <sys/types.h>
#endif

// Windows compatibility; define DWORD and TRUE/FALSE on non-Windows
#ifndef _WIN32
#ifndef DWORD
#define DWORD unsigned long
#endif
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#endif

#ifdef _MSC_VER
// MSVC Windows doesn't have ssize_t, so we define it here
#if SIZE_MAX == UINT_MAX
typedef int ssize_t;        /* common 32 bit case */
#define SSIZE_MIN  INT_MIN
#define SSIZE_MAX  INT_MAX
#elif SIZE_MAX == ULONG_MAX
typedef long ssize_t;       /* linux 64 bits */
#define SSIZE_MIN  LONG_MIN
#define SSIZE_MAX  LONG_MAX
#elif SIZE_MAX == ULLONG_MAX
typedef long long ssize_t;  /* windows 64 bits */
#define SSIZE_MIN  LLONG_MIN
#define SSIZE_MAX  LLONG_MAX
#elif SIZE_MAX == USHRT_MAX
typedef short ssize_t;      /* is this even possible? */
#define SSIZE_MIN  SHRT_MIN
#define SSIZE_MAX  SHRT_MAX
#elif SIZE_MAX == UINTMAX_MAX
typedef intmax_t ssize_t;  /* last resort, chux suggestion */
#define SSIZE_MIN  INTMAX_MIN
#define SSIZE_MAX  INTMAX_MAX
#else
#error platform has exotic SIZE_MAX
#endif

#endif // _MSC_VER

#endif // COMPAT_H
