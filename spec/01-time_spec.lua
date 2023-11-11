local system = require 'system.core'

describe('Test time functions', function()

  -- returns the new second, on the new second
  local function wait_for_second_rollover()
    local start_time = math.floor(os.time())
    local end_time = math.floor(os.time())
    while end_time == start_time do
      end_time = math.floor(os.time())
    end
    return end_time
  end


  describe("time()", function()

    it('returns current time', function()
      wait_for_second_rollover()
      local lua_time = wait_for_second_rollover()
      local aaa_time = system.time()
      assert.is.near(lua_time, aaa_time, 0.01)

      wait_for_second_rollover()
      assert.is.near(1, system.time() - aaa_time, 0.01)
    end)

  end)



  describe("monotime()", function()

    it('returns monotonically increasing time', function()
      local starttime = system.monotime()
      local endtime = system.monotime()
      local delta = endtime - starttime
      assert.is_true(starttime > 0)
      assert.is_true(delta >= 0)
      assert.is_true(system.monotime() - endtime >= 0)
    end)

  end)



  describe("sleep()", function()

    it("should sleep for the specified time", function()
      local start_time = wait_for_second_rollover()
      system.sleep(1)
      local end_time = os.time()
      local elapsed_time = end_time - start_time
      assert.is.near(elapsed_time, 1, 0.01)
    end)


    it("should sleep for the specified time; fractional", function()
      local start_time = system.time()
      system.sleep(0.5)
      local end_time = system.time()
      local elapsed_time = end_time - start_time
      assert.is.near(0.5, elapsed_time, 0.01)
    end)


    it("should return immediately for a non-positive sleep time", function()
      local start_time = wait_for_second_rollover()
      system.sleep(-1)
      local end_time = os.time()
      local elapsed_time = end_time - start_time
      assert.is.near(elapsed_time, 0, 0.001)
    end)

  end)

end)
