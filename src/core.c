#include <lua.h>
#include <lauxlib.h>

#define LUASYSTEM_VERSION   "LuaSystem 0.1.0"

#ifdef _WIN32
#define LUAEXPORT __declspec(dllexport)
#else
#define LUAEXPORT __attribute__((visibility("default")))
#endif

void time_open(lua_State *L);

/*-------------------------------------------------------------------------
 * Initializes all library modules.
 *-------------------------------------------------------------------------*/
LUAEXPORT int luaopen_system_core(lua_State *L) {
    lua_newtable(L);
    lua_pushstring(L, "_VERSION");
    lua_pushstring(L, LUASYSTEM_VERSION);
    lua_rawset(L, -3);
    time_open(L);
    return 1;
}
