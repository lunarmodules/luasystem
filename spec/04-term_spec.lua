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
        system.setconsoleflags(io.stdout, old_flags)   -- ensure we restore the original ones
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



  describe("getconsolecp()", function()

    win_it("gets the console codepage", function()
      local cp, err = system.getconsolecp()
      assert.is_nil(err)
      assert.is_number(cp)
    end)

    nix_it("gets the console codepage, always 65001 (utf8)", function()
      local cp, err = system.getconsolecp()
      assert.is_nil(err)
      assert.equals(65001, cp)
    end)

  end)



  describe("setconsolecp()", function()

    win_it("sets the console codepage", function()
      local old_cp = assert(system.getconsolecp())
      finally(function()
        system.setconsolecp(old_cp)  -- ensure we restore the original one
      end)

      local new_cp
      if old_cp ~= 65001 then
        new_cp = 65001  -- set to UTF8
      else
        new_cp = 850    -- another common one
      end

      local success, err = system.setconsolecp(new_cp)
      assert.is_nil(err)
      assert.is_true(success)

      local updated_cp = assert(system.getconsolecp())
      assert.equals(new_cp, updated_cp)
    end)


    nix_it("sets the console codepage, always succeeds", function()
      assert(system.setconsolecp(850))
    end)


    it("returns an error if called with an invalid argument", function()
      assert.has.error(function()
        system.setconsolecp("invalid")
      end, "bad argument #1 to 'setconsolecp' (number expected, got string)")
    end)

  end)



  describe("getconsoleoutputcp()", function()

    win_it("gets the console output codepage", function()
      local cp, err = system.getconsoleoutputcp()
      assert.is_nil(err)
      assert.is_number(cp)
    end)

    nix_it("gets the console output codepage, always 65001 (utf8)", function()
      local cp, err = system.getconsoleoutputcp()
      assert.is_nil(err)
      assert.equals(65001, cp)
    end)

  end)



  describe("setconsoleoutputcp()", function()

    win_it("sets the console output codepage", function()
      local old_cp = assert(system.getconsoleoutputcp())
      finally(function()
        system.setconsoleoutputcp(old_cp)  -- ensure we restore the original one
      end)

      local new_cp
      if old_cp ~= 65001 then
        new_cp = 65001  -- set to UTF8
      else
        new_cp = 850    -- another common one
      end

      local success, err = system.setconsoleoutputcp(new_cp)
      assert.is_nil(err)
      assert.is_true(success)

      local updated_cp = assert(system.getconsoleoutputcp())
      assert.equals(new_cp, updated_cp)
    end)


    nix_it("sets the console output codepage, always succeeds", function()
      assert(system.setconsoleoutputcp(850))
    end)


    it("returns an error if called with an invalid argument", function()
      assert.has.error(function()
        system.setconsoleoutputcp("invalid")
      end, "bad argument #1 to 'setconsoleoutputcp' (number expected, got string)")
    end)

  end)



  describe("getnonblock()", function()

    nix_it("gets the non-blocking flag", function()
      local nb, err = system.getnonblock(io.stdin)
      assert.is_nil(err)
      assert.is_boolean(nb)
    end)

    win_it("gets the non-blocking flag, always false", function()
      local nb, err = system.getnonblock(io.stdin)
      assert.is_nil(err)
      assert.is_false(nb)
    end)

  end)



  describe("setnonblock()", function()

    nix_it("sets the non-blocking flag", function()
      local old_nb = system.getnonblock(io.stdin)
      assert.is.boolean(old_nb)

      finally(function()
        system.setnonblock(io.stdin, old_nb)  -- ensure we restore the original one
      end)

      local new_nb = not old_nb

      local success, err = system.setnonblock(io.stdin, new_nb)
      assert.is_nil(err)
      assert.is_true(success)

      local updated_nb = assert(system.getnonblock(io.stdin))
      assert.equals(new_nb, updated_nb)
    end)


    win_it("sets the non-blocking flag, always succeeds", function()
      local success, err = system.setnonblock(io.stdin, true)
      assert.is_nil(err)
      assert.is_true(success)
    end)


    it("returns an error if called with an invalid argument", function()
      assert.has.error(function()
        system.setnonblock("invalid")
      end, "bad argument #1 to 'setnonblock' (FILE* expected, got string)")
    end)

  end)



  describe("termsize() #manual", function()

    it("gets the terminal size", function()
      local w, h = system.termsize()
      assert.is_number(w)
      assert.is_number(h)
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



  describe("termbackup() & termrestore()", function()

    -- this is all Lua code, so testing one platform should be good enough
    win_it("creates and restores a backup", function()
      local backup = system.termbackup()

      local old_cp = assert(system.getconsoleoutputcp())
      finally(function()
        system.setconsoleoutputcp(old_cp)  -- ensure we restore the original one
      end)

      -- get the console page...
      local new_cp
      if old_cp ~= 65001 then
        new_cp = 65001  -- set to UTF8
      else
        new_cp = 850    -- another common one
      end

      -- change the console page...
      local success, err = system.setconsoleoutputcp(new_cp)
      assert.is_nil(err)
      assert.is_true(success)
      -- ... and check it
      local updated_cp = assert(system.getconsoleoutputcp())
      assert.equals(new_cp, updated_cp)

      -- restore the console page
      system.termrestore(backup)
      local restored_cp = assert(system.getconsoleoutputcp())
      assert.equals(old_cp, restored_cp)
    end)


    it("termrestore() fails on bad input", function()
      assert.has.error(function()
        system.termrestore("invalid")
      end, "arg #1 to termrestore, expected backup table, got string")
    end)

  end)



  describe("termwrap()", function()

    local old_backup
    local old_restore
    local result

    setup(function()
      old_backup = system.termbackup
      old_restore = system.termrestore
      system.termbackup = function()
        table.insert(result,"backup")
      end

      system.termrestore = function()
        table.insert(result,"restore")
      end
    end)


    before_each(function()
      result = {}
    end)


    teardown(function()
      system.termbackup = old_backup
      system.termrestore = old_restore
    end)



    it("calls both backup and restore", function()
      system.termwrap(function()
        table.insert(result,"wrapped")
      end)()

      assert.are.same({"backup", "wrapped", "restore"}, result)
    end)


    it("passes all args", function()
      system.termwrap(function(...)
        table.insert(result,{...})
      end)(1, 2, 3)

      assert.are.same({"backup", {1,2,3}, "restore"}, result)
    end)


    it("returns all results", function()
      local a, b, c = system.termwrap(function(...)
        return 1, nil, 3 -- ensure nil is passed as well
      end)()

      assert.are.same({"backup", "restore"}, result)
      assert.equals(1, a)
      assert.is_nil(b)
      assert.equals(3, c)
    end)

  end)



  describe("keyboard input", function()

    local old_readkey = system._readkey
    local current_buffer
    local function setbuffer(str)
      assert(type(str) == "string", "setbuffer() expects a string")
      if str == "" then
        current_buffer = nil
      else
        current_buffer = str
      end
    end


    setup(function()
      system._readkey = function()
        if not current_buffer then
          return nil
        end
        local ch = current_buffer:byte(1, 1)
        if #current_buffer == 1 then
          current_buffer = nil
        else
          current_buffer = current_buffer:sub(2, -1)
        end
        return ch
      end
    end)


    teardown(function()
      system._readkey = old_readkey
    end)



    describe("readkey()", function()

      it("fails without a timeout", function()
        assert.has.error(function()
          system.readkey()
        end, "arg #1 to readkey, expected timeout in seconds, got nil")
      end)


      it("reads a single byte for single-byte characters", function()
        setbuffer("abc")
        assert.equals(string.byte("a"), system.readkey(0))
        assert.equals(string.byte("b"), system.readkey(0))
        assert.equals(string.byte("c"), system.readkey(0))
      end)


      it("reads a single byte for multi-byte chars", function()
        setbuffer(string.char(226, 130, 172) ..        -- "€"   single
                  string.char(240, 159, 154, 128))     -- "🚀"  double
        assert.equals(226, system.readkey(0))
        assert.equals(130, system.readkey(0))
        assert.equals(172, system.readkey(0))
        assert.equals(240, system.readkey(0))
        assert.equals(159, system.readkey(0))
        assert.equals(154, system.readkey(0))
        assert.equals(128, system.readkey(0))
      end)


      it("times out", function()
        setbuffer("")
        local timing = system.gettime()
        assert.same({ nil, "timeout" }, { system.readkey(0.5) })

        timing = system.gettime() - timing
        -- assert.is.near(0.5, timing, 0.1)  -- this works in CI for Unix and Windows, but not MacOS (locally it passes)
        assert.is.near(1, timing, 0.5)       -- this also works for MacOS in CI
      end)

    end)



    describe("readansi()", function()

      it("fails without a timeout", function()
        assert.has.error(function()
          system.readansi()
        end, "arg #1 to readansi, expected timeout in seconds, got nil")
      end)


      it("reads a single byte for single-byte characters", function()
        setbuffer("abc")
        assert.are.same({"a", "char"}, {system.readansi(0)})
        assert.are.same({"b", "char"}, {system.readansi(0)})
        assert.are.same({"c", "char"}, {system.readansi(0)})
      end)


      it("reads a multi-byte characters one at a time", function()
        setbuffer(string.char(226, 130, 172) ..        -- "€"   single
                  string.char(240, 159, 154, 128))     -- "🚀"  double
        assert.are.same({"€", "char"}, {system.readansi(0)})
        assert.are.same({"🚀", "char"}, {system.readansi(0)})
      end)


      it("reads ANSI escape sequences, marked by '<esc>['", function()
        setbuffer("\27[A\27[B\27[C\27[D")
        assert.are.same({"\27[A", "ansi"}, {system.readansi(0)})
        assert.are.same({"\27[B", "ansi"}, {system.readansi(0)})
        assert.are.same({"\27[C", "ansi"}, {system.readansi(0)})
        assert.are.same({"\27[D", "ansi"}, {system.readansi(0)})
      end)


      it("reads ANSI escape sequences, marked by '<esc>O'", function()
        setbuffer("\27OA\27OB\27OC\27OD")
        assert.are.same({"\27OA", "ansi"}, {system.readansi(0)})
        assert.are.same({"\27OB", "ansi"}, {system.readansi(0)})
        assert.are.same({"\27OC", "ansi"}, {system.readansi(0)})
        assert.are.same({"\27OD", "ansi"}, {system.readansi(0)})
      end)


      it("returns a single <esc> character if no sequence is found", function()
        setbuffer("\27\27[A")
        assert.are.same({"\27", "char"}, {system.readansi(0)})
        assert.are.same({"\27[A", "ansi"}, {system.readansi(0)})
      end)


      it("times out", function()
        setbuffer("")
        local timing = system.gettime()
        assert.same({ nil, "timeout" }, { system.readansi(0.5) })

        timing = system.gettime() - timing
        -- assert.is.near(0.5, timing, 0.1)  -- this works in CI for Unix and Windows, but not MacOS (locally it passes)
        assert.is.near(1, timing, 0.5)       -- this also works for MacOS in CI
      end)

    end)

  end)

end)
