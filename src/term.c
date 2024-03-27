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


// after an error is returned, GetLastError() result can be passed to this function to get a string
// representation of the error on the stack.
// result will be nil+error on the stack, always 2 results.
static void termFormatError(lua_State *L, DWORD errorCode, const char* prefix) {
//static void FormatErrorAndReturn(lua_State *L, DWORD errorCode, const char* prefix) {
    LPSTR messageBuffer = NULL;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    lua_pushnil(L);
    if (messageBuffer) {
        if (prefix) {
            lua_pushfstring(L, "%s: %s", prefix, messageBuffer);
        } else {
            lua_pushstring(L, messageBuffer);
        }
        LocalFree(messageBuffer);
    } else {
        lua_pushfstring(L, "%sError code %lu", prefix ? prefix : "", errorCode);
    }
}

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

// Define a macro to check if a constant is defined and set it to 0 if not.
// This is needed because some flags are not defined on all platforms. So we
// still export the constants, but they will be all 0, and hence not do anything.
#ifdef _WIN32
#define CHECK_FLAG_OR_ZERO(flag) flag
#else
#define CHECK_FLAG_OR_ZERO(flag) 0
#endif

// Export constants to Lua
static const struct ls_RegConst console_in_flags[] = {
    // Console Input Flags
    {"CIF_ECHO_INPUT", CHECK_FLAG_OR_ZERO(ENABLE_ECHO_INPUT)},
    {"CIF_INSERT_MODE", CHECK_FLAG_OR_ZERO(ENABLE_INSERT_MODE)},
    {"CIF_LINE_INPUT", CHECK_FLAG_OR_ZERO(ENABLE_LINE_INPUT)},
    {"CIF_MOUSE_INPUT", CHECK_FLAG_OR_ZERO(ENABLE_MOUSE_INPUT)},
    {"CIF_PROCESSED_INPUT", CHECK_FLAG_OR_ZERO(ENABLE_PROCESSED_INPUT)},
    {"CIF_QUICK_EDIT_MODE", CHECK_FLAG_OR_ZERO(ENABLE_QUICK_EDIT_MODE)},
    {"CIF_WINDOW_INPUT", CHECK_FLAG_OR_ZERO(ENABLE_WINDOW_INPUT)},
    {"CIF_VIRTUAL_TERMINAL_INPUT", CHECK_FLAG_OR_ZERO(ENABLE_VIRTUAL_TERMINAL_INPUT)},
    {"CIF_EXTENDED_FLAGS", CHECK_FLAG_OR_ZERO(ENABLE_EXTENDED_FLAGS)},
    {"CIF_AUTO_POSITION", CHECK_FLAG_OR_ZERO(ENABLE_AUTO_POSITION)},
    {NULL, 0}
};

