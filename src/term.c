/// @submodule system
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "compat.h"

#ifndef _MSC_VER
# include <unistd.h>
#endif


/***
Checks if a file-handle is a TTY.

@function isatty
@tparam file file the file-handle to check
@treturn boolean true if the file is a tty
*/
static int lua_isatty(lua_State* L) {
    FILE **fh = (FILE **) luaL_checkudata(L, 1, LUA_FILEHANDLE);
    lua_pushboolean(L, isatty(fileno(*fh)));
    return 1;
}



static luaL_Reg func[] = {
    { "isatty", lua_isatty },
    { NULL, NULL }
};

/*-------------------------------------------------------------------------
 * Initializes module
 *-------------------------------------------------------------------------*/
void term_open(lua_State *L) {
    luaL_setfuncs(L, func, 0);
}
