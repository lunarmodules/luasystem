local system = require 'system.core'



--- Debug function for console flags.
-- Pretty prints the current flags set for the handle.
-- @param fh file handle (`io.stdin`, `io.stdout`, `io.stderr`)
-- @usage -- Print the flags for stdin/out/err
-- system.listconsoleflags(io.stdin)
-- system.listconsoleflags(io.stdout)
-- system.listconsoleflags(io.stderr)
function system.listconsoleflags(fh)

  function bitand(a, b) -- Lua 5.1 has no native bitwise operations
    local result = 0
    local bitval = 1
    while a > 0 and b > 0 do
      if a % 2 == 1 and b % 2 == 1 then -- test the rightmost bits
          result = result + bitval      -- set the current bit
      end
      bitval = bitval * 2 -- shift left
      a = math.floor(a/2) -- shift right
      b = math.floor(b/2)
    end
    return result
  end

  local flagtype
  if fh == io.stdin then
    print "--- STDIN FLAGS ---"
    flagtype = "CIF_"
  elseif fh == io.stdout then
    print "--- STDOUT FLAGS ---"
    flagtype = "COF_"
  elseif fh == io.stderr then
    print "--- STDERR FLAGS ---"
    flagtype = "COF_"
  end

  local flags = assert(system.getconsoleflags(fh))
  local out = {}
  for k,v in pairs(system) do
    if type(k) == "string" and k:sub(1,4) == flagtype then
      if bitand(flags, v) ~= 0 then
        out[#out+1] = string.format("%3d [x] %s",v,k)
      else
        out[#out+1] = string.format("%3d [ ] %s",v,k)
      end
    end
  end
  table.sort(out)
  for k,v in pairs(out) do
    print(v)
  end
end



return system
