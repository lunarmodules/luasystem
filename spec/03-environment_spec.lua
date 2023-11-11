-- Import the library that contains the environment-related functions
local system = require("system")

describe("Environment Variables:", function()

  describe("setenv()", function()

    it("should set an environment variable", function()
      assert.is_true(system.setenv("TEST_VAR", "test_value"))
      assert.is_equal("test_value", os.getenv("TEST_VAR"))
    end)


    it("should set an empty environment variable value", function()
      assert.is_true(system.setenv("TEST_VAR", ""))
      assert.is_equal("", os.getenv("TEST_VAR"))
    end)


    it("should unset an environment variable on nil", function()
      assert.is_true(system.setenv("TEST_VAR", "test_value"))
      assert.is_equal("test_value", os.getenv("TEST_VAR"))

      assert.is_true(system.setenv("TEST_VAR", nil))
      assert.is_nil(os.getenv("TEST_VAR"))
    end)


    it("should error on input bad type", function()
      assert.has_error(function()
        system.setenv("TEST_VAR", {})
      end)
      assert.has_error(function()
        system.setenv({}, "test_value")
      end)
    end)


    it("should return success on deleting a variable that doesn't exist", function()
      if os.getenv("TEST_VAR") ~= nil then
        -- clear if it was already set
        assert.is_true(system.setenv("TEST_VAR", nil))
      end

      assert.is_true(system.setenv("TEST_VAR", nil)) -- clear again shouldn't fail
    end)

  end)



  describe("getenvs()", function()

    it("should list environment variables", function()
      assert.is_true(system.setenv("TEST_VAR1", nil))
      assert.is_true(system.setenv("TEST_VAR2", nil))
      assert.is_true(system.setenv("TEST_VAR3", nil))
      local envVars1 = system.getenvs()
      assert.is_true(system.setenv("TEST_VAR1", "test_value1"))
      assert.is_true(system.setenv("TEST_VAR2", "test_value2"))
      assert.is_true(system.setenv("TEST_VAR3", "test_value3"))
      local envVars2 = system.getenvs()
      assert.is_true(system.setenv("TEST_VAR1", nil))
      assert.is_true(system.setenv("TEST_VAR2", nil))
      assert.is_true(system.setenv("TEST_VAR3", nil))

      for k,v in pairs(envVars1) do
        envVars2[k] = nil
      end
      assert.are.same({
        TEST_VAR1 = "test_value1",
        TEST_VAR2 = "test_value2",
        TEST_VAR3 = "test_value3",
      }, envVars2)
    end)

  end)

end)
