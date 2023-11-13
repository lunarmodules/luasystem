# CHANGELOG

## Versioning

This library is versioned based on Semantic Versioning ([SemVer](https://semver.org/)).

#### Version scoping

The scope of what is covered by the version number excludes:

- error messages; the text of the messages can change, unless specifically documented.

#### Releasing new versions

- create a release branch
- update the changelog below
- update version and copyright-years in `./LICENSE.md` and `./src/time.c` (in module constants)
- create a new rockspec and update the version inside the new rockspec:<br/>
  `cp luasystem-scm-0.rockspec ./rockspecs/luasystem-X.Y.Z-1.rockspec`
- clean and render the docs: run `ldoc .`
- commit the changes as `Release vX.Y.Z`
- push the commit, and create a release PR
- after merging tag the release commit with `vX.Y.Z`
- upload to LuaRocks:<br/>
  `luarocks upload ./rockspecs/luasystem-X.Y.Z-1.rockspec --api-key=ABCDEFGH`
- test the newly created rock:<br/>
  `luarocks install luasystem`

## Version history

### Version X.Y.Z, unreleased

- Feat: on Windows `sleep` now has a precision parameter
- Feat: `setenv` added to set environment variables.
- Feat: `getenvs` added to list environment variables.
- Feat: `getenv` added to get environment variable previously set (Windows).
- Feat: `random` added to return high-quality random bytes
- Feat: `isatty` added to check if a file-handle is a tty

### Version 0.2.1, released 02-Oct-2016

### Version 0.2.0, released 08-May-2016

### Version 0.1.1, released 10-Apr-2016

### Version 0.1.0, released 11-Feb-2016

- initial release
