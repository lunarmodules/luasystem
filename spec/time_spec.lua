local system = require 'system.core'

describe('Test time functions', function()
  it('gettime returns current time', function()
    assert.is_near(os.time(), system.gettime(), 1.0)
  end)

  it('sleep will wait for specified amount of time', function()
    local starttime = system.gettime()
    system.sleep(0.5)
    assert.is_near(0.5, system.gettime() - starttime, 0.1)
  end)
end)
