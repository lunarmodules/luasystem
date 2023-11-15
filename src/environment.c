/// @submodule system
#include <lua.h>
#include <lauxlib.h>
#include "compat.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "windows.h"
#endif

/***
Gets the value of an environment variable.

__NOTE__: Windows has multiple copies of environment variables. For this reason,
the `setenv` function will not work with Lua's `os.getenv` on Windows. If you want
to use `setenv` then consider patching `os.getenv` with this implementation of `getenv`.
@function getenv
@tparam string name name of the environment variable
@treturn string|nil value of the environment variable, or nil if the variable is not set
*/
static int lua_get_environment_variable(lua_State* L) {
    const char* variableName = luaL_checkstring(L, 1);

#ifdef _WIN32
    // On Windows, use GetEnvironmentVariable to retrieve the value
    DWORD bufferSize = GetEnvironmentVariable(variableName, NULL, 0);
    if (bufferSize > 0) {
        char* buffer = (char*)malloc(bufferSize);
        if (GetEnvironmentVariable(variableName, buffer, bufferSize) > 0) {
            lua_pushstring(L, buffer);
            free(buffer);
            return 1;
        }
        free(buffer);
    }
#else
    // On non-Windows platforms, use getenv to retrieve the value
    const char* variableValue = getenv(variableName);
    if (variableValue != NULL) {
        lua_pushstring(L, variableValue);
        return 1;
    }
#endif

    // If the variable is not set or an error occurs, push nil
    lua_pushnil(L);
    return 1;
}


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

__NOTE__: Windows has multiple copies of environment variables. For this reason, the
`setenv` function will not work with Lua's `os.getenv` on Windows. If you want to use
it then consider patching `os.getenv` with the implementation of `system.getenv`.
@function setenv
@tparam string name name of the environment variable
@tparam[opt] string value value of the environment variable, if `nil` the variable will be deleted (on
Windows, setting an empty string, will also delete the variable)
@treturn boolean success
*/
static int lua_set_environment_variable(lua_State* L) {
    const char* variableName = luaL_checkstring(L, 1);
    const char* variableValue = luaL_optstring(L, 2, NULL);

#ifdef _WIN32
    // if (variableValue == NULL) {
    //     // If the value is nil, delete the environment variable
    //     if (SetEnvironmentVariable(variableName, NULL)) {
    //         lua_pushboolean(L, 1);
    //     } else {
    //         lua_pushboolean(L, 0);
    //     }
    // } else {
        // Set the environment variable with the provided value
        if (SetEnvironmentVariable(variableName, variableValue)) {
            lua_pushboolean(L, 1);
        } else {
            lua_pushboolean(L, 0);
        }
    // }
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
    { "getenv", lua_get_environment_variable },
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
