local sys = require "system"

print [[

This example will draw a snake like spiral on the screen. Showing ANSI escape
codes for moving the cursor around.

]]

-- backup term settings with auto-restore on exit
sys.autotermrestore()

-- setup Windows console to handle ANSI processing
sys.setconsoleflags(io.stdout, sys.getconsoleflags(io.stdout) + sys.COF_VIRTUAL_TERMINAL_PROCESSING)

-- start drawing the spiral.
-- start from current pos, then right, then up, then left, then down, and again.
local x, y = 1, 1     -- current position
local dx, dy = 1, 0   -- direction after each step
local wx, wy = 30, 30 -- width and height of the room
local mx, my = 1, 1   -- margin

-- commands to move the cursor
local move_left = "\27[1D"
local move_right = "\27[1C"
local move_up = "\27[1A"
local move_down = "\27[1B"

-- create room: 30 empty lines
print(("\n"):rep(wy))
local move = move_right

while wx > 0 and wy > 0 do
  sys.sleep(0.01) -- slow down the drawing a little
  io.write("*" .. move_left .. move )
  io.flush()
  x = x + dx
  y = y + dy

  if x > wx and move == move_right then
    -- end of move right
    dx = 0
    dy = 1
    move = move_up
    wy = wy - 1
    my = my + 1
  elseif y > wy and move == move_up then
    -- end of move up
    dx = -1
    dy = 0
    move = move_left
    wx = wx - 1
    mx = mx + 1
  elseif x < mx and move == move_left then
    -- end of move left
    dx = 0
    dy = -1
    move = move_down
    wy = wy - 1
    my = my + 1
  elseif y < my and move == move_down then
    -- end of move down
    dx = 1
    dy = 0
    move = move_right
    wx = wx - 1
    mx = mx + 1
  end
end

io.write(move_down:rep(15))
print("\nDone!")
