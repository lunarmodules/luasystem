-- sub-script executed for isatty test
local writefile = require("pl.utils").writefile
local isatty = require("system").isatty
assert(arg[1] == "--", "missing -- argument")
local tempfile = assert(arg[2], "missing tempfile argument")

-- print("my temp file: ", tempfile)

assert(writefile(tempfile, [[{
  stdin = ]]..tostring(isatty(io.stdin))..[[,
  stdout = ]]..tostring(isatty(io.stdout))..[[,
  stderr = ]]..tostring(isatty(io.stderr))..[[,
}]]))
