#include <lua.h>
#include <lauxlib.h>

#if LUA_VERSION_NUM == 501
void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
    luaL_checkstack(L, nup+1, "too many upvalues");
    for (; l->name != NULL; l++) {  /* fill the table with given functions */
        int i;
        lua_pushstring(L, l->name);
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
            lua_pushvalue(L, -(nup+1));
        lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
        lua_settable(L, -(nup + 3));
    }
    lua_pop(L, nup);  /* remove upvalues */
}

void *luaL_testudata(lua_State *L, int ud, const char *tname) {
    void *p = lua_touserdata(L, ud);
    if (p != NULL) {  /* Check for userdata */
        if (lua_getmetatable(L, ud)) {  /* Does it have a metatable? */
            lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* Get metatable we're looking for */
            if (lua_rawequal(L, -1, -2)) {  /* Compare metatables */
                lua_pop(L, 2);  /* Remove metatables from stack */
                return p;
            }
            lua_pop(L, 2);  /* Remove metatables from stack */
        }
    }
    return NULL;  /* Return NULL if check fails */
}

#endif
