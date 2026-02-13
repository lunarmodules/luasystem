/// @module system

/// Random.
// @section random


#include <lua.h>
#include <lauxlib.h>
#include "compat.h"
#include <fcntl.h>
#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
    #include <windows.h>
    #include <bcrypt.h>
#else
    #include <errno.h>
    #include <unistd.h>
    #include <string.h>
    #if defined(__linux__)
        // getrandom() requires random.h and is available from glibc 2.25
        #if !defined(__GLIBC__) || (__GLIBC__ < 2 || __GLIBC_MINOR__ < 25)
            #define USE_DEV_URANDOM 1
        #else
            #include <sys/random.h> // getrandom()
        #endif
    #elif defined(__APPLE__) || defined(__unix__)
        #include <stdlib.h>     // arc4random_buf()
    #endif
#endif


/* Fill buffer with n cryptographically secure random bytes. Returns 0 on success;
 * on failure pushes nil and error message and returns 2 (caller should return 2). */
static int fill_random_bytes(lua_State *L, unsigned char *buffer, size_t n) {
    size_t total_read = 0;

#ifdef _WIN32
    if (!BCRYPT_SUCCESS(BCryptGenRandom(NULL, buffer, (ULONG)n, BCRYPT_USE_SYSTEM_PREFERRED_RNG))) {
        lua_pushnil(L);
        lua_pushfstring(L, "failed to get random data: %lu", (unsigned long)GetLastError());
        return 2;
    }
    (void)total_read;

#elif defined(__linux__) && !defined(USE_DEV_URANDOM)
    while (total_read < n) {
        ssize_t got = getrandom(buffer + total_read, n - total_read, 0);
        if (got < 0) {
            if (errno == EINTR) continue;
            lua_pushnil(L);
            lua_pushfstring(L, "getrandom() failed: %s", strerror(errno));
            return 2;
        }
        total_read += (size_t)got;
    }

#elif defined(__APPLE__) || (defined(__unix__) && !defined(USE_DEV_URANDOM))
    arc4random_buf(buffer, n);

#else
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        lua_pushnil(L);
        lua_pushstring(L, "failed opening /dev/urandom");
        return 2;
    }
    while (total_read < n) {
        ssize_t got = read(fd, buffer + total_read, n - total_read);
        if (got < 0) {
            if (errno == EINTR) continue;
            lua_pushnil(L);
            lua_pushfstring(L, "failed reading /dev/urandom: %s", strerror(errno));
            close(fd);
            return 2;
        }
        total_read += (size_t)got;
    }
    close(fd);
#endif

    return 0;
}


/***
Generate random bytes.
This uses `BCryptGenRandom()` on Windows, `getrandom()` on Linux, `arc4random_buf` on BSD,
and `/dev/urandom` on other platforms. It will return the
requested number of bytes, or an error, never a partial result.
@function random
@tparam[opt=1] int length number of bytes to get
@treturn[1] string string of random bytes
@treturn[2] nil
@treturn[2] string error message
*/
static int lua_get_random_bytes(lua_State* L) {
    int num_bytes = luaL_optinteger(L, 1, 1); // Number of bytes, default to 1 if not provided

    if (num_bytes <= 0) {
        if (num_bytes == 0) {
            lua_pushliteral(L, "");
            return 1;
        }
        lua_pushnil(L);
        lua_pushstring(L, "invalid number of bytes, must not be less than 0");
        return 2;
    }

    unsigned char* buffer = (unsigned char*)lua_newuserdata(L, num_bytes);
    if (buffer == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "failed to allocate memory for random buffer");
        return 2;
    }

    int ret = fill_random_bytes(L, buffer, (size_t)num_bytes);
    if (ret != 0) return ret;

    lua_pushlstring(L, (const char*)buffer, num_bytes);
    return 1;
}


/* Read 8 bytes into *out; return 0 on success, 2 on error (nil + msg pushed). */
static int read_u64(lua_State *L, uint64_t *out) {
    unsigned char buf[8];

    int ret = fill_random_bytes(L, buf, 8);
    if (ret != 0) return ret;

    *out = (uint64_t)buf[0] | ((uint64_t)buf[1] << 8) | ((uint64_t)buf[2] << 16) |
           ((uint64_t)buf[3] << 24) | ((uint64_t)buf[4] << 32) | ((uint64_t)buf[5] << 40) |
           ((uint64_t)buf[6] << 48) | ((uint64_t)buf[7] << 56);
    return 0;
}

/* Project uniform random in [0, n] using rejection (Mersenne-style). *out is in [0, n]. */
static int project_u64(lua_State *L, uint64_t ran, uint64_t n, uint64_t *out) {
    if ((n & (n + 1)) == 0) {
        *out = ran & n;
        return 0;
    }
    uint64_t lim = n;
    lim |= (lim >> 1);
    lim |= (lim >> 2);
    lim |= (lim >> 4);
    lim |= (lim >> 8);
    lim |= (lim >> 16);
    lim |= (lim >> 32);
    while ((ran &= lim) > n) {
        int ret = read_u64(L, &ran);
        if (ret != 0) return ret;
    }
    *out = ran;
    return 0;
}


