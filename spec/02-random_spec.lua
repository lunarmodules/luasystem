local system = require("system")

describe("Random:", function()

  describe("random()", function()

    it("should return random bytes for a valid number of bytes", function()
        local num_bytes = 1
        local result, err_msg = system.random(num_bytes)
        assert.is_nil(err_msg)
        assert.is.string(result)
        assert.is_equal(num_bytes, #result)
    end)


    it("should return an empty string for 0 bytes", function()
        local num_bytes = 0
        local result, err_msg = system.random(num_bytes)
        assert.is_nil(err_msg)
        assert.are.equal("", result)
    end)


    it("should return an error message for an invalid number of bytes", function()
        local num_bytes = -1
        local result, err_msg = system.random(num_bytes)
        assert.is.falsy(result)
        assert.are.equal("invalid number of bytes, must not be less than 0", err_msg)
    end)


    it("should not return duplicate results", function()
      local num_bytes = 1025
      local result1, err_msg = system.random(num_bytes)
      assert.is_nil(err_msg)
      assert.is.string(result1)

      local result2, err_msg = system.random(num_bytes)
      assert.is_nil(err_msg)
      assert.is.string(result2)

      assert.is_not.equal(result1, result2)
    end)

  end)



  describe("rnd()", function()

    local has_math_type = type(math.type) == "function"

    it("with no args returns a number in [0, 1)", function()
      local v, err = system.rnd()
      assert.is_nil(err)
      assert.is_number(v)
      assert.is_true(v >= 0 and v < 1)
    end)


    it("with no args returns different values on multiple calls", function()
      local seen = {}
      for _ = 1, 20 do
        local v = system.rnd()
        seen[v] = true
      end
      local count = 0
      for _ in pairs(seen) do count = count + 1 end
      assert.is_true(count >= 2, "expected at least 2 distinct values in 20 calls")
    end)


    it("with one arg m returns integer in [1, m]", function()
      for _ = 1, 50 do
        local v, err = system.rnd(6)
        assert.is_nil(err)
        assert.is_true(v >= 1 and v <= 6 and math.floor(v) == v)
      end
    end)


    it("matches math.random type behaviour for one-arg m when available", function()
      if not has_math_type then
        return
      end
      -- use math.random as ground truth for numeric type
      local mval = math.random(6)
      local rval = system.rnd(6)
      assert.are.equal(math.type(mval), math.type(rval))
    end)


    it("with one arg 1 always returns 1", function()
      for _ = 1, 10 do
        local v, err = system.rnd(1)
        assert.is_nil(err)
        assert.are.equal(1, v)
      end
    end)


    it("with one arg 0 returns a full-range integer", function()
      local v, err = system.rnd(0)
      assert.is_nil(err)
      assert.is_true(type(v) == "number" or (math.type and math.type(v) == "integer"))
      assert.is_true(v >= -0x8000000000000000 and v <= 0x7fffffffffffffff)
    end)


    it("matches math.random type behaviour for arg 0 when math.random(0) is supported", function()
      if not has_math_type then
        return
      end
      local ok, mval = pcall(math.random, 0)
      if not ok then
        return -- older Lua where math.random(0) is not supported
      end
      local rval = system.rnd(0)
      assert.are.equal(math.type(mval), math.type(rval))
    end)


    it("with two args returns integer in [m, n]", function()
      for _ = 1, 30 do
        local v, err = system.rnd(10, 20)
        assert.is_nil(err)
        assert.is_true(v >= 10 and v <= 20 and math.floor(v) == v)
      end
    end)


    it("matches math.random type behaviour for two-arg range when available", function()
      if not has_math_type then
        return
      end
      local mval = math.random(10, 20)
      local rval = system.rnd(10, 20)
      assert.are.equal(math.type(mval), math.type(rval))
    end)


    it("with two args supports negative range", function()
      for _ = 1, 30 do
        local v, err = system.rnd(-5, 5)
        assert.is_nil(err)
        assert.is_true(v >= -5 and v <= 5 and math.floor(v) == v)
      end
    end)


    it("with two equal args returns that value", function()
      local v, err = system.rnd(7, 7)
      assert.is_nil(err)
      assert.are.equal(7, v)
    end)


    it("throws for empty interval (m > n), like math.random", function()
      local ok_math, _ = pcall(math.random, 10, 5)
      assert.is_falsy(ok_math, "math.random(10, 5) should error")
      local ok_rnd, _ = pcall(system.rnd, 10, 5)
      assert.is_falsy(ok_rnd, "rnd(10, 5) should throw like math.random")
    end)


    it("throws for invalid one-arg (m < 1, m ~= 0), like math.random", function()
      local ok_math, _ = pcall(math.random, -1)
      assert.is_falsy(ok_math, "math.random(-1) should error")
      local ok_rnd, _ = pcall(system.rnd, -1)
      assert.is_falsy(ok_rnd, "rnd(-1) should throw like math.random")
    end)


    it("throws for wrong number of arguments, like math.random", function()
      local ok_math, _ = pcall(math.random, 1, 2, 3)
      assert.is_falsy(ok_math, "math.random(1, 2, 3) should error")
      local ok_rnd, _ = pcall(system.rnd, 1, 2, 3)
      assert.is_falsy(ok_rnd, "rnd(1, 2, 3) should throw like math.random")
    end)

  end)

end)
