/// @module system

/// Terminal.
// Unix: see https://blog.nelhage.com/2009/12/a-brief-introduction-to-termios-termios3-and-stty/
//
// Windows: see https://learn.microsoft.com/en-us/windows/console/console-reference
// @section terminal

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "compat.h"
#include "bitflags.h"


#ifdef _WIN32
# include <windows.h>
# include <locale.h>
# ifndef _MSC_VER
#  include <conio.h>
#  include <unistd.h>
# endif
#else
# include <termios.h>
# include <string.h>
# include <errno.h>
# include <fcntl.h>
# include <sys/ioctl.h>
# include <unistd.h>
# include <wchar.h>
# include <locale.h>
#endif


// Windows does not have a wcwidth function, so we use compatibilty code from
// http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c by Markus Kuhn
#include "wcwidth.h"


#ifdef _WIN32
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
        lua_pushfstring(L, "%sError code %d", prefix ? prefix : "", errorCode);
    }
}
#else
static int pusherror(lua_State *L, const char *info)
{
	lua_pushnil(L);
	if (info==NULL)
		lua_pushstring(L, strerror(errno));
	else
		lua_pushfstring(L, "%s: %s", info, strerror(errno));
	lua_pushinteger(L, errno);
	return 3;
}
#endif

/***
Checks if a file-handle is a TTY.

@function isatty
@tparam file file the file-handle to check, one of `io.stdin`, `io.stdout`, `io.stderr`.
@treturn boolean true if the file is a tty
@usage
local system = require('system')
if system.isatty(io.stdin) then
    -- enable ANSI coloring etc on Windows, does nothing in Posix.
    local flags = system.getconsoleflags(io.stdout)
    system.setconsoleflags(io.stdout, flags + sys.COF_VIRTUAL_TERMINAL_PROCESSING)
end
*/
static int lst_isatty(lua_State* L) {
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
#define CHECK_WIN_FLAG_OR_ZERO(flag) flag
#define CHECK_NIX_FLAG_OR_ZERO(flag) 0
#else
#define CHECK_WIN_FLAG_OR_ZERO(flag) 0
#define CHECK_NIX_FLAG_OR_ZERO(flag) flag
#endif

// Export Windows constants to Lua
static const struct ls_RegConst win_console_in_flags[] = {
    // Console Input Flags
    {"CIF_ECHO_INPUT", CHECK_WIN_FLAG_OR_ZERO(ENABLE_ECHO_INPUT)},
    {"CIF_INSERT_MODE", CHECK_WIN_FLAG_OR_ZERO(ENABLE_INSERT_MODE)},
    {"CIF_LINE_INPUT", CHECK_WIN_FLAG_OR_ZERO(ENABLE_LINE_INPUT)},
    {"CIF_MOUSE_INPUT", CHECK_WIN_FLAG_OR_ZERO(ENABLE_MOUSE_INPUT)},
    {"CIF_PROCESSED_INPUT", CHECK_WIN_FLAG_OR_ZERO(ENABLE_PROCESSED_INPUT)},
    {"CIF_QUICK_EDIT_MODE", CHECK_WIN_FLAG_OR_ZERO(ENABLE_QUICK_EDIT_MODE)},
    {"CIF_WINDOW_INPUT", CHECK_WIN_FLAG_OR_ZERO(ENABLE_WINDOW_INPUT)},
    {"CIF_VIRTUAL_TERMINAL_INPUT", CHECK_WIN_FLAG_OR_ZERO(ENABLE_VIRTUAL_TERMINAL_INPUT)},
    {"CIF_EXTENDED_FLAGS", CHECK_WIN_FLAG_OR_ZERO(ENABLE_EXTENDED_FLAGS)},
    {"CIF_AUTO_POSITION", CHECK_WIN_FLAG_OR_ZERO(ENABLE_AUTO_POSITION)},
    {NULL, 0}
};

static const struct ls_RegConst win_console_out_flags[] = {
    // Console Output Flags
    {"COF_PROCESSED_OUTPUT", CHECK_WIN_FLAG_OR_ZERO(ENABLE_PROCESSED_OUTPUT)},
    {"COF_WRAP_AT_EOL_OUTPUT", CHECK_WIN_FLAG_OR_ZERO(ENABLE_WRAP_AT_EOL_OUTPUT)},
    {"COF_VIRTUAL_TERMINAL_PROCESSING", CHECK_WIN_FLAG_OR_ZERO(ENABLE_VIRTUAL_TERMINAL_PROCESSING)},
    {"COF_DISABLE_NEWLINE_AUTO_RETURN", CHECK_WIN_FLAG_OR_ZERO(DISABLE_NEWLINE_AUTO_RETURN)},
    {"COF_ENABLE_LVB_GRID_WORLDWIDE", CHECK_WIN_FLAG_OR_ZERO(ENABLE_LVB_GRID_WORLDWIDE)},
    {NULL, 0}
};


// Export Unix constants to Lua
static const struct ls_RegConst nix_tcsetattr_actions[] = {
    // The optional actions for tcsetattr
    {"TCSANOW", CHECK_NIX_FLAG_OR_ZERO(TCSANOW)},
    {"TCSADRAIN", CHECK_NIX_FLAG_OR_ZERO(TCSADRAIN)},
    {"TCSAFLUSH", CHECK_NIX_FLAG_OR_ZERO(TCSAFLUSH)},
    {NULL, 0}
};

static const struct ls_RegConst nix_console_i_flags[] = {
    // Input flags (c_iflag)
    {"I_IGNBRK", CHECK_NIX_FLAG_OR_ZERO(IGNBRK)},
    {"I_BRKINT", CHECK_NIX_FLAG_OR_ZERO(BRKINT)},
    {"I_IGNPAR", CHECK_NIX_FLAG_OR_ZERO(IGNPAR)},
    {"I_PARMRK", CHECK_NIX_FLAG_OR_ZERO(PARMRK)},
    {"I_INPCK", CHECK_NIX_FLAG_OR_ZERO(INPCK)},
    {"I_ISTRIP", CHECK_NIX_FLAG_OR_ZERO(ISTRIP)},
    {"I_INLCR", CHECK_NIX_FLAG_OR_ZERO(INLCR)},
    {"I_IGNCR", CHECK_NIX_FLAG_OR_ZERO(IGNCR)},
    {"I_ICRNL", CHECK_NIX_FLAG_OR_ZERO(ICRNL)},
#ifndef __APPLE__
    {"I_IUCLC", CHECK_NIX_FLAG_OR_ZERO(IUCLC)}, // Might not be available on all systems
#else
    {"I_IUCLC", 0},
#endif
    {"I_IXON", CHECK_NIX_FLAG_OR_ZERO(IXON)},
    {"I_IXANY", CHECK_NIX_FLAG_OR_ZERO(IXANY)},
    {"I_IXOFF", CHECK_NIX_FLAG_OR_ZERO(IXOFF)},
    {"I_IMAXBEL", CHECK_NIX_FLAG_OR_ZERO(IMAXBEL)},
    {NULL, 0}
};

static const struct ls_RegConst nix_console_o_flags[] = {
    // Output flags (c_oflag)
    {"O_OPOST", CHECK_NIX_FLAG_OR_ZERO(OPOST)},
#ifndef __APPLE__
    {"O_OLCUC", CHECK_NIX_FLAG_OR_ZERO(OLCUC)}, // Might not be available on all systems
#else
    {"O_OLCUC", 0},
#endif
    {"O_ONLCR", CHECK_NIX_FLAG_OR_ZERO(ONLCR)},
    {"O_OCRNL", CHECK_NIX_FLAG_OR_ZERO(OCRNL)},
    {"O_ONOCR", CHECK_NIX_FLAG_OR_ZERO(ONOCR)},
    {"O_ONLRET", CHECK_NIX_FLAG_OR_ZERO(ONLRET)},
    {"O_OFILL", CHECK_NIX_FLAG_OR_ZERO(OFILL)},
    {"O_OFDEL", CHECK_NIX_FLAG_OR_ZERO(OFDEL)},
    {"O_NLDLY", CHECK_NIX_FLAG_OR_ZERO(NLDLY)},
    {"O_CRDLY", CHECK_NIX_FLAG_OR_ZERO(CRDLY)},
    {"O_TABDLY", CHECK_NIX_FLAG_OR_ZERO(TABDLY)},
    {"O_BSDLY", CHECK_NIX_FLAG_OR_ZERO(BSDLY)},
    {"O_VTDLY", CHECK_NIX_FLAG_OR_ZERO(VTDLY)},
    {"O_FFDLY", CHECK_NIX_FLAG_OR_ZERO(FFDLY)},
    {NULL, 0}
};

static const struct ls_RegConst nix_console_l_flags[] = {
    // Local flags (c_lflag)
    {"L_ISIG", CHECK_NIX_FLAG_OR_ZERO(ISIG)},
    {"L_ICANON", CHECK_NIX_FLAG_OR_ZERO(ICANON)},
#ifndef __APPLE__
    {"L_XCASE", CHECK_NIX_FLAG_OR_ZERO(XCASE)}, // Might not be available on all systems
#else
    {"L_XCASE", 0},
#endif
    {"L_ECHO", CHECK_NIX_FLAG_OR_ZERO(ECHO)},
    {"L_ECHOE", CHECK_NIX_FLAG_OR_ZERO(ECHOE)},
    {"L_ECHOK", CHECK_NIX_FLAG_OR_ZERO(ECHOK)},
    {"L_ECHONL", CHECK_NIX_FLAG_OR_ZERO(ECHONL)},
    {"L_NOFLSH", CHECK_NIX_FLAG_OR_ZERO(NOFLSH)},
    {"L_TOSTOP", CHECK_NIX_FLAG_OR_ZERO(TOSTOP)},
    {"L_ECHOCTL", CHECK_NIX_FLAG_OR_ZERO(ECHOCTL)}, // Might not be available on all systems
    {"L_ECHOPRT", CHECK_NIX_FLAG_OR_ZERO(ECHOPRT)}, // Might not be available on all systems
    {"L_ECHOKE", CHECK_NIX_FLAG_OR_ZERO(ECHOKE)}, // Might not be available on all systems
    {"L_FLUSHO", CHECK_NIX_FLAG_OR_ZERO(FLUSHO)},
    {"L_PENDIN", CHECK_NIX_FLAG_OR_ZERO(PENDIN)},
    {"L_IEXTEN", CHECK_NIX_FLAG_OR_ZERO(IEXTEN)},
    {NULL, 0}
};

static DWORD win_valid_in_flags = 0;
static DWORD win_valid_out_flags = 0;
static DWORD nix_valid_i_flags = 0;
static DWORD nix_valid_o_flags = 0;
static DWORD nix_valid_l_flags = 0;
static void initialize_valid_flags()
{
    win_valid_in_flags = 0;
    for (int i = 0; win_console_in_flags[i].name != NULL; i++)
    {
        win_valid_in_flags |= win_console_in_flags[i].value;
    }
    win_valid_out_flags = 0;
    for (int i = 0; win_console_out_flags[i].name != NULL; i++)
    {
        win_valid_out_flags |= win_console_out_flags[i].value;
    }
    nix_valid_i_flags = 0;
    for (int i = 0; nix_console_i_flags[i].name != NULL; i++)
    {
        nix_valid_i_flags |= nix_console_i_flags[i].value;
    }
    nix_valid_o_flags = 0;
    for (int i = 0; nix_console_o_flags[i].name != NULL; i++)
    {
        nix_valid_o_flags |= nix_console_o_flags[i].value;
    }
    nix_valid_l_flags = 0;
    for (int i = 0; nix_console_l_flags[i].name != NULL; i++)
    {
        nix_valid_l_flags |= nix_console_l_flags[i].value;
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
        valid = win_valid_in_flags;

    } else if (file == stdout && file != NULL) {
        handle =  GetStdHandle(STD_OUTPUT_HANDLE);
        valid = win_valid_out_flags;

    } else if (file == stderr && file != NULL) {
        handle =  GetStdHandle(STD_ERROR_HANDLE);
        valid = win_valid_out_flags;

    } else {
        luaL_argerror(L, 1, "invalid file handle"); // does not return
    }

    if (handle == INVALID_HANDLE_VALUE) {
        termFormatError(L, GetLastError(), "failed to retrieve std handle");
        lua_error(L); // does not return
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

    LSBF_BITFLAG flags = lsbf_checkbitflags(L, 2);
    if ((flags & ~valid) != 0) {
        luaL_argerror(L, 2, "invalid flags");
    }

    return handle;
}
#else
// first item on the stack should be io.stdin, io.stderr, or io.stdout. Throws a
// Lua error if the file is not one of these.
static int get_console_handle(lua_State *L)
{
    FILE **file = (FILE **)luaL_checkudata(L, 1, LUA_FILEHANDLE);
    if (file == NULL || *file == NULL) {
        return luaL_argerror(L, 1, "expected file handle"); // call doesn't return
    }

    // Check if the file is stdin, stdout, or stderr
    if (*file == stdin || *file == stdout || *file == stderr) {
        // Push the file descriptor onto the Lua stack
        return fileno(*file);
    }

    return luaL_argerror(L, 1, "invalid file handle"); // does not return
}
#endif



/***
Sets the console flags (Windows).
The `CIF_` and `COF_` constants are available on the module table. Where `CIF` are the
input flags (for use with `io.stdin`) and `COF` are the output flags (for use with
`io.stdout`/`io.stderr`).

To see flag status and constant names check `listconsoleflags`.

Note: not all combinations of flags are allowed, as some are mutually exclusive or mutually required.
See [setconsolemode documentation](https://learn.microsoft.com/en-us/windows/console/setconsolemode)
@function setconsoleflags
@tparam file file file handle to operate on, one of `io.stdin`, `io.stdout`, `io.stderr`
@tparam bitflags bitflags the flags to set/unset
@treturn[1] boolean `true` on success
@treturn[2] nil
@treturn[2] string error message
@usage
local system = require('system')
system.listconsoleflags(io.stdout) -- List all the available flags and their current status

local flags = system.getconsoleflags(io.stdout)
assert(system.setconsoleflags(io.stdout,
        flags + system.COF_VIRTUAL_TERMINAL_PROCESSING)

system.listconsoleflags(io.stdout) -- List again to check the differences
*/
static int lst_setconsoleflags(lua_State *L)
{
#ifdef _WIN32
    HANDLE console_handle = get_console_handle(L, 0);
    if (console_handle == NULL) {
        return 2; // error message is already on the stack
    }
    LSBF_BITFLAG new_console_mode = lsbf_checkbitflags(L, 2);

    if (!SetConsoleMode(console_handle, new_console_mode)) {
        termFormatError(L, GetLastError(), "failed to set console mode");
        return 2;
    }

#else
    get_console_handle(L); // to validate args
#endif

    lua_pushboolean(L, 1);
    return 1;
}



/***
Gets console flags (Windows).
The `CIF_` and `COF_` constants are available on the module table. Where `CIF` are the
input flags (for use with `io.stdin`) and `COF` are the output flags (for use with
`io.stdout`/`io.stderr`).

_Note_: See [setconsolemode documentation](https://learn.microsoft.com/en-us/windows/console/setconsolemode)
for more information on the flags.



@function getconsoleflags
@tparam file file file handle to operate on, one of `io.stdin`, `io.stdout`, `io.stderr`
@treturn[1] bitflags the current console flags.
@treturn[2] nil
@treturn[2] string error message
@usage
local system = require('system')

local flags = system.getconsoleflags(io.stdout)
print("Current stdout flags:", tostring(flags))

if flags:has_all_of(system.COF_VIRTUAL_TERMINAL_PROCESSING + system.COF_PROCESSED_OUTPUT) then
    print("Both flags are set")
else
    print("At least one flag is not set")
end
*/
static int lst_getconsoleflags(lua_State *L)
{
    DWORD console_mode = 0;

#ifdef _WIN32
    HANDLE console_handle = get_console_handle(L, 1);
    if (console_handle == NULL) {
        return 2; // error message is already on the stack
    }

    if (GetConsoleMode(console_handle, &console_mode) == 0)
    {
        lua_pushnil(L);
        lua_pushliteral(L, "failed to get console mode");
        return 2;
    }
#else
    get_console_handle(L); // to validate args

#endif
    lsbf_pushbitflags(L, console_mode);
    return 1;
}



/*-------------------------------------------------------------------------
 * Unix tcgetattr/tcsetattr functions
 *-------------------------------------------------------------------------*/
// Code modified from the LuaPosix library by Gary V. Vaughan
// see https://github.com/luaposix/luaposix

/***
Get termios state (Posix).
The terminal attributes is a table with the following fields:

- `iflag` input flags
- `oflag` output flags
- `lflag` local flags
- `cflag` control flags
- `ispeed` input speed
- `ospeed` output speed
- `cc` control characters

@function tcgetattr
@tparam file fd file handle to operate on, one of `io.stdin`, `io.stdout`, `io.stderr`
@treturn[1] termios terminal attributes, if successful. On Windows the bitflags are all 0, and the `cc` table is empty.
@treturn[2] nil
@treturn[2] string error message
@treturn[2] int errnum
@return error message if failed
@usage
local system = require('system')

local status = assert(tcgetattr(io.stdin))
if status.iflag:has_all_of(system.I_IGNBRK) then
    print("Ignoring break condition")
end
*/
static int lst_tcgetattr(lua_State *L)
{
#ifndef _WIN32
    int r, i;
    struct termios t;
    int fd = get_console_handle(L);

    r = tcgetattr(fd, &t);
    if (r == -1) return pusherror(L, NULL);

    lua_newtable(L);
    lsbf_pushbitflags(L, t.c_iflag);
    lua_setfield(L, -2, "iflag");

    lsbf_pushbitflags(L, t.c_oflag);
    lua_setfield(L, -2, "oflag");

    lsbf_pushbitflags(L, t.c_lflag);
    lua_setfield(L, -2, "lflag");

    lsbf_pushbitflags(L, t.c_cflag);
    lua_setfield(L, -2, "cflag");

    lua_pushinteger(L, cfgetispeed(&t));
    lua_setfield(L, -2, "ispeed");

    lua_pushinteger(L, cfgetospeed(&t));
    lua_setfield(L, -2, "ospeed");

    lua_newtable(L);
    for (i=0; i<NCCS; i++)
    {
        lua_pushinteger(L, i);
        lua_pushinteger(L, t.c_cc[i]);
        lua_settable(L, -3);
    }
    lua_setfield(L, -2, "cc");

#else
    lua_settop(L, 1); // remove all but file handle
    get_console_handle(L, 1); //check args

    lua_newtable(L);
    lsbf_pushbitflags(L, 0);
    lua_setfield(L, -2, "iflag");
    lsbf_pushbitflags(L, 0);
    lua_setfield(L, -2, "oflag");
    lsbf_pushbitflags(L, 0);
    lua_setfield(L, -2, "lflag");
    lsbf_pushbitflags(L, 0);
    lua_setfield(L, -2, "cflag");
    lua_pushinteger(L, 0);
    lua_setfield(L, -2, "ispeed");
    lua_pushinteger(L, 0);
    lua_setfield(L, -2, "ospeed");
    lua_newtable(L);
    lua_setfield(L, -2, "cc");

#endif
    return 1;
}



/***
Set termios state (Posix).
This function will set the flags as given.

The `I_`, `O_`, and `L_` constants are available on the module table. They are the respective
flags for the `iflags`, `oflags`, and `lflags` bitmasks.

To see flag status and constant names check `listtermflags`. For their meaning check
[the manpage](https://www.man7.org/linux/man-pages/man3/termios.3.html).

_Note_: only `iflag`, `oflag`, and `lflag` are supported at the moment. The other fields are ignored.
@function tcsetattr
@tparam file fd file handle to operate on, one of `io.stdin`, `io.stdout`, `io.stderr`
@int actions one of `TCSANOW`, `TCSADRAIN`, `TCSAFLUSH`
@tparam table termios a table with bitflag fields:
@tparam[opt] bitflags termios.iflag if given will set the input flags
@tparam[opt] bitflags termios.oflag if given will set the output flags
@tparam[opt] bitflags termios.lflag if given will set the local flags
@treturn[1] bool `true`, if successful. Always returns `true` on Windows.
@return[2] nil
@treturn[2] string error message
@treturn[2] int errnum
@usage
local system = require('system')

local status = assert(tcgetattr(io.stdin))
if not status.lflag:has_all_of(system.L_ECHO) then
    -- if echo is off, turn echoing newlines on
    tcsetattr(io.stdin, system.TCSANOW, { lflag = status.lflag + system.L_ECHONL }))
end
*/
static int lst_tcsetattr(lua_State *L)
{
#ifndef _WIN32
    struct termios t;
    int r, i;
    int fd = get_console_handle(L);     // first is the console handle
    int act = luaL_checkinteger(L, 2);  // second is the action to take

    r = tcgetattr(fd, &t);
    if (r == -1) return pusherror(L, NULL);

    t.c_iflag = lsbf_checkbitflagsfield(L, 3, "iflag", t.c_iflag);
    t.c_oflag = lsbf_checkbitflagsfield(L, 3, "oflag", t.c_oflag);
    t.c_lflag = lsbf_checkbitflagsfield(L, 3, "lflag", t.c_lflag);

    // Skipping the others for now

    // lua_getfield(L, 3, "cflag"); t.c_cflag = optint(L, -1, 0); lua_pop(L, 1);
    // lua_getfield(L, 3, "ispeed"); cfsetispeed( &t, optint(L, -1, B0) ); lua_pop(L, 1);
    // lua_getfield(L, 3, "ospeed"); cfsetospeed( &t, optint(L, -1, B0) ); lua_pop(L, 1);

    // lua_getfield(L, 3, "cc");
    // for (i=0; i<NCCS; i++)
    // {
    //     lua_pushinteger(L, i);
    //     lua_gettable(L, -2);
    //     t.c_cc[i] = optint(L, -1, 0);
    //     lua_pop(L, 1);
    // }

    r = tcsetattr(fd, act, &t);
    if (r == -1) return pusherror(L, NULL);

#else
    // Windows does not have a tcsetattr function, but we check arguments anyway
    luaL_checkinteger(L, 2);
    lsbf_checkbitflagsfield(L, 3, "iflag", 0);
    lsbf_checkbitflagsfield(L, 3, "oflag", 0);
    lsbf_checkbitflagsfield(L, 3, "lflag", 0);
    lua_settop(L, 1); // remove all but file handle
    get_console_handle(L, 1);
#endif

    lua_pushboolean(L, 1);
    return 1;
}



/***
Enables or disables non-blocking mode for a file (Posix).
@function setnonblock
@tparam file fd file handle to operate on, one of `io.stdin`, `io.stdout`, `io.stderr`
@tparam boolean make_non_block a truthy value will enable non-blocking mode, a falsy value will disable it.
@treturn[1] bool `true`, if successful
@treturn[2] nil
@treturn[2] string error message
@treturn[2] int errnum
@see getnonblock
@usage
local sys = require('system')

-- set io.stdin to non-blocking mode
local old_setting = sys.getnonblock(io.stdin)
sys.setnonblock(io.stdin, true)

-- do stuff

-- restore old setting
sys.setnonblock(io.stdin, old_setting)
*/
static int lst_setnonblock(lua_State *L)
{
#ifndef _WIN32

    int fd = get_console_handle(L);

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return pusherror(L, "Error getting handle flags: ");
    }
    if (lua_toboolean(L, 2)) {
        // truthy: set non-blocking
        flags |= O_NONBLOCK;
    } else {
        // falsy: set disable non-blocking
        flags &= ~O_NONBLOCK;
    }
    if (fcntl(fd, F_SETFL, flags) == -1) {
        return pusherror(L, "Error changing O_NONBLOCK: ");
    }

#else
    if (lua_gettop(L) > 1) {
        lua_settop(L, 1); // use one argument, because the second boolean will fail as get_console_flags expects bitflags
    }
    HANDLE console_handle = get_console_handle(L, 1);
    if (console_handle == NULL) {
        return 2; // error message is already on the stack
    }

#endif

    lua_pushboolean(L, 1);
    return 1;
}



/***
Gets non-blocking mode status for a file (Posix).
@function getnonblock
@tparam file fd file handle to operate on, one of `io.stdin`, `io.stdout`, `io.stderr`
@treturn[1] bool `true` if set to non-blocking, `false` if not. Always returns `false` on Windows.
@treturn[2] nil
@treturn[2] string error message
@treturn[2] int errnum
*/
static int lst_getnonblock(lua_State *L)
{
#ifndef _WIN32

    int fd = get_console_handle(L);

    // Set O_NONBLOCK
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return pusherror(L, "Error getting handle flags: ");
    }
    if (flags & O_NONBLOCK) {
        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }

#else
    if (lua_gettop(L) > 1) {
        lua_settop(L, 1); // use one argument, because the second boolean will fail as get_console_flags expects bitflags
    }
    HANDLE console_handle = get_console_handle(L, 1);
    if (console_handle == NULL) {
        return 2; // error message is already on the stack
    }

    lua_pushboolean(L, 0);

#endif
    return 1;
}



