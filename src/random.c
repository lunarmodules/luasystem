/// @submodule system
#include <lua.h>
#include <lauxlib.h>
#include "compat.h"
#include <fcntl.h>

#ifdef _WIN32
#include "windows.h"
#include "wincrypt.h"
#else
#include <unistd.h>
#endif


// Maximum buffer size for random bytes
#define MAX_RANDOM_BUFFER_SIZE 1024


/***
Generate random bytes.
This uses `getrandom()` on Linux, `CryptGenRandom()` on Windows, and `/dev/urandom` on macOS.
@function random
@tparam[opt=1] int length number of bytes to get, must be less than or equal to `MAX_RANDOM_BUFFER_SIZE` (1024)
@treturn[1] string string of random bytes
@treturn[2] nil
@treturn[2] string error message
*/
static int lua_get_random_bytes(lua_State* L) {
    int num_bytes = luaL_optinteger(L, 1, 1); // Number of bytes, default to 1 if not provided

    if (num_bytes <= 0 || num_bytes > MAX_RANDOM_BUFFER_SIZE) {
        lua_pushnil(L);
        lua_pushfstring(L, "invalid number of bytes, must be between 1 and %d", MAX_RANDOM_BUFFER_SIZE);
        return 2;
    }

    unsigned char buffer[MAX_RANDOM_BUFFER_SIZE];
    size_t n;

#ifdef _WIN32
    HCRYPTPROV hCryptProv;
    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        DWORD error = GetLastError();
        lua_pushnil(L);
        lua_pushfstring(L, "failed to acquire cryptographic context: %lu", error);
        return 2;
    }

    if (!CryptGenRandom(hCryptProv, num_bytes, buffer)) {
        DWORD error = GetLastError();
        lua_pushnil(L);
        lua_pushfstring(L, "failed to get random data: %lu", error);
        CryptReleaseContext(hCryptProv, 0);
        return 2;
    }

    CryptReleaseContext(hCryptProv, 0);
#else
#ifndef __APPLE__
    // Neither Apple nor Windows
    n = getrandom(buffer, num_bytes, 0);

    if (n < 0) {
        lua_pushnil(L);
        lua_pushstring(L, "failed to get random data");
        return 2;
    }
#else
    // macOS uses /dev/urandom for non-blocking
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        lua_pushnil(L);
        lua_pushstring(L, "failed opening /dev/urandom");
        return 2;
    }
    n = read(fd, buffer, num_bytes);
    close(fd);
    if (n < 0) {
      lua_pushnil(L);
      lua_pushstring(L, "failed reading /dev/urandom");
    return 2;
}

#endif

#endif

    lua_pushlstring(L, (const char*)buffer, num_bytes);
    return 1;
}



static luaL_Reg func[] = {
    { "random", lua_get_random_bytes },
    { NULL, NULL }
};



/*-------------------------------------------------------------------------
 * Initializes module
 *-------------------------------------------------------------------------*/
void random_open(lua_State *L) {
    luaL_setfuncs(L, func, 0);
    lua_pushinteger(L, MAX_RANDOM_BUFFER_SIZE);
    lua_setfield(L, -2, "MAX_RANDOM_BUFFER_SIZE");
}
