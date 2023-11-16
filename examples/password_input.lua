local sys = require "system"

print [[

This example shows how to disable the "echo" of characters read to the console,
useful for reading secrets from the user.

]]

--- Function to read from stdin without echoing the input (for secrets etc).
-- It will (in a platform agnostic way) disable echo on the terminal, read the
-- input, and then re-enable echo.
-- @param ... Arguments to pass to `io.stdin:read()`
-- @return the results of `io.stdin:read(...)`
local function read_secret(...)
  local w_oldflags, p_oldflags

  if sys.isatty(io.stdin) then
    -- backup settings, configure echo flags
    w_oldflags = sys.getconsoleflags(io.stdin)
    p_oldflags = sys.tcgetattr(io.stdin)
    -- set echo off to not show password on screen
    assert(sys.setconsoleflags(io.stdin, w_oldflags - sys.CIF_ECHO_INPUT))
    assert(sys.tcsetattr(io.stdin, sys.TCSANOW, { lflag = p_oldflags.lflag - sys.L_ECHO }))
  end

  local secret, err = io.stdin:read(...)

  -- restore settings
  if sys.isatty(io.stdin) then
    io.stdout:write("\n")  -- Add newline after reading the password
    sys.setconsoleflags(io.stdin, w_oldflags)
    sys.tcsetattr(io.stdin, sys.TCSANOW, p_oldflags)
  end

  return secret, err
end



-- Get username
io.write("Username: ")
local username = io.stdin:read("*l")

-- Get the secret
io.write("Password: ")
local password = read_secret("*l")

-- Get domainname
io.write("Domain  : ")
local domain = io.stdin:read("*l")


-- Print the results
print("")
print("Here's what we got:")
print("  username: " .. username)
print("  password: " .. password)
print("  domain  : " .. domain)
