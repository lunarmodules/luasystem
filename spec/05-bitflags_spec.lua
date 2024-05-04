describe("BitFlags library", function()

  local sys = require("system")

  it("creates new flag objects", function()
    local bf = sys.bitflag(255)
    assert.is_not_nil(bf)
    assert.are.equal(255, bf:value())
    assert.is.userdata(bf)
  end)

  it("converts to a hex string", function()
    local bf = sys.bitflag(255)
    assert.are.equal("bitflags: 255", tostring(bf))
  end)

  it("handles OR/ADD operations", function()
    -- one at a time
    local bf1 = sys.bitflag(1)    -- b0001
    local bf2 = sys.bitflag(2)    -- b0010
    local bf3 = bf1 + bf2         -- b0011
    assert.are.equal(3, bf3:value())
    -- multiple at once
    local bf4 = sys.bitflag(4+8)  -- b1100
    local bf5 = bf3 + bf4         -- b1111
    assert.are.equal(15, bf5:value())
    -- multiple that were already set
    local bf6 = sys.bitflag(15)   -- b1111
    local bf7 = sys.bitflag(8+2)  -- b1010
    local bf8 = bf6 + bf7         -- b1111
    assert.are.equal(15, bf8:value())
  end)

  it("handles AND-NOT/SUBSTRACT operations", function()
    -- one at a time
    local bf1 = sys.bitflag(3)    -- b0011
    local bf2 = sys.bitflag(1)    -- b0001
    local bf3 = bf1 - bf2         -- b0010
    assert.are.equal(2, bf3:value())
    -- multiple at once
    local bf4 = sys.bitflag(15)   -- b1111
    local bf5 = sys.bitflag(8+2)  -- b1010
    local bf6 = bf4 - bf5         -- b0101
    assert.are.equal(5, bf6:value())
    -- multiple that were not set
    local bf7 = sys.bitflag(3)    -- b0011
    local bf8 = sys.bitflag(15)   -- b1111
    local bf9 = bf7 - bf8         -- b0000
    assert.are.equal(0, bf9:value())
  end)

  it("checks for equality", function()
    local bf1 = sys.bitflag(4)
    local bf2 = sys.bitflag(4)
    local bf3 = sys.bitflag(5)
    assert.is.True(bf1 == bf2)
    assert.is.False(bf1 == bf3)
  end)

  it("indexes bits correctly", function()
    local bf = sys.bitflag(4)   -- b100
    assert.is_true(bf[2])
    assert.is_false(bf[1])
  end)

  it("errors on reading invalid bit indexes", function()
    local bf = sys.bitflag(4)
    assert.has_error(function() return bf[-10] end, "index out of range")
    assert.has_error(function() return bf[10000] end, "index out of range")
    assert.has_no_error(function() return bf.not_a_number end)
  end)

  it("sets and clears bits correctly", function()
    local bf = sys.bitflag(8)   -- b1000
    bf[1] = true
    assert.is_true(bf[1])       -- b1010
    assert.equals(10, bf:value())
    bf[1] = false
    assert.is_false(bf[1])      -- b1000
    assert.equals(8, bf:value())
  end)

  it("errors on setting invalid bit indexes", function()
    local bf = sys.bitflag(0)
    assert.has_error(function() bf[-10] = true end, "index out of range")
    assert.has_error(function() bf[10000] = true end, "index out of range")
    assert.has_error(function() bf.not_a_number = true end, "index must be a number")
  end)

  it("checks for a subset using 'has_all_of'", function()
    local bf1 = sys.bitflag(3)    -- b0011
    local bf2 = sys.bitflag(3)    -- b0011
    local bf3 = sys.bitflag(15)   -- b1111
    local bf0 = sys.bitflag(0)    -- b0000
    assert.is_true(bf1:has_all_of(bf2))  -- equal
    assert.is_true(bf3:has_all_of(bf1))  -- is a subset, and has more flags
    assert.is_false(bf1:has_all_of(bf3)) -- not a subset, bf3 has more flags
    assert.is_false(bf1:has_all_of(bf0)) -- bf0 is unset, always returns false
  end)

  it("checks for a subset using 'has_any_of'", function()
    local bf1 = sys.bitflag(3)    -- b0011
    local bf2 = sys.bitflag(3)    -- b0011
    local bf3 = sys.bitflag(7)    -- b0111
    local bf4 = sys.bitflag(8)    -- b1000
    local bf0 = sys.bitflag(0)    -- b0000
    assert.is_true(bf1:has_any_of(bf2))  -- equal
    assert.is_true(bf3:has_any_of(bf1))  -- is a subset, and has more flags
    assert.is_false(bf3:has_any_of(bf4)) -- no overlap in flags
    assert.is_true(bf1:has_any_of(bf3))  -- not a subset, bf3 has more flags but still some overlap
    assert.is_false(bf1:has_all_of(bf0)) -- bf0 is unset, always returns false
  end)

end)
