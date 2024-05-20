-- Import the library that contains the environment-related functions
local system = require("system")
require("spec.helpers")

describe("Terminal:", function()

  local wincodepage

  setup(function()
    wincodepage = system.getconsoleoutputcp()
    assert(system.setconsoleoutputcp(65001))
  end)

  teardown(function()
    assert(system.setconsoleoutputcp(wincodepage))
  end)



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

    win_it("returns the consoleflags #manual", function()
      local flags, err = system.getconsoleflags(io.stdout)
      assert.is_nil(err)
      assert.is_userdata(flags)
      assert.equals("bitflags:", tostring(flags):sub(1,9))
    end)


    nix_it("returns the consoleflags, always value 0", function()
      local flags, err = system.getconsoleflags(io.stdout)
      assert.is_nil(err)
      assert.is_userdata(flags)
      assert.equals("bitflags:", tostring(flags):sub(1,9))
      assert.equals(0, flags:value())
    end)


    it("returns an error if called with an invalid argument", function()
      assert.has.error(function()
        system.getconsoleflags("invalid")
      end, "bad argument #1 to 'getconsoleflags' (FILE* expected, got string)")
    end)

  end)



  describe("setconsoleflags()", function()

    win_it("sets the consoleflags #manual", function()
      local old_flags = assert(system.getconsoleflags(io.stdout))
      finally(function()
        system.setconsoleflags(io.stdout, old_flags)
      end)

      local new_flags
      if old_flags:has_all_of(system.COF_VIRTUAL_TERMINAL_PROCESSING) then
        new_flags = old_flags - system.COF_VIRTUAL_TERMINAL_PROCESSING
      else
        new_flags = old_flags + system.COF_VIRTUAL_TERMINAL_PROCESSING
      end

      local success, err = system.setconsoleflags(io.stdout, new_flags)
      assert.is_nil(err)
      assert.is_true(success)

      local updated_flags = assert(system.getconsoleflags(io.stdout))
      assert.equals(new_flags:value(), updated_flags:value())
    end)


    nix_it("sets the consoleflags, always succeeds", function()
      assert(system.setconsoleflags(io.stdout, system.getconsoleflags(io.stdout)))
    end)


    it("returns an error if called with an invalid argument", function()
      assert.has.error(function()
        system.setconsoleflags("invalid")
      end, "bad argument #1 to 'setconsoleflags' (FILE* expected, got string)")
    end)

  end)



  pending("tcgetattr()", function()

    pending("sets the consoleflags, if called with flags", function()
    end)

  end)



  pending("tcsetattr()", function()

    pending("sets the consoleflags, if called with flags", function()
    end)

  end)



  pending("getconsolecp()", function()

    pending("sets the consoleflags, if called with flags", function()
    end)

  end)



  pending("setconsolecp()", function()

    pending("sets the consoleflags, if called with flags", function()
    end)

  end)



  pending("getconsoleoutputcp()", function()

    pending("sets the consoleflags, if called with flags", function()
    end)

  end)



  pending("setconsoleoutputcp()", function()

    pending("sets the consoleflags, if called with flags", function()
    end)

  end)



  pending("getnonblock()", function()

    pending("sets the consoleflags, if called with flags", function()
    end)

  end)



  pending("setnonblock()", function()

    pending("sets the consoleflags, if called with flags", function()
    end)

  end)



  pending("termsize()", function()

    pending("sets the consoleflags, if called with flags", function()
    end)

  end)



  describe("utf8cwidth()", function()

    local ch1 = string.char(226, 130, 172)       -- "€"   single
    local ch2 = string.char(240, 159, 154, 128)  -- "🚀"  double
    local ch3 = string.char(228, 189, 160)       -- "你"  double
    local ch4 = string.char(229, 165, 189)       -- "好"  double

    it("handles zero width characters", function()
      assert.same({0}, {system.utf8cwidth("")}) -- empty string returns 0-size
      assert.same({nil, 'Character width determination failed'}, {system.utf8cwidth("\a")})  -- bell character
      assert.same({nil, 'Character width determination failed'}, {system.utf8cwidth("\27")}) -- escape character
    end)

    it("handles single width characters", function()
      assert.same({1}, {system.utf8cwidth("a")})
      assert.same({1}, {system.utf8cwidth(ch1)})
    end)

    it("handles double width characters", function()
      assert.same({2}, {system.utf8cwidth(ch2)})
      assert.same({2}, {system.utf8cwidth(ch3)})
      assert.same({2}, {system.utf8cwidth(ch4)})
    end)

    it("returns the width of the first character in the string", function()
      assert.same({nil, 'Character width determination failed'}, {system.utf8cwidth("\a" .. ch1)})  -- bell character + EURO
      assert.same({1}, {system.utf8cwidth(ch1 .. ch2)})
      assert.same({2}, {system.utf8cwidth(ch2 .. ch3 .. ch4)})
    end)

  end)



  describe("utf8swidth()", function()

    local ch1 = string.char(226, 130, 172)       -- "€"   single
    local ch2 = string.char(240, 159, 154, 128)  -- "🚀"  double
    local ch3 = string.char(228, 189, 160)       -- "你"  double
    local ch4 = string.char(229, 165, 189)       -- "好"  double

    it("handles zero width characters", function()
      assert.same({0}, {system.utf8swidth("")}) -- empty string returns 0-size
      assert.same({nil, 'Character width determination failed'}, {system.utf8swidth("\a")})  -- bell character
      assert.same({nil, 'Character width determination failed'}, {system.utf8swidth("\27")}) -- escape character
    end)

    it("handles multi-character UTF8 strings", function()
      assert.same({15}, {system.utf8swidth("hello " .. ch1 .. ch2 .. " world")})
      assert.same({16}, {system.utf8swidth("hello " .. ch3 .. ch4 .. " world")})
    end)

  end)



  pending("termbackup()", function()

  end)



  pending("termrestore()", function()

  end)



  pending("termwrap()", function()

  end)



  pending("readkey()", function()

  end)



  pending("readansi()", function()

  end)

end)
