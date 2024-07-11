# best

The Best C++ Library

`best` is an "STL replacement" for C++20, in the spirit of projects like Abseil
and Folly. However, unlike those, it aims to _completely_ replace the C++
standard library and some of the language's own primitives.

## Goals and Design Philosophy

Our goal is not interop, nor backwards compatibility: it is total replacement of
40 years of tech debt. `best` is what you use if you must use C++, but really
hate the STL. Of course, you can use `best` alongside the standard library, and
`best` depends on it for many things. But `best` encourages you to use _its_
vocabulary types: you should pick the "`best` solution".

We will only support "reasonable" configurations: modern architectures, and
operating systems whose names are "Linux", "Darwin", and "WindowsNT". `best`
will be free-standing friendly where that is feasible.

`best` is not exception-friendly. Exceptions introduce performance
pessimizations and safe exception-safe code in C++ is difficult to write and
test. It will always require C++20 as a minimum, and will only ever support
recent Clang and GCC; if you need to build for windows, use clang-cl.

> NB: CI currently only tests recent Clang on `x86_64-unknown-linux-glibc`. If
> you want to bring up a new, reasonable platform, you should make sure to turn
> on some kind of CI support.

The essence of `best`'s design philosophy is to copy Rust where it makes sense
and to apply lessons learned from Abseil. Neither of these is obviously better,
so we pick the `best` designs from each.

This is the `best` library in spite of C++, not because of it.

## Name

The library is called `best`, styled in monospace. You do not need to write
`` `best` `` in contexts where backticks are not interpreted as monospace
markup.

The name comes from an unofficial synonym for Abseil's namespace, `absl`: "A
Better Standard Library". Because much of this library is a response to Abseil,
it feels right that it should be better than "better", and therefore `best`.

The `best` name is also an excellent opportunity for puns, and the names of
important types and functions are chosen so that `best::blah` reads well. For
example, `best::option`, `best::result`, and `best::choice` are all named with
this convention in mind.

The `best` name is not intended to be braggadocious. I just think it's fun and
punny.

## Building

`best` currently only supports building with Bazel 7. It includes a script,
`./bazel`, that will download Bazelisk, and then download Bazel, and run that,
ensuring the only build dependency is Bash. Bazel will download and install a
hermetic Clang toolchain.

`best` will only have one source-of-truth build system. If you want to use
`best` with CMake or a similar build system, you are on your own.

To run all of `best`'s tests, simply run the following; you should not need to
download any dependencies.

```sh
./bazel test //...
```

## Organization

`best`'s code is organized into shallow subdirectories that capture different
pieces of functionality.

- `best/base` - Basic types and functions; portability helpers.
- `best/cli` - Utilities for building CLI applications.
- `best/container` - Container types. We interpret any generic "wrapper" type as
  a container.
- `best/func` - Helpers for manipulating functions as first-class objects.
- `best/iter` - Iterators and other functional programming types.
- `best/log` - Logging utilities.
- `best/math` - Numeric and integer utilities.
- `best/memory` - Low-level memory management.
- `best/meta` - Metaprogramming utilities, including reflection.
- `best/test` - A unit testing framework.
- `best/text` - Text processing utilities. `best` is a Unicode-first library.

## Contributing + Style

Bugfixes welcome, but this library is still under construction, so complex PRs
will likely be rejected. If you need functionality, please open an issue
instead.

The C++ code mostly follows
[Google's style guide](https://google.github.io/styleguide/cppguide.html), with
a few changes:

- The names of types and functions are in `snake_case`, following the STL.
- FTADLEs and other extension points are in `PascalCase`, and start with `Best`.
- Non-template constants, and tests, are `PascalCase` rather than `kConst`; all
  other variables are `snake_case`.
- Type parameters are `PascalCase`.
- Documentation comments are Rust style. Header comments follow the include
  list. The first line of a documentation is the name of the thing being
  documented.
- All public APIs must have a doc comment, except when they are implementing an
  interface (e.g., `override`s, magic identifiers like `.begin()`, and FTADLEs).

## Legal

`best` is made available to you under the terms of the Apache 2.0 license, the
full text of which can be found in the [LICENSE](LICENSE.md) file. `best` also
contains some algorithms copied from Rust's `libstd`, which is also Apache-2.0
licensed.

The authors of this work for the purposes of copyright are Miguel Young de la
Sota and the Best Contributors (which is to say, anyone who contributes code or
assets to be made available under the aforementioned license).
