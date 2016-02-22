local system = require 'system.core'

describe('Test time functions', function()
  it('gettime returns current time', function()
    local starttime = system.gettime()
    local expected = os.time()
    local endtime = system.gettime()
    local delta = endtime - starttime
    local avg = starttime + delta/2
    assert.is_true(expected >= math.floor(starttime))
    assert.is_true(expected <= math.ceil(endtime))
    assert.is_near(expected, avg, 1 + delta)
  end)

  it('sleep will wait for specified amount of time', function()
    local starttime = system.gettime()
    system.sleep(0.5)
    assert.is_near(0.5, system.gettime() - starttime, 0.1)
  end)
end)
