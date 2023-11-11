/// @submodule system
#include <lua.h>
#include <lauxlib.h>
#include "compat.h"
#include <stdlib.h>
#include <string.h>


/***
Returns a table with all environment variables.
@function getenvs
@treturn table table with all environment variables and their values
*/
static int lua_list_environment_variables(lua_State* L) {
    lua_newtable(L);

#ifdef _WIN32
    char* envStrings = GetEnvironmentStrings();
    char* envString = envStrings;

    if (envStrings == NULL) {
        lua_pushnil(L);
        return 1;
    }

    while (*envString != '\0') {
        const char* envVar = envString;

        // Split the environment variable into key and value
        char* equals = strchr(envVar, '=');
        if (equals != NULL) {
            lua_pushlstring(L, envVar, equals - envVar); // Push the key
            lua_pushstring(L, equals + 1); // Push the value
            lua_settable(L, -3); // Set the key-value pair in the table
        }

        envString += strlen(envString) + 1;
    }

    FreeEnvironmentStrings(envStrings);
#else
    extern char** environ;

    if (environ != NULL) {
        for (char** envVar = environ; *envVar != NULL; envVar++) {
            const char* envVarStr = *envVar;

            // Split the environment variable into key and value
            char* equals = strchr(envVarStr, '=');
            if (equals != NULL) {
                lua_pushlstring(L, envVarStr, equals - envVarStr); // Push the key
                lua_pushstring(L, equals + 1); // Push the value
                lua_settable(L, -3); // Set the key-value pair in the table
            }
        }
    }
#endif

    return 1;
}


/***
Sets an environment variable.
@function setenv
@tparam string name name of the environment variable
@tparam[opt] string value value of the environment variable, if nil the variable will be deleted
@treturn boolean success
*/
static int lua_set_environment_variable(lua_State* L) {
    const char* variableName = luaL_checkstring(L, 1);
    const char* variableValue = luaL_optstring(L, 2, NULL);

#ifdef _WIN32
    if (variableValue == NULL) {
        // If the value is nil, delete the environment variable
        if (SetEnvironmentVariable(variableName, NULL)) {
            lua_pushboolean(L, 1);
        } else {
            lua_pushboolean(L, 0);
        }
    } else {
        // Set the environment variable with the provided value
        if (SetEnvironmentVariable(variableName, variableValue)) {
            lua_pushboolean(L, 1);
        } else {
            lua_pushboolean(L, 0);
        }
    }
#else
    if (variableValue == NULL) {
        // If the value is nil, delete the environment variable
        if (unsetenv(variableName) == 0) {
            lua_pushboolean(L, 1);
        } else {
            lua_pushboolean(L, 0);
        }
    } else {
        // Set the environment variable with the provided value
        if (setenv(variableName, variableValue, 1) == 0) {
            lua_pushboolean(L, 1);
        } else {
            lua_pushboolean(L, 0);
        }
    }
#endif

    return 1;
}



static luaL_Reg func[] = {
    { "setenv", lua_set_environment_variable },
    { "getenvs", lua_list_environment_variables },
    { NULL, NULL }
};

/*-------------------------------------------------------------------------
 * Initializes module
 *-------------------------------------------------------------------------*/
void environment_open(lua_State *L) {
    luaL_setfuncs(L, func, 0);
}
