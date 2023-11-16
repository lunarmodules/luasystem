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



local read_input do
  local left_over_key

  -- Reads a single key, if it is a 27 (start of ansi escape sequence) then it reads
  -- the rest of the sequence.
  -- This function is non-blocking, and will return nil if no key is available.
  -- In case of an ANSI sequence, it will return the full sequence as a string.
  -- @return nil|string the key read, or nil if no key is available
  function read_input()
    if left_over_key then
      -- we still have a cached key, return it
      local key = left_over_key
      left_over_key = nil
      return string.char(key)
    end

    local key = sys.readkey()
    if key == nil then
      return nil
    end

    if key ~= 27 then
      return string.char(key)
    end

    -- looks like an ansi escape sequence, immediately read next char
    -- as an heuristic against manually typing escape sequences
    local brack = sys.readkey()
    if brack ~= 91 then
      -- not the expected [ character, so we return the key as is
      -- and store the extra key read for the next call
      left_over_key = brack
      return string.char(key)
    end

    -- escape sequence detected, read the rest of the sequence
    local seq = { key, brack }
    while true do
      key = sys.readkey()
      table.insert(seq, key)
      if (key >= 65 and key <= 90) or (key >= 97 and key <= 126) then
        -- end of sequence, return the full sequence
        return string.char((unpack or table.unpack)(seq))
      end
    end
    -- unreachable
  end
end



print("Press a key, or 'A' to get cursor position, 'ESC' to exit")
while true do
  local key

  -- wait for a key, and sleep a bit to not do a busy-wait
  while not key do
    key = read_input()
    if not key then sys.sleep(0.1) end
  end

  if key == "A" then io.write(get_cursor_pos); io.flush() end

  -- check if we got a key or ANSI sequence
  if #key == 1 then
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

  else
    -- we got an ANSI sequence
    local seq = { key:byte(1, #key) }
    print("ANSI sequence received: " .. key:sub(2,-1), "(bytes: " .. table.concat(seq, ", ")..")")
  end
end



-- Clean up afterwards
sys.setnonblock(io.stdin, false)
sys.setconsoleflags(io.stdout, of_out)
sys.setconsoleflags(io.stdin, of_in)
sys.tcsetattr(io.stdin, sys.TCSANOW, of_attr)
sys.setnonblock(io.stdin, of_block)
