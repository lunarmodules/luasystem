/// Bitflags module.
// The bitflag object makes it easy to manipulate flags in a bitmask.
//
// It has metamethods that do the hard work, adding flags sets them, substracting
// unsets them. Comparing flags checks if all flags in the second set are also set
// in the first set. The `has` method checks if all flags in the second set are
// also set in the first set, but behaves slightly different.
//
// Indexing allows checking values or setting them by bit index (eg. 0-7 for flags
// in the first byte).
//
// _NOTE_: unavailable flags (eg. Windows flags on a Posix system) should not be
// omitted, but be assigned a value of 0. This is because the `has` method will
// return `false` if the flags are checked and the value is 0.
//
// See `system.bitflag` (the constructor) for extensive examples on usage.
// @classmod bitflags
#include "bitflags.h"

#define BITFLAGS_MT_NAME "LuaSystem.BitFlags"

typedef struct {
    LSBF_BITFLAG flags;
} LS_BitFlags;

/// Bit flags.
// Bitflag objects can be used to easily manipulate and compare bit flags.
// These are primarily for use with the terminal functions, but can be used
// in other places as well.
// @section bitflags


// pushes a new LS_BitFlags object with the given value onto the stack
void lsbf_pushbitflags(lua_State *L, LSBF_BITFLAG value) {
    LS_BitFlags *obj = (LS_BitFlags *)lua_newuserdata(L, sizeof(LS_BitFlags));
    if (!obj) luaL_error(L, "Memory allocation failed");
    luaL_getmetatable(L, BITFLAGS_MT_NAME);
    lua_setmetatable(L, -2);
    obj->flags = value;
}

// gets the LS_BitFlags value at the given index. Returns a Lua error if it is not
// a LS_BitFlags object.
LSBF_BITFLAG lsbf_checkbitflags(lua_State *L, int index) {
    LS_BitFlags *obj = (LS_BitFlags *)luaL_checkudata(L, index, BITFLAGS_MT_NAME);
    return obj->flags;
}

/***
Creates a new bitflag object from the given value.
@function system.bitflag
@tparam[opt=0] number value the value to create the bitflag object from.
@treturn bitflag bitflag object with the given values set.
@usage
local sys = require 'system'
local flags = sys.bitflag(2)    -- b0010

-- get state of individual bits
print(flags[0])                 -- false
print(flags[1])                 -- true

-- set individual bits
flags[0] = true                 -- b0011
print(flags:value())            -- 3
print(flags)                    -- "bitflags: 3"

-- adding flags (bitwise OR)
local flags1 = sys.bitflag(1)   -- b0001
local flags2 = sys.bitflag(2)   -- b0010
local flags3 = flags1 + flags2  -- b0011

-- substracting flags (bitwise AND NOT)
print(flags3:value())           -- 3
flag3 = flag3 - flag3           -- b0000
print(flags3:value())           -- 0

-- comparing flags
local flags4 = sys.bitflag(7)   -- b0111
local flags5 = sys.bitflag(255) -- b11111111
print(flags5 >= flags4)         -- true, all bits in flags4 are set in flags5

-- comparing with 0 flags: comparison and `has` behave differently
local flags6 = sys.bitflag(0)   -- b0000
local flags7 = sys.bitflag(1)   -- b0001
print(flags6 < flags7)          -- true, flags6 is a subset of flags7
print(flags7:has(flags6))       -- false, flags6 is not set in flags7
*/
static int lsbf_new(lua_State *L) {
    LSBF_BITFLAG flags = 0;
    if (lua_gettop(L) > 0) {
        flags = luaL_checkinteger(L, 1);
    }
    lsbf_pushbitflags(L, flags);
    return 1;
}

/***
Retrieves the numeric value of the bitflag object.
@function bitflag:value
@treturn number the numeric value of the bitflags.
@usage
local sys = require 'system'
local flags = sys.bitflag()     -- b0000
flags[0] = true                 -- b0001
flags[2] = true                 -- b0101
print(flags:value())            -- 5
*/
static int lsbf_value(lua_State *L) {
    lua_pushinteger(L, lsbf_checkbitflags(L, 1));
    return 1;
}

static int lsbf_tostring(lua_State *L) {
    lua_pushfstring(L, "bitflags: %d", lsbf_checkbitflags(L, 1));
    return 1;
}

