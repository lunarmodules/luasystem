# 1. Introduction

luasystem is a platform independent system call library for Lua.
Supports Unix, Windows, MacOS, `Lua >= 5.1` and `luajit >= 2.0.0`.

Lua is typically platform independent, but it requires adhering to very old C
standards. This in turn means that many common features (according to todays standards)
are not available. This module attempts to overcome some of those hurdles by providing
functions that cover those common needs.

This is not a kitchen sink library, but a minimalistic one with a focus on platform
independence.