/*-------------------------------------------------------------------------
 * Reading keyboard input
 *-------------------------------------------------------------------------*/

#ifdef _WIN32
// Define a static buffer for UTF-8 characters
static char utf8_buffer[4];
static int utf8_buffer_len = 0;
static int utf8_buffer_index = 0;
#endif


/***
Reads a key from the console non-blocking. This function should not be called
directly, but through the `system.readkey` or `system.readansi` functions. It
will return the next byte from the input stream, or `nil` if no key was pressed.

On Posix, `io.stdin` must be set to non-blocking mode using `setnonblock`
and canonical mode must be turned off using `tcsetattr`,
before calling this function. Otherwise it will block. No conversions are
done on Posix, so the byte read is returned as-is.

On Windows this reads a wide character and converts it to UTF-8. Multi-byte
sequences will be buffered internally and returned one byte at a time.

@function _readkey
@treturn[1] integer the byte read from the input stream
@treturn[2] nil if no key was pressed
@treturn[3] nil on error
@treturn[3] string error message
@treturn[3] int errnum (on posix)
*/
static int lst_readkey(lua_State *L) {
#ifdef _WIN32
    if (utf8_buffer_len > 0) {
        // Buffer not empty, return the next byte
        lua_pushinteger(L, (unsigned char)utf8_buffer[utf8_buffer_index]);
        utf8_buffer_index++;
        utf8_buffer_len--;
        // printf("returning from buffer: %d\n", luaL_checkinteger(L, -1));
        if (utf8_buffer_len == 0) {
            utf8_buffer_index = 0;
        }
        return 1;
    }

    if (!_kbhit()) {
        return 0;
    }

    wchar_t wc = _getwch();
    // printf("----\nread wchar_t: %x\n", wc);
    if (wc == WEOF) {
        lua_pushnil(L);
        lua_pushliteral(L, "read error");
        return 2;
    }

    if (sizeof(wchar_t) == 2) {
        // printf("2-byte wchar_t\n");
        // only 2 bytes wide, not 4
        if (wc >= 0xD800 && wc <= 0xDBFF) {
            // printf("2-byte wchar_t, received high, getting low...\n");

            // we got a high surrogate, so we need to read the next one as the low surrogate
            if (!_kbhit()) {
                lua_pushnil(L);
                lua_pushliteral(L, "incomplete surrogate pair");
                return 2;
            }

            wchar_t wc2 = _getwch();
            // printf("read wchar_t 2: %x\n", wc2);
            if (wc2 == WEOF) {
                lua_pushnil(L);
                lua_pushliteral(L, "read error");
                return 2;
            }

            if (wc2 < 0xDC00 || wc2 > 0xDFFF) {
                lua_pushnil(L);
                lua_pushliteral(L, "invalid surrogate pair");
                return 2;
            }
            // printf("2-byte pair complete now\n");
            wchar_t wch_pair[2] = { wc, wc2 };
            utf8_buffer_len = WideCharToMultiByte(CP_UTF8, 0, wch_pair, 2, utf8_buffer, sizeof(utf8_buffer), NULL, NULL);

        } else {
            // printf("2-byte wchar_t, no surrogate pair\n");
            // not a high surrogate, so we can handle just the 2 bytes directly
            utf8_buffer_len = WideCharToMultiByte(CP_UTF8, 0, &wc, 1, utf8_buffer, sizeof(utf8_buffer), NULL, NULL);
        }

    } else {
        // printf("4-byte wchar_t\n");
        // 4 bytes wide, so handle as UTF-32 directly
        utf8_buffer_len = WideCharToMultiByte(CP_UTF8, 0, &wc, 1, utf8_buffer, sizeof(utf8_buffer), NULL, NULL);
    }
    // printf("utf8_buffer_len: %d\n", utf8_buffer_len);
    utf8_buffer_index = 0;
    if (utf8_buffer_len <= 0) {
        lua_pushnil(L);
        lua_pushliteral(L, "UTF-8 conversion error");
        return 2;
    }

    lua_pushinteger(L, (unsigned char)utf8_buffer[utf8_buffer_index]);
    utf8_buffer_index++;
    utf8_buffer_len--;
    // printf("returning from buffer: %x\n", luaL_checkinteger(L, -1));
    return 1;

#else
    // Posix implementation
    char ch;
    ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
    if (bytes_read > 0) {
        lua_pushinteger(L, (unsigned char)ch);
        return 1;

    } else if (bytes_read == 0) {
        return 0;  // End of file or stream closed

    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Resource temporarily unavailable, no data available to read
            return 0;
        } else {
            return pusherror(L, "read error");
        }
    }

