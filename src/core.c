/// Platform independent system calls for Lua.
// @module system

#include <lua.h>
#include <lauxlib.h>

#define LUASYSTEM_VERSION   "LuaSystem 0.2.1"

#ifdef _WIN32
#define LUAEXPORT __declspec(dllexport)
#else
#define LUAEXPORT __attribute__((visibility("default")))
#endif

void time_open(lua_State *L);
void environment_open(lua_State *L);
void random_open(lua_State *L);
void term_open(lua_State *L);

/*-------------------------------------------------------------------------
 * Initializes all library modules.
 *-------------------------------------------------------------------------*/
LUAEXPORT int luaopen_system_core(lua_State *L) {
    lua_newtable(L);
    lua_pushstring(L, "_VERSION");
    lua_pushstring(L, LUASYSTEM_VERSION);
    lua_rawset(L, -3);
    lua_pushstring(L, "windows");
#ifdef _WIN32
    lua_pushboolean(L, 1);
#else
    lua_pushboolean(L, 0);
#endif
    lua_rawset(L, -3);
    time_open(L);
    random_open(L);
    term_open(L);
    environment_open(L);
    return 1;
}
