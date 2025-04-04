# 3. Terminal functionality

Terminals are fundamentally different on Windows and Posix. So even though
`luasystem` provides primitives to manipulate both the Windows and Posix terminals,
the user will still have to write platform specific code.

To mitigate this a little, all functions are available on all platforms. They just
will be a no-op if invoked on another platform. This means that no platform specific
branching is required (but still possible) in user code. The user must simply set
up both platforms to make it work.

## 3.1 Backup and Restore terminal settings

Since there are a myriad of settings available;

- `system.setconsoleflags` (Windows)
- `system.setconsolecp` (Windows)
- `system.setconsoleoutputcp` (Windows)
- `system.detachfds` (Posix)
- `system.setnonblock` (Posix)
- `system.tcsetattr` (Posix)

Some helper functions are available to backup and restore them all at once.
See `termbackup`, `termrestore`, `autotermrestore` and `termwrap`.


## 3.1 Terminal ANSI sequences

Windows is catching up with this. In Windows 10 (since 2019), the Windows Terminal application (not to be
mistaken for the `cmd` console application) supports ANSI sequences. However this
might not be enabled by default.

ANSI processing can be set up both on the input (key sequences, reading cursor position)
as well as on the output (setting colors and cursor shapes).

To enable it use `system.setconsoleflags` like this:

    -- setup Windows console to handle ANSI processing on output
    sys.setconsoleflags(io.stdout, sys.getconsoleflags(io.stdout) + sys.COF_VIRTUAL_TERMINAL_PROCESSING)
    sys.setconsoleflags(io.stderr, sys.getconsoleflags(io.stderr) + sys.COF_VIRTUAL_TERMINAL_PROCESSING)

    -- setup Windows console to handle ANSI processing on input
    sys.setconsoleflags(io.stdin, sys.getconsoleflags(io.stdin) + sys.CIF_VIRTUAL_TERMINAL_INPUT)


## 3.2 UTF-8 in/output and display width

### 3.2.1 UTF-8 in/output

Where (most) Posix systems use UTF-8 by default, Windows internally uses UTF-16. More
recent versions of Lua also have UTF-8 support. So `luasystem` also focusses on UTF-8.

On Windows UTF-8 output can be enabled by setting the output codepage like this:

    -- setup Windows output codepage to UTF-8
    sys.setconsoleoutputcp(sys.CODEPAGE_UTF8)

Terminal input is handled by the [`_getwchar()`](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/getchar-getwchar) function on Windows which returns
UTF-16 surrogate pairs. `luasystem` will automatically convert those to UTF-8.
So when using `readkey` or `readansi` to read keyboard input no additional changes
are required.

### 3.2.2 UTF-8 display width

Typical western characters and symbols are single width characters and will use only
a single column when displayed on a terminal. However many characters from other
languages/cultures or emojis require 2 columns for display.

Typically the `wcwidth` function is used on Posix to check the number of columns
required for display. However since Windows doesn't provide this functionality a
custom implementation is included based on [the work by Markus Kuhn](http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c).

2 functions are provided, `system.utf8cwidth` for a single character, and `system.utf8swidth` for
a string. When writing terminal applications the display width is relevant to
positioning the cursor properly. For an example see the [`examples/readline.lua`](../examples/readline.lua.html) file.


## 3.3 reading keyboard input

### 3.3.1 Non-blocking

There are 2 functions for keyboard input (actually 3, if taking `system._readkey` into
account): `readkey` and `readansi`.

`readkey` is a low level function and should preferably not be used, it returns
a byte at a time, and hence can leave stray/invalid byte sequences in the buffer if
only the start of a UTF-8 or ANSI sequence is consumed.

The preferred way is to use `readansi` which will parse and return entire characters in
single or multiple bytes, or a full ANSI sequence.

On Windows the input is read using [`_getwchar()`](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/getchar-getwchar) which bypasses the terminal and reads
the input directly from the keyboard buffer. This means however that the character is
also not being echoed to the terminal (independent of the echo settings used with
`system.setconsoleflags`).

On Posix the traditional file approach is used, which:

- is blocking by default
- echoes input to the terminal
- requires enter to be pressed to pass the input (canonical mode)

To use non-blocking input here's how to set it up:

    -- Detach stdin/out/err; to get their own independent file descriptions
    sys.detachfds()

    -- setup Windows console to disable echo and line input (not required since _getwchar is used, just for consistency)
    sys.setconsoleflags(io.stdin, sys.getconsoleflags(io.stdin) - sys.CIF_ECHO_INPUT - sys.CIF_LINE_INPUT)

    -- setup Posix by disabling echo, canonical mode, and making non-blocking
    local of_attr = sys.tcgetattr(io.stdin)
    sys.tcsetattr(io.stdin, sys.TCSANOW, {
      lflag = of_attr.lflag - sys.L_ICANON - sys.L_ECHO,
    })
    sys.setnonblock(io.stdin, true)


Both `readkey` and `readansi` require a timeout to be provided which allows for proper asynchronous
code to be written. The underlying sleep method to use can be provided, and defaults to `system.sleep`.
Just passing a coroutine enabled sleep method should be all that is needed to make
the result work with asynchroneous coroutine schedulers. Alternatively just patch `system.sleep`.

### 3.3.2 Blocking input

When using traditional input method like `io.stdin:read()` (which is blocking) the echo
and newline properties should be set on Windows similar to Posix.
For an example see [`examples/password_input.lua`](../examples/password_input.lua.html).
