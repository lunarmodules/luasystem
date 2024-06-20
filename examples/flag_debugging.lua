local sys = require "system"

-- Print the Windows Console flags for stdin
sys.listconsoleflags(io.stdin)

-- Print the Posix termios flags for stdin
sys.listtermflags(io.stdin)
