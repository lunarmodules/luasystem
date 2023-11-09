# CHANGELOG

## Versioning

This library is versioned based on Semantic Versioning ([SemVer](https://semver.org/)).

#### Version scoping

The scope of what is covered by the version number excludes:

- error messages; the text of the messages can change, unless specifically documented.

#### Releasing new versions

- create a release branch
- update the changelog below
- update version and copyright-years in `./LICENSE.md` and `./src/[module-name]/init.lua` (in doc-comments
  header, and in module constants)
- create a new rockspec and update the version inside the new rockspec:<br/>
  `cp [module-name]-scm-1.rockspec ./rockspecs/[module-name]-X.Y.Z-1.rockspec`
- test: run `make test` and `make lint`
- clean and render the docs: run `make clean` and `make docs`
- commit the changes as `release X.Y.Z`
- push the commit, and create a release PR
- after merging tag the release commit with `X.Y.Z`
- upload to LuaRocks:<br/>
  `luarocks upload ./rockspecs/[module-name]-X.Y.Z-1.rockspec --api-key=ABCDEFGH`
- test the newly created rock:<br/>
  `luarocks install [module-name]`

## Version history

### Version X.Y.Z, unreleased

- a fix
- a change

### Version 0.1.0, released 01-Jan-2022

- initial release
