# busted_time makefile
#
# see csrc/makefile for description of how to customize the build
#
# Targets:
#   install            install system independent support
#   install-all        install for lua51 lua52 lua53
#   print              print the build settings

ifeq ($(origin PLAT),undefined)
UNAME_S:=$(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLAT=linux
endif
ifeq ($(UNAME_S),Darwin)
    PLAT=macosx
endif
ifeq ($(UNAME_S),FreeBSD)
    PLAT=freebsd
endif
ifeq ($(patsubst MINGW%,MINGW,$(UNAME_S)),MINGW)
    PLAT=mingw
endif
endif
PLAT?= linux
PLATS= macosx linux win32 mingw freebsd

all: $(PLAT)

$(PLATS) none install local clean:
	$(MAKE) -C src $@

print:
	$(MAKE) -C src $@

install-all:
	$(MAKE) clean
	@cd src && $(MAKE) $(PLAT) LUA_VERSION=5.1
	@cd src && $(MAKE) install LUA_VERSION=5.1
	$(MAKE) clean
	@cd src && $(MAKE) $(PLAT) LUA_VERSION=5.2
	@cd src && $(MAKE) install LUA_VERSION=5.2
	$(MAKE) clean
	@cd src && $(MAKE) $(PLAT) LUA_VERSION=5.3
	@cd src && $(MAKE) install LUA_VERSION=5.3

.PHONY: test
