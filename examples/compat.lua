-- This example shows how to remove platform differences to create a
-- cross-platform level playing field.

local sys = require "system"



if sys.is_windows then
  -- Windows holds multiple copies of environment variables, to ensure `getenv`
  -- returns what `setenv` sets we need to use the `system.getenv` instead of
  -- `os.getenv`.
  os.getenv = sys.getenv  -- luacheck: ignore

  -- Set up the terminal to handle ANSI escape sequences on Windows.
  if sys.isatty(io.stdout) then
    sys.setconsoleflags(io.stdout, sys.getconsoleflags(io.stdout) + sys.COF_VIRTUAL_TERMINAL_PROCESSING)
  end
  if sys.isatty(io.stderr) then
    sys.setconsoleflags(io.stderr, sys.getconsoleflags(io.stderr) + sys.COF_VIRTUAL_TERMINAL_PROCESSING)
  end
  if sys.isatty(io.stdin) then
    sys.setconsoleflags(io.stdin, sys.getconsoleflags(io.stdout) + sys.ENABLE_VIRTUAL_TERMINAL_INPUT)
  end


else
  -- On Posix, one can set a variable to an empty string, but on Windows, this
  -- will remove the variable from the environment. To make this consistent
  -- across platforms, we will remove the variable from the environment if the
  -- value is an empty string.
  local old_setenv = sys.setenv
  function sys.setenv(name, value)
    if value == "" then value = nil end
    return old_setenv(name, value)
  end
end

