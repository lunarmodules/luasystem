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

end)
