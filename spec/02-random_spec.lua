local system = require("system")

describe("Random:", function()

  describe("random()", function()

    it("should return random bytes for a valid number of bytes", function()
        local num_bytes = system.MAX_RANDOM_BUFFER_SIZE
        local result, err_msg = system.random(num_bytes)
        assert.is_nil(err_msg)
        assert.is.string(result)
        assert.is_equal(num_bytes, #result)
    end)


    it("should return an error message for an invalid number of bytes", function()
        local num_bytes = 0
        local result, err_msg = system.random(num_bytes)
        assert.is.falsy(result)
        assert.are.equal("invalid number of bytes, must be between 1 and 1024", err_msg)
    end)


    it("should return an error message for exceeding the maximum buffer size", function()
        local num_bytes = system.MAX_RANDOM_BUFFER_SIZE + 1
        local result, err_msg = system.random(num_bytes)
        assert.is.falsy(result)
        assert.are.equal("invalid number of bytes, must be between 1 and 1024", err_msg)
    end)


    it("should not return duplicate results", function()
      local num_bytes = 10
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
