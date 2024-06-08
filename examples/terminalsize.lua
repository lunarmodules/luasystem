local sys = require("system")

sys.autotermrestore()  -- set up auto restore of terminal settings on exit

-- setup Windows console to handle ANSI processing
sys.setconsoleflags(io.stdout, sys.getconsoleflags(io.stdout) + sys.COF_VIRTUAL_TERMINAL_PROCESSING)
sys.setconsoleflags(io.stdin, sys.getconsoleflags(io.stdin) + sys.CIF_VIRTUAL_TERMINAL_INPUT)

-- setup Posix to disable canonical mode and echo
local of_attr = sys.tcgetattr(io.stdin)
sys.setnonblock(io.stdin, true)
sys.tcsetattr(io.stdin, sys.TCSANOW, {
  lflag = of_attr.lflag - sys.L_ICANON - sys.L_ECHO, -- disable canonical mode and echo
})


-- generate string to move cursor horizontally
-- positive goes right, negative goes left
local function cursor_move_horiz(n)
  if n == 0 then
    return ""
  end
  return "\27[" .. (n > 0 and n or -n) .. (n > 0 and "C" or "D")
end


local rows, cols
print("Change the terminal window size, press any key to exit")
while not sys.readansi(0.2) do  -- use readansi to not leave stray bytes in the input buffer
  local nrows, ncols = sys.termsize()
  if rows ~= nrows or cols ~= ncols then
    rows, cols = nrows, ncols
    local text = "Terminal size: " .. rows .. "x" .. cols .. "     "
    io.write(text .. cursor_move_horiz(-#text))
    io.flush()
  end
end
