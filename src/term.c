/// @submodule system
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "compat.h"

#ifndef _MSC_VER
# include <unistd.h>
#endif

#ifdef _WIN32
# include <windows.h>
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


/*-------------------------------------------------------------------------
 * Windows Get/SetConsoleMode functions
 *-------------------------------------------------------------------------*/

typedef struct ls_RegConst {
  const char *name;
  DWORD value;
} ls_RegConst;

// Export constants to Lua
static const struct ls_RegConst console_in_flags[] = {
#ifdef _WIN32
    // setting for input handles
    {"CIF_ECHO_INPUT", ENABLE_ECHO_INPUT},
    {"CIF_INSERT_MODE", ENABLE_INSERT_MODE},
    {"CIF_LINE_INPUT", ENABLE_LINE_INPUT},
    {"CIF_MOUSE_INPUT", ENABLE_MOUSE_INPUT},
    {"CIF_PROCESSED_INPUT", ENABLE_PROCESSED_INPUT},
    {"CIF_QUICK_EDIT_MODE", ENABLE_QUICK_EDIT_MODE},
    {"CIF_WINDOW_INPUT", ENABLE_WINDOW_INPUT},
    {"CIF_VIRTUAL_TERMINAL_INPUT", ENABLE_VIRTUAL_TERMINAL_INPUT},
    {"CIF_EXTENDED_FLAGS", ENABLE_EXTENDED_FLAGS},
    {"CIF_AUTO_POSITION", ENABLE_AUTO_POSITION}, // TODO: undocumented, might be an "out" flag...
#endif
    {NULL, 0}
};

static const struct ls_RegConst console_out_flags[] = {
#ifdef _WIN32
    // setting for output handles
    {"COF_PROCESSED_OUTPUT", ENABLE_PROCESSED_OUTPUT},
    {"COF_WRAP_AT_EOL_OUTPUT", ENABLE_WRAP_AT_EOL_OUTPUT},
    {"COF_VIRTUAL_TERMINAL_PROCESSING", ENABLE_VIRTUAL_TERMINAL_PROCESSING},
    {"COF_DISABLE_NEWLINE_AUTO_RETURN", DISABLE_NEWLINE_AUTO_RETURN},
    {"COF_ENABLE_LVB_GRID_WORLDWIDE ", ENABLE_LVB_GRID_WORLDWIDE},
#endif
    {NULL, 0}
};

static DWORD valid_in_flags = 0;
static DWORD valid_out_flags = 0;
static void initialize_valid_flags()
{
    valid_in_flags = 0;
    for (int i = 0; console_in_flags[i].name != NULL; i++)
    {
        valid_in_flags |= console_in_flags[i].value;
    }
    valid_out_flags = 0;
    for (int i = 0; console_out_flags[i].name != NULL; i++)
    {
        valid_out_flags |= console_out_flags[i].value;
    }
}

#ifdef _WIN32
// first item on the stack should be io.stdin, io.stderr, or io.stdout, second item
// should be the flags to validate.
// If it returns NULL, then it leaves nil+err on the stack
static HANDLE get_console_handle(lua_State *L, int flags_optional)
{
    if (lua_gettop(L) < 1) {
        luaL_argerror(L, 1, "expected file handle");
    }

    HANDLE handle;
    DWORD valid;
    FILE *file = *(FILE **)luaL_checkudata(L, 1, LUA_FILEHANDLE);
    if (file == stdin && file != NULL) {
        handle = GetStdHandle(STD_INPUT_HANDLE);
        valid = valid_in_flags;

    } else if (file == stdout && file != NULL) {
        handle =  GetStdHandle(STD_OUTPUT_HANDLE);
        valid = valid_out_flags;

    } else if (file == stderr && file != NULL) {
        handle =  GetStdHandle(STD_ERROR_HANDLE);
        valid = valid_out_flags;

    } else {
        luaL_argerror(L, 1, "invalid file handle"); // does not return
    }

    if (handle == INVALID_HANDLE_VALUE || handle == NULL) {
        lua_pushnil(L);
        lua_pushliteral(L, "failed to get console handle");
        return NULL;
    }

    if (!flags_optional && lua_gettop(L) < 2) {
        return handle;
    }

    if (lua_gettop(L) < 2) {
        luaL_argerror(L, 2, "expected flags");
    }

    DWORD flags = (DWORD)luaL_checkinteger(L, 2);
    if ((flags & ~valid) != 0) {
        luaL_argerror(L, 2, "invalid flags");
    }

    return handle;
}
#endif


