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
      local expected_time = wait_for_second_rollover()
      local received_time = system.gettime()
      assert.is.near(expected_time, received_time, 0.02)

      wait_for_second_rollover()
      assert.is.near(1, system.gettime() - received_time, 0.02)
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
      local start_time = system.gettime()
      system.sleep(1, 1)
      local end_time = system.gettime()
      local elapsed_time = end_time - start_time
      assert.is.near(elapsed_time, 1, 0.01)
    end)


    it("should sleep for the specified time; fractional", function()
      local start_time = system.gettime()
      system.sleep(0.5, 1)
      local end_time = system.gettime()
      local elapsed_time = end_time - start_time
      assert.is.near(0.5, elapsed_time, 0.01)
    end)


    it("should return immediately for a non-positive sleep time", function()
      local start_time = system.gettime()
      system.sleep(-1)
      local end_time = system.gettime()
      local elapsed_time = end_time - start_time
      assert.is.near(elapsed_time, 0, 0.01)
    end)

  end)

end)
