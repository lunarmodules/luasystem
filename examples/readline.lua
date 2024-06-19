--- An example class for reading a line of input from the user in a non-blocking way.
-- It uses ANSI escape sequences to move the cursor and handle input.
-- It can be used to read a line of input from the user, with a prompt.
-- It can handle double-width UTF-8 characters.
-- It can be used asynchroneously if `system.sleep` is patched to yield to a coroutine scheduler.

local sys = require("system")


-- Mapping of key-sequences to key-names
local key_names = {
  ["\27[C"] = "right",
  ["\27[D"] = "left",
  ["\127"] = "backspace",
  ["\27[3~"] = "delete",
  ["\27[H"] = "home",
  ["\27[F"] = "end",
  ["\27"] = "escape",
  ["\9"] = "tab",
  ["\27[Z"] = "shift-tab",
}

if sys.windows then
  key_names["\13"] = "enter"
else
  key_names["\10"] = "enter"
end


-- Mapping of key-names to key-sequences
local key_sequences = {}
for k, v in pairs(key_names) do
  key_sequences[v] = k
end


-- bell character
local function bell()
  io.write("\7")
  io.flush()
end


-- generate string to move cursor horizontally
-- positive goes right, negative goes left
local function cursor_move_horiz(n)
  if n == 0 then
    return ""
  end
  return "\27[" .. (n > 0 and n or -n) .. (n > 0 and "C" or "D")
end


-- -- generate string to move cursor vertically
-- -- positive goes down, negative goes up
-- local function cursor_move_vert(n)
--   if n == 0 then
--     return ""
--   end
--   return "\27[" .. (n > 0 and n or -n) .. (n > 0 and "B" or "A")
-- end


-- -- log to the line above the current line
-- local function log(...)
--   local arg = { n = select("#", ...), ...}
--   for i = 1, arg.n do
--     arg[i] = tostring(arg[i])
--   end
--   arg = " " .. table.concat(arg, " ") .. " "