static const struct ls_RegConst console_out_flags[] = {
    // Console Output Flags
    {"COF_PROCESSED_OUTPUT", CHECK_FLAG_OR_ZERO(ENABLE_PROCESSED_OUTPUT)},
    {"COF_WRAP_AT_EOL_OUTPUT", CHECK_FLAG_OR_ZERO(ENABLE_WRAP_AT_EOL_OUTPUT)},
    {"COF_VIRTUAL_TERMINAL_PROCESSING", CHECK_FLAG_OR_ZERO(ENABLE_VIRTUAL_TERMINAL_PROCESSING)},
    {"COF_DISABLE_NEWLINE_AUTO_RETURN", CHECK_FLAG_OR_ZERO(DISABLE_NEWLINE_AUTO_RETURN)},
    {"COF_ENABLE_LVB_GRID_WORLDWIDE ", CHECK_FLAG_OR_ZERO(ENABLE_LVB_GRID_WORLDWIDE)},
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

    if (handle == INVALID_HANDLE_VALUE) {
        termFormatError(L, GetLastError(), "failed to retrieve std handle");
        return 2;
    }

    if (handle == NULL) {
        lua_pushnil(L);
        lua_pushliteral(L, "failed to get console handle");
        return NULL;
    }

    if (flags_optional && lua_gettop(L) < 2) {
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

#define SETFLAGS_AS_GIVEN 0
#define SETFLAGS_ENABLE 1
#define SETFLAGS_DISABLE 2

static int lualib_setconsoleflags(lua_State *L, int setting)
{
#ifdef _WIN32
    HANDLE console_handle = get_console_handle(L, 0);
    if (console_handle == NULL) {
        return 2; // error message is already on the stack
    }
    DWORD flags = (DWORD)lua_tointeger(L, 2); // flags are already validated

    DWORD prev_console_mode;
    if (GetConsoleMode(console_handle, &prev_console_mode) == 0)
    {
        termFormatError(L, GetLastError(), "failed to get console mode");
        return 2;
    }

    DWORD new_console_mode = flags;
    if (setting == SETFLAGS_DISABLE) {
        new_console_mode = prev_console_mode & ~flags;
    } else if (setting == SETFLAGS_ENABLE) {
        new_console_mode = prev_console_mode | flags;
    }

    int success = SetConsoleMode(console_handle, new_console_mode) != 0;
    if (!success)
    {
        termFormatError(L, GetLastError(), "failed to set console mode");
        return 2;
    }

    lua_pushinteger(L, prev_console_mode);
    lua_pushinteger(L, new_console_mode);
    return 2;

#else
    lua_pushinteger(L, 0);
    lua_pushinteger(L, 0);
    return 2;

#endif
}



/***
Sets the full set of console flags (Windows).

@function setconsoleflags
@tparam file file the file-handle to set the flags on
@tparam integer bitmask the flags to set/unset
@treturn[1] the old console flags (always 0 on non-Windows platforms)
@treturn[1] the new console flags (always 0 on non-Windows platforms)
@treturn[2] nil
@treturn[2] string error message
@see https://learn.microsoft.com/en-us/windows/console/setconsolemode
*/
static int lua_setconsoleflags(lua_State *L)
{
    return lualib_setconsoleflags(L, SETFLAGS_AS_GIVEN);
}



/***
Enables console flags (Windows).

@function enableconsoleflags
@tparam file file the file-handle to set the flags on
@tparam integer bitmask the flags to set
@treturn[1] the old console flags (always 0 on non-Windows platforms)
@treturn[1] the new console flags (always 0 on non-Windows platforms)
@treturn[2] nil
@treturn[2] string error message
@see https://learn.microsoft.com/en-us/windows/console/setconsolemode
*/
static int lua_enableconsoleflags(lua_State *L)
{
    return lualib_setconsoleflags(L, SETFLAGS_ENABLE);
}



/***
Disables console flags (Windows).

@function disableconsoleflags
@tparam file file the file-handle to unset the flags on
@tparam integer bitmask the flags to disable
@treturn[1] the old console flags (always 0 on non-Windows platforms)
@treturn[1] the new console flags (always 0 on non-Windows platforms)
@treturn[2] nil
@treturn[2] string error message
@see https://learn.microsoft.com/en-us/windows/console/setconsolemode
*/
static int lua_disableconsoleflags(lua_State *L)
{
    return lualib_setconsoleflags(L, SETFLAGS_DISABLE);
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
@see https://learn.microsoft.com/en-us/windows/console/getconsolemode
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
    DWORD flags = (DWORD)luaL_checkinteger(L, 2); // flags were already validated
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



/*-------------------------------------------------------------------------
 * backup/restore terminal state
 *-------------------------------------------------------------------------*/

static const char *backup_key = "LuaSystem.TermBackup";
static const char *backup_key_mt = "LuaSystem.TermBackup.mt";

typedef struct TermBackup {
    int stdin_value;
    int stdout_value;
    int stderr_value;
} TermBackup;



/***
Stores the current console flags (Windows).

Creates a backup of the console settings. This backup can be restored using `termrestore`.
Upon exiting the Lua state, the backup will be automatically restored.
@function termbackup
@treturn[1] boolean true if the backup was successful
@treturn[2] nil
@treturn[2] string error message, if the backup already existed
*/
static int lua_termbackup(lua_State *L) {
    // Check if the backup already exists in the registry
    lua_getfield(L, LUA_REGISTRYINDEX, backup_key);
    if (!lua_isnil(L, -1)) {
        lua_pushnil(L);
        lua_pushliteral(L, "backup already exists");
        return 2;
    }

    lua_pop(L, 1); // Pop the nil value off the stack

    TermBackup *backup = (TermBackup *)lua_newuserdata(L, sizeof(TermBackup));

    // Get current state of the console flags
    if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &backup->stdin_value)) {
        backup->stdin_value = -1;
    };
    if (!GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &backup->stdout_value)) {
        backup->stdout_value = -1;
    };
    if (!GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &backup->stderr_value)) {
        backup->stderr_value = -1;
    };
    luaL_getmetatable(L, backup_key_mt);
    lua_setmetatable(L, -2);

    // Store the backup in the registry
    lua_setfield(L, LUA_REGISTRYINDEX, backup_key);

    lua_pushboolean(L, TRUE);
    return 1;
}


/***
Restores the console flags backup (Windows).

Restores the console settings from a backup created with `termbackup`.
It will automatically be restored when the Lua state is closed, if not done already.
@function termrestore
@treturn[1] boolean true if the restore was successful
@treturn[2] nil
@treturn[2] string error message, if the backup didn't exist (anymore)
*/
static int lua_termrestore(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, backup_key);
    if (lua_isnil(L, -1)) {
        lua_pushnil(L);
        lua_pushliteral(L, "backup does not exist");
        return 2;
    }
    TermBackup *backup = (TermBackup *)lua_touserdata(L, -1);

    // Restore the backup values
    if (backup->stdin_value != -1) {
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), backup->stdin_value);
        backup->stdin_value = -1;
    }
    if (backup->stdout_value != -1) {
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), backup->stdout_value);
        backup->stdout_value = -1;
    }
    if (backup->stderr_value != -1) {
        SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), backup->stderr_value);
        backup->stderr_value = -1;
    }

    lua_pushboolean(L, TRUE);
    return 1;
}



static int termbackup_gc(lua_State *L) {
    lua_termrestore(L);
    // printf("termbackup_gc\n");
    return 0;
}


/*-------------------------------------------------------------------------
 * Initializes module
 *-------------------------------------------------------------------------*/

static luaL_Reg func[] = {
    { "isatty", lua_isatty },
    { "enableconsoleflags", lua_enableconsoleflags },
    { "disableconsoleflags", lua_disableconsoleflags },
    { "getconsoleflags", lua_getconsoleflags },
    { "setconsoleflags", lua_setconsoleflags },
    { "termbackup", lua_termbackup },
    { "termrestore", lua_termrestore },
    { NULL, NULL }
};



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

    // set up backup/restore meta-table
    luaL_newmetatable(L, backup_key_mt);
    lua_pushcfunction(L, termbackup_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1); // pop the metatable from the stack

    // export functions
    luaL_setfuncs(L, func, 0);
}