static int lsbf_add(lua_State *L) {
    lsbf_pushbitflags(L, lsbf_checkbitflags(L, 1) | lsbf_checkbitflags(L, 2));
    return 1;
}

static int lsbf_sub(lua_State *L) {
    lsbf_pushbitflags(L, lsbf_checkbitflags(L, 1) & ~lsbf_checkbitflags(L, 2));
    return 1;
}

static int lsbf_eq(lua_State *L) {
    lua_pushboolean(L, lsbf_checkbitflags(L, 1) == lsbf_checkbitflags(L, 2));
    return 1;
}

static int lsbf_le(lua_State *L) {
    LSBF_BITFLAG a = lsbf_checkbitflags(L, 1);
    LSBF_BITFLAG b = lsbf_checkbitflags(L, 2);
    // Check if all bits in b are also set in a
    lua_pushboolean(L, (a & b) == a);
    return 1;
}

/***
Checks if the given flags are set.
This is different from the `>=` and `<=` operators because if the flag to check
has a value `0`, it will always return `false`. So if there are flags that are
unsupported on a platform, they can be set to 0 and the `has` function will
return `false` if the flags are checked.
@function bitflag:has
@tparam bitflag subset the flags to check for.
@treturn boolean true if all the flags are set, false otherwise.
@usage
local sys = require 'system'
local flags = sys.bitflag(12)   -- b1100
local myflags = sys.bitflag(15) -- b1111
print(flags:has(myflags))       -- false, not all bits in myflags are set in flags
print(myflags:has(flags))       -- true, all bits in flags are set in myflags
*/
static int lsbf_has(lua_State *L) {
    LSBF_BITFLAG a = lsbf_checkbitflags(L, 1);
    LSBF_BITFLAG b = lsbf_checkbitflags(L, 2);
    // Check if all bits in b are also set in a, and b is not 0
    lua_pushboolean(L, (a | b) == a && b != 0);
    return 1;
}

static int lsbf_lt(lua_State *L) {
    LSBF_BITFLAG a = lsbf_checkbitflags(L, 1);
    LSBF_BITFLAG b = lsbf_checkbitflags(L, 2);
    // Check if a is strictly less than b, meaning a != b and a is a subset of b
    lua_pushboolean(L, (a != b) && ((a & b) == a));
    return 1;
}

static int lsbf_index(lua_State *L) {
    if (!lua_isnumber(L, 2)) {
        // the parameter isn't a number, just lookup the key in the metatable
        lua_getmetatable(L, 1);
        lua_pushvalue(L, 2);
        lua_gettable(L, -2);
        return 1;
    }

    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index >= sizeof(LSBF_BITFLAG) * 8) {
        return luaL_error(L, "index out of range");
    }
    lua_pushboolean(L, (lsbf_checkbitflags(L, 1) & (1 << index)) != 0);
    return 1;
}

static int lsbf_newindex(lua_State *L) {
    LS_BitFlags *obj = (LS_BitFlags *)luaL_checkudata(L, 1, BITFLAGS_MT_NAME);

    if (!lua_isnumber(L, 2)) {
        return luaL_error(L, "index must be a number");
    }
    int index = luaL_checkinteger(L, 2);
    if (index < 0 || index >= sizeof(LSBF_BITFLAG) * 8) {
        return luaL_error(L, "index out of range");
    }

    luaL_checkany(L, 3);
    if (lua_toboolean(L, 3)) {
        obj->flags |= (1 << index);
    } else {
        obj->flags &= ~(1 << index);
    }
    return 0;
}

static const struct luaL_Reg lsbf_funcs[] = {
    {"bitflag", lsbf_new},
    {NULL, NULL}
};

static const struct luaL_Reg lsbf_methods[] = {
    {"value", lsbf_value},
    {"has", lsbf_has},
    {"__tostring", lsbf_tostring},
    {"__add", lsbf_add},
    {"__sub", lsbf_sub},
    {"__eq", lsbf_eq},
    {"__le", lsbf_le},
    {"__lt", lsbf_lt},
    {"__index", lsbf_index},
    {"__newindex", lsbf_newindex},
    {NULL, NULL}
};

void bitflags_open(lua_State *L) {
    luaL_newmetatable(L, BITFLAGS_MT_NAME);
    luaL_setfuncs(L, lsbf_methods, 0);
    lua_pop(L, 1);

    luaL_setfuncs(L, lsbf_funcs, 0);
}
