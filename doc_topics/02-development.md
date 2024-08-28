# 2. Development

Some tests cannot be run in CI becasue they test system specifics that
simply do not exist in a CI environment. An example is the `isatty` function
Which cannot be set to return a truthy value in CI.

The tests concerned are all labelled with `#manual`. And in CI they will
be skipped because `--exclude-tags=manual` is being passed to the
`busted` command line.

Hence if tests like these are being added, then please ensure the tests
pass locally, and do not rely on CI only.
