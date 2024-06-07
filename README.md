# best

The Best Library.

`best` is an "STL replacement" for C++20, in the spirit of projects like Abseil
and Folly. However, unlike those, it aims to _completely_ replace the C++
standard library and some of the language's own primitives.

The goal is not interop, nor backwards compatibility: it is total replacement of
40 years of tech debt. `best` is what you use if you must use C++, but really
hate the STL. Of course, you can use `best` alongside the standard library, and
`best` depends on it for many things. But `best` encourages you to use _its_
vocabulary types.

We will only support "reasonable" configurations: modern architectures, and
operating systems whose names are "Linux", "Darwin", and "WindowsNT". `best`
will be free-standing friendly where that is feasible.

`best` is not exception-friendly. Exceptions introduce performance
pessimizations and safe exception-safe code in C++ is difficult to write and
test. It will always require C++20 as a minimum, and will only ever support
recent Clang and GCC; if you need to build for windows, use clang-cl.

`best`'s design philosophy is to copy Rust where it makes sense and to apply
lessons learned from Abseil. Neither of these is obviously better, so we pick
the `best` designs from each.

This is the `best` library in spite of C++, not because of it.

## Building

`best` currently only supports building with Bazel 7. It bundles scripts under
`bazel/` for running Bazel via Bazelisk.

`best` will only have one source-of-truth build system. If you want to use
`best` with CMake or a similar build system, you are on your own.

## Organization

`best`'s code is organized into shallow subdirectories that capture different
pieces of functionality.

- `best/base` - Low-level portability utilities.
- `best/container` - Container types. We interpret any generic "wrapper" type as
  a container.
- `best/log` - Logging utilities.
- `best/meta` - Metaprogramming utilities.
- `best/string` - Text processing utilities. `best` is a Unicode-first library.
- `best/test` - A unit testing framework.

## Contributing + Style

Bugfixes welcome, but this library is still under construction.

The C++ code mostly follows
[Google's style guide](https://google.github.io/styleguide/cppguide.html), with
a few changes:

- The names of types and functions are in `snake_case`, following the STL.
- Non-template constants, and tests, are `PascalCase` rather than `kConst`; all
  other variables are `snake_case`.
- Type parameters are `PascalCase`.
- Documentation comments are Rust style. Header comments follow the include
  list.

## Legal

`best` is licensed Apache-2.0.