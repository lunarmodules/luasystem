local sys = require("system")

print [[

An example to display a spinner, whilst a long running task executes.

]]


-- start make backup, to auto-restore on exit
sys.autotermrestore()
-- configure console
sys.setconsoleflags(io.stdin, sys.getconsoleflags(io.stdin) - sys.CIF_ECHO_INPUT - sys.CIF_LINE_INPUT)
local of = sys.tcgetattr(io.stdin)
sys.tcsetattr(io.stdin, sys.TCSANOW, { lflag = of.lflag - sys.L_ICANON - sys.L_ECHO })
sys.setnonblock(io.stdin, true)



local function hideCursor()
  io.write("\27[?25l")
  io.flush()
end

local function showCursor()
  io.write("\27[?25h")
  io.flush()
end

local function left(n)
  io.write("\27[",n or 1,"D")
  io.flush()
end



local spinner do
  local spin = [[|/-\]]
  local i = 1
  spinner = function()
    hideCursor()
    io.write(spin:sub(i, i))
    left()
    i = i + 1
    if i > #spin then i = 1 end

    if sys.keypressed() then
      sys.readkey() -- consume key pressed
      io.write(" ");
      left()
      showCursor()
      return true
    else
      return false
    end
  end
end

io.stdout:write("press any key to stop the spinner... ")
while not spinner() do
  sys.sleep(0.1)
end

print("Done!")
