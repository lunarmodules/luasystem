-- Import the library that contains the environment-related functions
local system = require("system")
require("spec.helpers")

describe("Terminal:", function()

  describe("isatty()", function()

    local newtmpfile = require("pl.path").tmpname

    -- set each param to true to make it a tty, to false for a stream
    local function getttyresults(sin, sout, serr)
      assert(type(sin) == "boolean", "sin must be a boolean")
      assert(type(sout) == "boolean", "sout must be a boolean")
      assert(type(serr) == "boolean", "serr must be a boolean")

      local tmpfile = "./spec/04-term_helper.output"
      local execcmd = "lua ./spec/04-term_helper.lua -- " .. tmpfile

      sin = sin and "" or 'echo "hello" | '
      if system.windows then
        sout = sout and "" or (" > " .. newtmpfile())
        serr = serr and "" or (" 2> " .. newtmpfile())
      else
        sout = sout and "" or (" > " .. newtmpfile())
        serr = serr and "" or (" 2> " .. newtmpfile())
      end

      local cmd = sin .. execcmd .. sout .. serr

      -- print("cmd: ", cmd)

      os.remove(tmpfile)
      assert(os.execute(cmd))
      local result = assert(require("pl.utils").readfile(tmpfile))
      os.remove(tmpfile)

      -- print("result: ", result)

      return assert(require("pl.compat").load("return " .. result))()
    end



    it("returns true for all if a terminal #manual", function()
      assert.are.same(
        {
          stdin = true,
          stdout = true,
          stderr = true,
        },
        getttyresults(true, true, true)
      )
    end)


    it("returns false for stdin if not a terminal #manual", function()
      assert.are.same(
        {
          stdin = false,
          stdout = true,
          stderr = true,
        },
        getttyresults(false, true, true)
      )
    end)


    it("returns false for stdout if not a terminal #manual", function()
      assert.are.same(
        {
          stdin = true,
          stdout = false,
          stderr = true,
        },
        getttyresults(true, false, true)
      )
    end)


    it("returns false for stderr if not a terminal #manual", function()
      assert.are.same(
        {
          stdin = true,
          stdout = true,
          stderr = false,
        },
        getttyresults(true, true, false)
      )
    end)

  end)



  describe("getconsoleflags()", function()

    pending("returns the consoleflags, if called without flags", function()
print"1"
package.loaded["system"] = nil
package.loaded["system.core"] = nil
print"2"
local system = require "system"
print"3"
for k,v in pairs(system) do print(k,v) end
for k,v in pairs(debug.getinfo(system.isatty)) do print(k,v) end

      local flags, err = system.getconsoleflags(io.stdin)
      assert.is_nil(err)
      assert.is_integer(flags)
    end)

  end)
end)
