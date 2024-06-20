#ifndef LSBITFLAGS_H
#define LSBITFLAGS_H

#include <lua.h>
#include "compat.h"
#include <lauxlib.h>
#include <stdlib.h>

// type used to store the bitflags
#define LSBF_BITFLAG lua_Integer

// Validates that the given index is a bitflag object and returns its value.
// If the index is not a bitflag object, a Lua error is raised.
// The value will be left on the stack.
LSBF_BITFLAG lsbf_checkbitflags(lua_State *L, int index);


// Validates that the given index is a table containing a field 'fieldname'
// which is a bitflag object and returns its value.
// If the index is not a table or the field is not a bitflag object, a Lua
// error is raised. If the bitflag is not present, the default value is returned.
// The stack remains unchanged.
LSBF_BITFLAG lsbf_checkbitflagsfield(lua_State *L, int index, const char *fieldname, LSBF_BITFLAG default_value);


// Pushes a new bitflag object with the given value onto the stack.
// Might raise a Lua error if memory allocation fails.
void lsbf_pushbitflags(lua_State *L, LSBF_BITFLAG value);

#endif
