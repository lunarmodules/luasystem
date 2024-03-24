local busted = require("busted")

local function is_windows()
  return package.config:sub(1,1) == "\\"
end

local function postfixer(postfix)
  return function(description, ...)
    return busted.pending(description.." ["..postfix.."]", ...)
  end
end

-- win_it only executes on Windows, and is "pending" otherwise
win_it = is_windows() and busted.it or postfixer("Windows only")
-- nix_it only executes on Unix/Mac, and is "pending" otherwise
nix_it = is_windows() and postfixer("Unix/Mac only") or busted.it