static int lua_setconsoleflags(lua_State *L, int unsetting)
{
#ifdef _WIN32
    HANDLE console_handle = get_console_handle(L, 0);
    if (console_handle == NULL) {
        return 2; // error message is already on the stack
    }
    DWORD flags = (DWORD)luaL_checkinteger(L, 2); // flags are already validated

    if (unsetting) {
        flags = ~flags;
    }

    DWORD prev_console_mode;
    if (GetConsoleMode(console_handle, &prev_console_mode) == 0)
    {
        lua_pushnil(L);
        lua_pushliteral(L, "failed to get console mode");
        return 2;
    }

    DWORD new_console_mode = (prev_console_mode & ~flags) | flags;

    int success = SetConsoleMode(console_handle, new_console_mode) != 0;
    if (!success)
    {
        lua_pushnil(L);
        lua_pushliteral(L, "failed to set console mode");
        return 2;
    }
#endif

    lua_pushboolean(L, 1);
    return 1;
}



/***
Enables console flags (Windows).

@function enableconsoleflags
@tparam file file the file-handle to set the flags on
@tparam integer bitmask the flags to set
@treturn[1] boolean success (always `true` on non-Windows platforms)
@treturn[2] nil
@treturn[2] string error message
*/
static int lua_enableconsoleflags(lua_State *L)
{
    return lua_setconsoleflags(L, 0);
}



/***
Disables console flags (Windows).

@function disableconsoleflags
@tparam file file the file-handle to unset the flags on
@tparam integer bitmask the flags to disable
@treturn[1] boolean success (always `true` on non-Windows platforms)
@treturn[2] nil
@treturn[2] string error message
*/
static int lua_disableconsoleflags(lua_State *L)
{
    return lua_setconsoleflags(L, 1);
}


/***
Checks console flags (Windows).

@function getconsoleflags
@tparam file file the file-handle to check the flags on
@tparam[opt] integer bitmask the flags to check. If not provided, returns the combined current console flags.
@treturn[1] integer the current console flags (if no parameters are passed), always 0 on non-Windows platforms.
@treturn[2] boolean true if all the requested flags are currently set, or false if at least one is not set. Always false on non-Windows platforms.
@treturn[3] nil
@treturn[3] string error message
*/
static int lua_getconsoleflags(lua_State *L)
{
#ifdef _WIN32
    HANDLE console_handle = get_console_handle(L, 1);
    if (console_handle == NULL) {
        return 2; // error message is already on the stack
    }

    DWORD console_mode;
    if (GetConsoleMode(console_handle, &console_mode) == 0)
    {
        lua_pushnil(L);
        lua_pushliteral(L, "failed to get console mode");
        return 2;
    }

    // If no flag parameters are passed, return the current console flags
    if (lua_gettop(L) < 2)
    {
        lua_pushinteger(L, console_mode);
        return 1;
    }

    // If parameters are passed, check if the provided flags are currently set
    DWORD flags = (DWORD)luaL_checkinteger(L, 2); // flags are already validated
    int all_set = (console_mode & flags) == flags;
    lua_pushboolean(L, all_set);
    return 1;
#else
    if (lua_gettop(L) < 2)
    {
        lua_pushinteger(L, 0);
        return 1;
    }
    lua_pushboolean(L, 0);
#endif
}



static luaL_Reg func[] = {
    { "isatty", lua_isatty },
    { "enableconsoleflags", lua_enableconsoleflags },
    { "disableconsoleflags", lua_disableconsoleflags },
    { "getconsoleflags", lua_getconsoleflags },
    { NULL, NULL }
};

/*-------------------------------------------------------------------------
 * Initializes module
 *-------------------------------------------------------------------------*/
void term_open(lua_State *L) {
    // set up constants and export the constants in module table
    initialize_valid_flags();
    for (int i = 0; console_in_flags[i].name != NULL; i++)
    {
        lua_pushinteger(L, console_in_flags[i].value);
        lua_setfield(L, -2, console_in_flags[i].name);
    }
    for (int i = 0; console_out_flags[i].name != NULL; i++)
    {
        lua_pushinteger(L, console_out_flags[i].value);
        lua_setfield(L, -2, console_out_flags[i].name);
    }
    // export functions
    luaL_setfuncs(L, func, 0);
}