/***
Random number mimicking Lua 5.4 math.random, using crypto-secure bytes.
- No args: returns a float in [0, 1).
- One arg 0: returns a full-range random integer (whole lua_Integer range).
- One arg m (m >= 1): returns an integer in [1, m] (inclusive).
- Two args m, n: returns an integer in [m, n] (inclusive).
On invalid arguments throws an error (same as math.random).
@function rnd
@tparam[opt] int m upper bound (or 0 for full-range), or lower bound when used with n
@tparam[opt] int n upper bound (when used with m)
@treturn number|int float in [0,1) or integer in the requested range
*/
static int lua_rnd(lua_State *L) {
    int nargs = lua_gettop(L);
    unsigned char buf[8];
    uint64_t u;

    if (nargs == 0) {
        int ret = fill_random_bytes(L, buf, 8);
        if (ret != 0) return ret;
        u = (uint64_t)buf[0] | ((uint64_t)buf[1] << 8) | ((uint64_t)buf[2] << 16) |
            ((uint64_t)buf[3] << 24) | ((uint64_t)buf[4] << 32) | ((uint64_t)buf[5] << 40) |
            ((uint64_t)buf[6] << 48) | ((uint64_t)buf[7] << 56);
        /* 53 bits for double [0, 1) */
        lua_pushnumber(L, (lua_Number)((u >> 11) * (1.0 / ((uint64_t)1 << 53))));
        return 1;
    }

#if LUA_VERSION_NUM >= 503
    #define RND_INT lua_Integer
    #define RND_CHECKINT(L, i) luaL_checkinteger(L, (i))
    #define RND_MAXINTEGER LUA_MAXINTEGER
    #define RND_MININTEGER LUA_MININTEGER
#else
    #define RND_INT long long
    #define RND_CHECKINT(L, i) ((long long)luaL_checknumber(L, (i)))
#endif

    if (nargs == 1) {
        RND_INT m = RND_CHECKINT(L, 1);
        if (m == 0) {
            /* full-range random integer */
            int ret = read_u64(L, &u);
            if (ret != 0) return ret;
#if LUA_VERSION_NUM >= 503
            lua_pushinteger(L, (lua_Integer)(int64_t)u);
#else
            lua_pushnumber(L, (lua_Number)(long long)(int64_t)u);
#endif
            return 1;
        }
        if (m < 1) {
            return luaL_error(L, "interval is empty");
        }
        /* [1, m] -> range size m, values 0..m-1 then +1 */
        {
            uint64_t r;
            int ret = read_u64(L, &u);
            if (ret != 0) return ret;
            ret = project_u64(L, u, (uint64_t)(m - 1), &r);
            if (ret != 0) return ret;
#if LUA_VERSION_NUM >= 503
            lua_pushinteger(L, (lua_Integer)r + 1);
#else
            lua_pushnumber(L, (lua_Number)((long long)r + 1));
#endif
            return 1;
        }
    }

    if (nargs == 2) {
        RND_INT low = RND_CHECKINT(L, 1);
        RND_INT up  = RND_CHECKINT(L, 2);
        if (low > up) {
            return luaL_error(L, "interval is empty");
        }
#if LUA_VERSION_NUM >= 503
        if (low < 0 && up > 0 && (lua_Unsigned)(up - low) > (lua_Unsigned)LUA_MAXINTEGER) {
            return luaL_error(L, "interval too large");
        }
#endif
        {
            uint64_t range = (uint64_t)(up - low) + 1;
            uint64_t r;
            int ret = read_u64(L, &u);
            if (ret != 0) return ret;
            ret = project_u64(L, u, range - 1, &r);
            if (ret != 0) return ret;
#if LUA_VERSION_NUM >= 503
            lua_pushinteger(L, (lua_Integer)((lua_Unsigned)low + (lua_Unsigned)r));
#else
            lua_pushnumber(L, (lua_Number)((long long)low + (long long)r));
#endif
            return 1;
        }
    }

    return luaL_error(L, "wrong number of arguments");

#undef RND_INT
#undef RND_CHECKINT
#undef RND_MAXINTEGER
#undef RND_MININTEGER
}


static luaL_Reg func[] = {
    { "random", lua_get_random_bytes },
    { "rnd", lua_rnd },
    { NULL, NULL }
};



/*-------------------------------------------------------------------------
 * Initializes module
 *-------------------------------------------------------------------------*/
void random_open(lua_State *L) {
    luaL_setfuncs(L, func, 0);
}