#endif
}



/*-------------------------------------------------------------------------
 * Retrieve terminal size
 *-------------------------------------------------------------------------*/


/***
Get the size of the terminal in rows and columns.
@function termsize
@treturn[1] int the number of rows
@treturn[1] int the number of columns
@treturn[2] nil
@treturn[2] string error message
*/
static int lst_termsize(lua_State *L) {
    int columns, rows;

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        termFormatError(L, GetLastError(), "Failed to get terminal size.");
        return 2;
    }
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

#else
    struct winsize ws;
    if (ioctl(1, TIOCGWINSZ, &ws) == -1) {
        return pusherror(L, "Failed to get terminal size.");
    }
    columns = ws.ws_col;
    rows = ws.ws_row;

#endif
    lua_pushinteger(L, rows);
    lua_pushinteger(L, columns);
    return 2;
}



/*-------------------------------------------------------------------------
 * utf8 conversion and support
 *-------------------------------------------------------------------------*/

// Function to convert a single UTF-8 character to a Unicode code point (uint32_t)
// To prevent having to do codepage/locale changes, we use a custom implementation
int utf8_to_wchar(const char *utf8, size_t len, mk_wchar_t *codepoint) {
    if (len == 0) {
        return -1; // No input provided
    }

    unsigned char c = (unsigned char)utf8[0];
    if (c <= 0x7F) {
        *codepoint = c;
        return 1;
    } else if ((c & 0xE0) == 0xC0) {
        if (len < 2) return -1; // Not enough bytes
        *codepoint = ((utf8[0] & 0x1F) << 6) | (utf8[1] & 0x3F);
        return 2;
    } else if ((c & 0xF0) == 0xE0) {
        if (len < 3) return -1; // Not enough bytes
        *codepoint = ((utf8[0] & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
        return 3;
    } else if ((c & 0xF8) == 0xF0) {
        if (len < 4) return -1; // Not enough bytes
        *codepoint = ((utf8[0] & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
        return 4;
    } else {
        // Invalid UTF-8 character
        return -1;
    }
}


/***
Get the width of a utf8 character for terminal display.
@function utf8cwidth
@tparam string utf8_char the utf8 character to check, only the width of the first character will be returned
@treturn[1] int the display width in columns of the first character in the string (0 for an empty string)
@treturn[2] nil
@treturn[2] string error message
*/
int lst_utf8cwidth(lua_State *L) {
    const char *utf8_char;
    size_t utf8_len;
    utf8_char = luaL_checklstring(L, 1, &utf8_len);
    int width = 0;

    mk_wchar_t wc;

    if (utf8_len == 0) {
        lua_pushinteger(L, 0);
        return 1;
    }

    // Convert the UTF-8 string to a wide character
    int bytes_processed = utf8_to_wchar(utf8_char, utf8_len, &wc);
    if (bytes_processed == -1) {
        lua_pushnil(L);
        lua_pushstring(L, "Invalid UTF-8 character");
        return 2;
    }

    // Get the width of the wide character
    width = mk_wcwidth(wc);
    if (width == -1) {
        lua_pushnil(L);
        lua_pushstring(L, "Character width determination failed");
        return 2;
    }

    lua_pushinteger(L, width);
    return 1;
}




/***
Get the width of a utf8 string for terminal display.
@function utf8swidth
@tparam string utf8_string the utf8 string to check
@treturn[1] int the display width of the string in columns (0 for an empty string)
@treturn[2] nil
@treturn[2] string error message
*/
int lst_utf8swidth(lua_State *L) {
    const char *utf8_str;
    size_t utf8_len;
    utf8_str = luaL_checklstring(L, 1, &utf8_len);
    int total_width = 0;

    if (utf8_len == 0) {
        lua_pushinteger(L, 0);
        return 1;
    }

    int bytes_processed = 0;
    size_t i = 0;
    mk_wchar_t wc;

    while (i < utf8_len) {
        bytes_processed = utf8_to_wchar(utf8_str + i, utf8_len - i, &wc);
        if (bytes_processed == -1) {
            lua_pushnil(L);
            lua_pushstring(L, "Invalid UTF-8 character");
            return 2;
        }

        int width = mk_wcwidth(wc);
        if (width == -1) {
            lua_pushnil(L);
            lua_pushstring(L, "Character width determination failed");
            return 2;
        }

        total_width += width;
        i += bytes_processed;
    }

    lua_pushinteger(L, total_width);
    return 1;
}



/*-------------------------------------------------------------------------
 * Windows codepage functions
 *-------------------------------------------------------------------------*/


/***
Gets the current console code page (Windows).
@function getconsolecp
@treturn[1] int the current code page (always 65001 on Posix systems)
*/
static int lst_getconsolecp(lua_State *L) {
    unsigned int cp = 65001;
#ifdef _WIN32
    cp = GetConsoleCP();
#endif
    lua_pushinteger(L, cp);
    return 1;
}



/***
Sets the current console code page (Windows).
@function setconsolecp
@tparam int cp the code page to set, use `system.CODEPAGE_UTF8` (65001) for UTF-8
@treturn[1] bool `true` on success (always `true` on Posix systems)
*/
static int lst_setconsolecp(lua_State *L) {
    unsigned int cp = (unsigned int)luaL_checkinteger(L, 1);
    int success = TRUE;
#ifdef _WIN32
    SetConsoleCP(cp);
#endif
    lua_pushboolean(L, success);
    return 1;
}



/***
Gets the current console output code page (Windows).
@function getconsoleoutputcp
@treturn[1] int the current code page (always 65001 on Posix systems)
*/
static int lst_getconsoleoutputcp(lua_State *L) {
    unsigned int cp = 65001;
#ifdef _WIN32
    cp = GetConsoleOutputCP();
#endif
    lua_pushinteger(L, cp);
    return 1;
}



/***
Sets the current console output code page (Windows).
@function setconsoleoutputcp
@tparam int cp the code page to set, use `system.CODEPAGE_UTF8` (65001) for UTF-8
@treturn[1] bool `true` on success (always `true` on Posix systems)
*/
static int lst_setconsoleoutputcp(lua_State *L) {
    unsigned int cp = (unsigned int)luaL_checkinteger(L, 1);
    int success = TRUE;
#ifdef _WIN32
    SetConsoleOutputCP(cp);
#endif
    lua_pushboolean(L, success);
    return 1;
}



/*-------------------------------------------------------------------------
 * Initializes module
 *-------------------------------------------------------------------------*/

static luaL_Reg func[] = {
    { "isatty", lst_isatty },
    { "getconsoleflags", lst_getconsoleflags },
    { "setconsoleflags", lst_setconsoleflags },
    { "tcgetattr", lst_tcgetattr },
    { "tcsetattr", lst_tcsetattr },
    { "getnonblock", lst_getnonblock },
    { "setnonblock", lst_setnonblock },
    { "_readkey", lst_readkey },
    { "termsize", lst_termsize },
    { "utf8cwidth", lst_utf8cwidth },
    { "utf8swidth", lst_utf8swidth },
    { "getconsolecp", lst_getconsolecp },
    { "setconsolecp", lst_setconsolecp },
    { "getconsoleoutputcp", lst_getconsoleoutputcp },
    { "setconsoleoutputcp", lst_setconsoleoutputcp },
    { NULL, NULL }
};



void term_open(lua_State *L) {
    // set up constants and export the constants in module table
    initialize_valid_flags();
    // Windows flags
    for (int i = 0; win_console_in_flags[i].name != NULL; i++)
    {
        lsbf_pushbitflags(L, win_console_in_flags[i].value);
        lua_setfield(L, -2, win_console_in_flags[i].name);
    }
    for (int i = 0; win_console_out_flags[i].name != NULL; i++)
    {
        lsbf_pushbitflags(L, win_console_out_flags[i].value);
        lua_setfield(L, -2, win_console_out_flags[i].name);
    }
    // Unix flags
    for (int i = 0; nix_console_i_flags[i].name != NULL; i++)
    {
        lsbf_pushbitflags(L, nix_console_i_flags[i].value);
        lua_setfield(L, -2, nix_console_i_flags[i].name);
    }
    for (int i = 0; nix_console_o_flags[i].name != NULL; i++)
    {
        lsbf_pushbitflags(L, nix_console_o_flags[i].value);
        lua_setfield(L, -2, nix_console_o_flags[i].name);
    }
    for (int i = 0; nix_console_l_flags[i].name != NULL; i++)
    {
        lsbf_pushbitflags(L, nix_console_l_flags[i].value);
        lua_setfield(L, -2, nix_console_l_flags[i].name);
    }
    // Unix tcsetattr actions
    for (int i = 0; nix_tcsetattr_actions[i].name != NULL; i++)
    {
        lua_pushinteger(L, nix_tcsetattr_actions[i].value);
        lua_setfield(L, -2, nix_tcsetattr_actions[i].name);
    }

    // export functions
    luaL_setfuncs(L, func, 0);
}
