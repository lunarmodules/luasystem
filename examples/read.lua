local sys = require "system"

print [[

This example shows how to do a non-blocking read from the cli.

]]

-- setup Windows console to handle ANSI processing
local of_in = sys.getconsoleflags(io.stdin)
local of_out = sys.getconsoleflags(io.stdout)
sys.setconsoleflags(io.stdout, sys.getconsoleflags(io.stdout) + sys.COF_VIRTUAL_TERMINAL_PROCESSING)
sys.setconsoleflags(io.stdin, sys.getconsoleflags(io.stdin) + sys.CIF_VIRTUAL_TERMINAL_INPUT)

-- setup Posix terminal to use non-blocking mode, and disable line-mode
local of_attr = sys.tcgetattr(io.stdin)
local of_block = sys.getnonblock(io.stdin)
sys.setnonblock(io.stdin, true)
sys.tcsetattr(io.stdin, sys.TCSANOW, {
  lflag = of_attr.lflag - sys.L_ICANON - sys.L_ECHO, -- disable canonical mode and echo
})

-- cursor sequences
local get_cursor_pos = "\27[6n"



print("Press a key, or 'A' to get cursor position, 'ESC' to exit")
while true do
  local key, keytype

  -- wait for a key
  while not key do
    key, keytype = sys.readansi(math.huge)
  end

  if key == "A" then io.write(get_cursor_pos); io.flush() end

  -- check if we got a key or ANSI sequence
  if keytype == "key" then
    -- just a key
    local b = key:byte()
    if b < 32 then
      key = "." -- replace control characters with a simple "." to not mess up the screen
    end

    print("you pressed: " .. key .. " (" .. b .. ")")
    if b == 27 then
      print("Escape pressed, exiting")
      break
    end

  elseif keytype == "ansi" then
    -- we got an ANSI sequence
    local seq = { key:byte(1, #key) }
    print("ANSI sequence received: " .. key:sub(2,-1), "(bytes: " .. table.concat(seq, ", ")..")")

  else
    print("unknown key type received: " .. tostring(keytype))
  end
end



-- Clean up afterwards
sys.setnonblock(io.stdin, false)
sys.setconsoleflags(io.stdout, of_out)
sys.setconsoleflags(io.stdin, of_in)
sys.tcsetattr(io.stdin, sys.TCSANOW, of_attr)
sys.setnonblock(io.stdin, of_block)