--   io.write(cursor_move_vert(-1), arg, cursor_move_vert(1), cursor_move_horiz(-#arg))
-- end


-- UTF8 character size in bytes
-- @tparam number b the byte value of the first byte of a UTF8 character
local function utf8size(b)
  return b < 128 and 1 or b < 224 and 2 or b < 240 and 3 or b < 248 and 4
end



local utf8parse do
  local utf8_value_mt = {
    __tostring = function(self)
      return table.concat(self, "")
    end,
  }

  -- Parses a UTF8 string into list of individual characters.
  -- key 'chars' gets the length in UTF8 characters, whilst # returns the length
  -- for display (to handle double-width UTF8 chars).
  -- in the list the double-width characters are followed by an empty string.
  -- @tparam string s the UTF8 string to parse
  -- @treturn table the list of characters
  function utf8parse(s)
    local t = setmetatable({ chars = 0 }, utf8_value_mt)
    local i = 1
    while i <= #s do
      local b = s:byte(i)
      local w = utf8size(b)
      local char = s:sub(i, i + w - 1)
      t[#t + 1] = char
      t.chars = t.chars + 1
      if sys.utf8cwidth(char) == 2 then
        -- double width character, add empty string to keep the length of the
        -- list the same as the character width on screen
        t[#t + 1] = ""
      end
      i = i + w
    end
    return t
  end
end



-- inline tests for utf8parse
-- do
--   local t = utf8parse("a你b好c")
--   assert(t[1] == "a")
--   assert(t[2] == "你")  -- double width
--   assert(t[3] == "")
--   assert(t[4] == "b")
--   assert(t[5] == "好")  -- double width
--   assert(t[6] == "")
--   assert(t[7] == "c")
--   assert(#t == 7)       -- size as displayed
-- end



-- readline class

local readline = {}
readline.__index = readline


--- Create a new readline object.
-- @tparam table opts the options for the readline object
-- @tparam[opt=""] string opts.prompt the prompt to display
-- @tparam[opt=80] number opts.max_length the maximum length of the input (in characters, not bytes)
-- @tparam[opt=""] string opts.value the default value
-- @tparam[opt=`#value`] number opts.position of the cursor in the input
-- @tparam[opt={"\10"/"\13"}] table opts.exit_keys an array of keys that will cause the readline to exit
-- @treturn readline the new readline object
function readline.new(opts)
  local value = utf8parse(opts.value or "")
  local prompt = utf8parse(opts.prompt or "")
  local pos = math.floor(opts.position or (#value + 1))
  pos = math.max(math.min(pos, (#value + 1)), 1)
  local len = math.floor(opts.max_length or 80)
  if len < 1 then
    error("max_length must be at least 1", 2)
  end

  if value.chars > len then
    error("value is longer than max_length", 2)
  end

  local exit_keys = {}
  for _, key in ipairs(opts.exit_keys or {}) do
    exit_keys[key] = true
  end
  if exit_keys[1] == nil then
    -- nothing provided, default to Enter-key
    exit_keys[1] = key_sequences.enter
  end

  local self = {
    value = value,          -- the default value
    max_length = len,       -- the maximum length of the input
    prompt = prompt,        -- the prompt to display
    position = pos,         -- the current position in the input
    drawn_before = false,   -- if the prompt has been drawn
    exit_keys = exit_keys,  -- the keys that will cause the readline to exit
  }

  setmetatable(self, readline)
  return self
end



-- draw the prompt and the input value, and position the cursor.
local function draw(self, redraw)
  if redraw or not self.drawn_before then
    -- we are at start of prompt
    self.drawn_before = true
  else
    -- we are at current cursor position, move to start of prompt
    io.write(cursor_move_horiz(-(#self.prompt + self.position)))
  end
  -- write prompt & value
  io.write(tostring(self.prompt) .. tostring(self.value))
  -- clear remainder of input size
  io.write(string.rep(" ", self.max_length - self.value.chars))
  io.write(cursor_move_horiz(-(self.max_length - self.value.chars)))
  -- move to cursor position
  io.write(cursor_move_horiz(-(#self.value + 1 - self.position)))
  io.flush()
end


local handle_key do -- keyboard input handler

  local key_handlers
  key_handlers = {
    left = function(self)
      if self.position == 1 then
        bell()
        return
      end

      local new_pos = self.position - 1
      while self.value[new_pos] == "" do -- skip empty strings; double width chars
        new_pos = new_pos - 1
      end

      io.write(cursor_move_horiz(-(self.position - new_pos)))
      io.flush()
      self.position = new_pos
    end,

    right = function(self)
      if self.position == #self.value + 1 then
        bell()
        return
      end

      local new_pos = self.position + 1
      while self.value[new_pos] == "" do -- skip empty strings; double width chars
        new_pos = new_pos + 1
      end

      io.write(cursor_move_horiz(new_pos - self.position))
      io.flush()
      self.position = new_pos
    end,

    backspace = function(self)
      if self.position == 1 then
        bell()
        return
      end

      while self.value[self.position - 1] == "" do -- remove empty strings; double width chars
        io.write(cursor_move_horiz(-1))
        self.position = self.position - 1
        table.remove(self.value, self.position)
      end
      -- remove char itself
      io.write(cursor_move_horiz(-1))
      self.position = self.position - 1
      table.remove(self.value, self.position)
      self.value.chars = self.value.chars - 1
      draw(self)
    end,

    home = function(self)
      local new_pos = 1
      io.write(cursor_move_horiz(new_pos - self.position))
      self.position = new_pos
    end,

    ["end"] = function(self)
      local new_pos = #self.value + 1
      io.write(cursor_move_horiz(new_pos - self.position))
      self.position = new_pos
    end,

    delete = function(self)
      if self.position > #self.value then
        bell()
        return
      end

      key_handlers.right(self)
      key_handlers.backspace(self)
    end,
  }


  -- handles a single input key/ansi-sequence.
  -- @tparam string key the key or ansi-sequence (from `system.readansi`)
  -- @tparam string keytype the type of the key, either "char" or "ansi" (from `system.readansi`)
  -- @treturn string status the status of the key handling, either "ok", "exit_key" or an error message
  function handle_key(self, key, keytype)
    if self.exit_keys[key] then
      -- registered exit key
      return "exit_key"
    end

    local handler = key_handlers[key_names[key] or true ]
    if handler then
      handler(self)
      return "ok"
    end

    if keytype == "ansi" then
      -- we got an ansi sequence, but dunno how to handle it, ignore
      -- print("unhandled ansi: ", key:sub(2,-1), string.byte(key, 1, -1))
      bell()
      return "ok"
    end

    -- just a single key
    if key < " " then
      -- control character
      bell()
      return "ok"
    end

    if self.value.chars >= self.max_length then
      bell()
      return "ok"
    end

    -- insert the key into the value
    if sys.utf8cwidth(key) == 2 then
      -- double width character, insert empty string after it
      table.insert(self.value, self.position, "")
      table.insert(self.value, self.position, key)
      self.position = self.position + 2
      io.write(cursor_move_horiz(2))
    else
      table.insert(self.value, self.position, key)
      self.position = self.position + 1
      io.write(cursor_move_horiz(1))
    end
    self.value.chars = self.value.chars + 1
    draw(self)
    return "ok"
  end
end



--- Get_size returns the maximum size of the input box (prompt + input).
-- The size is in rows and columns. Columns is determined by
-- the prompt and the `max_length * 2` (characters can be double-width).
-- @treturn number the number of rows (always 1)
-- @treturn number the number of columns
function readline:get_size()
  return 1, #self.prompt + self.max_length * 2
end



--- Get coordinates of the cursor in the input box (prompt + input).
-- The coordinates are 1-based. They are returned as row and column, within the
-- size as reported by `get_size`.
-- @treturn number the row of the cursor (always 1)
-- @treturn number the column of the cursor
function readline:get_cursor()
  return 1, #self.prompt + self.position
end



--- Set the coordinates of the cursor in the input box (prompt + input).
-- The coordinates are 1-based. They are expected to be within the
-- size as reported by `get_size`, and beyond the prompt.
-- If the position is invalid, it will be corrected.
-- Use the results to check if the position was adjusted.
-- @tparam number row the row of the cursor (always 1)
-- @tparam number col the column of the cursor
-- @return results of get_cursor
function readline:set_cursor(row, col)
  local l_prompt = #self.prompt
  local l_value = #self.value

  if col < l_prompt + 1 then
    col = l_prompt + 1
  elseif col > l_prompt + l_value + 1 then
    col = l_prompt + l_value + 1
  end

  while self.value[col - l_prompt] == "" do
    col = col - 1 -- on an empty string, so move back to start of double-width char
  end

  local new_pos = col - l_prompt

  cursor_move_horiz(self.position - new_pos)
  io.flush()

  self.position = new_pos
  return self:get_cursor()
end



--- Read a line of input from the user.
-- It will first print the `prompt` and then wait for input. Ensure the cursor
-- is at the correct position before calling this function. This function will
-- do all cursor movements in a relative way.
-- Can be called again after an exit-key or timeout has occurred. Just make sure
-- the cursor is at the same position where is was when it returned the last time.
-- Alternatively the cursor can be set to the position of the prompt (the position
-- the cursor was in before the first call), and the parameter `redraw` can be set
-- to `true`.
-- @tparam[opt=math.huge] number timeout the maximum time to wait for input in seconds
-- @tparam[opt=false] boolean redraw if `true` the prompt will be redrawn (cursor must be at prompt position!)
-- @treturn[1] string the input string as entered the user
-- @treturn[1] string the exit-key used to exit the readline (see `new`)
-- @treturn[2] nil when input is incomplete
-- @treturn[2] string error message, the reason why the input is incomplete, `"timeout"`, or an error reading a key
function readline:__call(timeout, redraw)
  draw(self, redraw)
  timeout = timeout or math.huge
  local timeout_end = sys.gettime() + timeout

  while true do
    local key, keytype = sys.readansi(timeout_end - sys.gettime())
    if not key then
      -- error or timeout
      return nil, keytype
    end

    local status = handle_key(self, key, keytype)
    if status == "exit_key" then
      return tostring(self.value), key

    elseif status ~= "ok" then
      error("unknown status received: " .. tostring(status))
    end
  end
end



-- return readline  -- normally we'd return here, but for the example we continue




local backup = sys.termbackup()

-- setup Windows console to handle ANSI processing
sys.setconsoleflags(io.stdout, sys.getconsoleflags(io.stdout) + sys.COF_VIRTUAL_TERMINAL_PROCESSING)
sys.setconsoleflags(io.stdin, sys.getconsoleflags(io.stdin) + sys.CIF_VIRTUAL_TERMINAL_INPUT)
-- set output to UTF-8
sys.setconsoleoutputcp(sys.CODEPAGE_UTF8)

-- setup Posix terminal to disable canonical mode and echo
sys.tcsetattr(io.stdin, sys.TCSANOW, {
  lflag = sys.tcgetattr(io.stdin).lflag - sys.L_ICANON - sys.L_ECHO,
})
-- setup stdin to non-blocking mode
sys.setnonblock(io.stdin, true)


local rl = readline.new{
  prompt = "Enter something: ",
  max_length = 60,
  value = "Hello, 你-好 World 🚀!",
  -- position = 2,
  exit_keys = {key_sequences.enter, "\27", "\t", "\27[Z"}, -- enter, escape, tab, shift-tab
}


local result, key = rl()
print("")  -- newline after input, to move cursor down from the input line
print("Result (string): '" .. result .. "'")
print("Result (bytes):", result:byte(1,-1))
print("Exit-Key (bytes):", key:byte(1,-1))


-- Clean up afterwards
sys.termrestore(backup)
