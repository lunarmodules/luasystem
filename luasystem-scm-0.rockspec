package = 'luasystem'
version = 'scm-0'
source = {
  url = "git://github.com/o-lim/luasystem",
  branch = "master"
}
description = {
  summary = 'Platform independent system calls for Lua.',
  detailed = [[
    Adds a Lua API for making platform independent system calls.
  ]],
  homepage = 'http://olivinelabs.com/luasystem/',
  license = 'MIT <http://opensource.org/licenses/MIT>'
}
dependencies = {
  'lua >= 5.1',
}

local function make_platform(plat)
  local defines = {
    unix = { },
    macosx = { },
    win32 = { },
    mingw32 = { "WINVER=0x0501" },
  }
  return {
    modules = {
      ['system.core'] = {
        sources = { 'src/core.c', 'src/compat.c', 'src/time.c', },
        defines = defines[plat],
      },
    },
  }
end

build = {
  type = 'builtin',
  platforms = {
    unix = make_platform('unix'),
    macosx = make_platform('macosx'),
    win32 = make_platform('win32'),
    mingw32 = make_platform('mingw32'),
  },
  modules = {
    ['system.init'] = 'system/init.lua',
  },
}
