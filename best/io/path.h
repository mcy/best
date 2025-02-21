/* //-*- C++ -*-///////////////////////////////////////////////////////////// *\

  Copyright 2024
  Miguel Young de la Sota and the Best Contributors 🧶🐈‍⬛

  Licensed under the Apache License, Version 2.0 (the "License"); you may not
  use this file except in compliance with the License. You may obtain a copy
  of the License at

                https://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
  License for the specific language governing permissions and limitations
  under the License.

\* ////////////////////////////////////////////////////////////////////////// */

#ifndef BEST_BASE_FWD2_H_
#define BEST_BASE_FWD2_H_

#include "best/base/port.h"
#include "best/io/ioerr.h"
#include "best/text/ascii.h"
#include "best/text/encoding.h"
#include "best/text/str.h"
#include "best/text/strbuf.h"
#include "best/text/utf8.h"

namespace best {
enum class filetype {
  File,
  Dir,
  Symlink,
};

/// # `best::path`
///
/// A string reference representing a filesystem path. See `best::pathbuf` for
/// the owned equivalent.
///
/// Internally, a path is represented as a probably-UTF-8 string, but allowing
/// for arbitrary bytes therein.
class path final {
 public:
  /// # `path::str`
  ///
  /// The string representation for a path.
  using str = best::pretext<best::wtf8>;

  /// # `path::Separator`
  ///
  /// The path separator on this platform.
  static constexpr str Separator = BEST_IS_WINDOWS ? "\\" : "/";

  /// # `path::Root`, `path::Cwd`, `path::Parent`
  ///
  /// Paths representing the special root, cwd, and parent components.
  static const path Root, Cwd, Parent;

  /// # `path::path("...")`
  ///
  /// Creates a new path from a string.
  constexpr path(best::converts_to<str> auto&& str) : str_(BEST_FWD(str)) {}

  /// # `path::as_os_str()`
  ///
  /// Returns the underlying UTF-8-ish string.
  constexpr str as_os_str() const { return str_; }

  /// # `path::to_pathbuf()`
  ///
  /// Makes a copy of this path and returns a pathbuf.
  best::pathbuf to_pathbuf() const;

  /// # `path::is_absolute()`, `path::is_relative()`
  ///
  /// Return whether this path is absolute or relative. On all platforms except
  /// Windows, this looks for a / prefix. On Windows, this is a path prefix,
  /// followed by a \ root.
  constexpr bool is_absolute() const;
  constexpr bool is_relative() const { return !is_absolute(); }

  /// # `path::windows_prefix()`
  ///
  /// Returns a Windows path prefix, such as a drive letter `C:`.
  constexpr best::option<str> windows_prefix() const;

  constexpr best::option<best::path> parent() const;

  constexpr best::option<str> name() const;
  constexpr best::option<str> stem() const;
  constexpr best::option<str> extension() const;

  best::pathbuf with_name(str name) const;
  best::pathbuf with_extension(str extension) const;

  constexpr best::option<best::path> relative_to(best::path base) const;

 private:
  class component_impl;

 public:
  /// # `path::component_iter`, `path::components()`
  ///
  /// An iterator over the path components of this path.
  using component_iter = best::iter<component_impl>;
  constexpr component_iter components() const;

  constexpr bool operator==(best::converts_to<path> auto&& that) const {
    return str_ == path(BEST_FWD(that)).str_;
  }

 private:
  best::pretext<best::wtf8> str_;
};

inline constexpr path path::Root(path::Separator), path::Cwd("."),
  path::Parent("..");

/// # `best::pathbuf`
///
/// An owned string representing a filesystem path. This is the owned equivalent
/// of `best::path`.
class pathbuf final {
 public:
  /// # `pathbuf::pathbuf("...")`
  ///
  /// Creates a new path from a string.
  constexpr explicit pathbuf(best::converts_to<best::path::str> auto str)
    : str_(str.as_codes()) {}

 private:
  best::vec<best::path::str::code> str_;
};
}  // namespace best

namespace best {
inline best::pathbuf best::path::to_pathbuf() const {
  return best::pathbuf(as_os_str());
}

inline constexpr bool best::path::is_absolute() const {
  auto str = str_;
  if (auto prefix = windows_prefix()) {
    str = str[{.start = prefix->size()}];
  } else if (BEST_IS_WINDOWS) {
    return false;
  }
  return str.starts_with(Separator);
}

inline constexpr best::option<best::path::str> best::path::windows_prefix()
  const {
  if (!BEST_IS_WINDOWS) { return best::none; }

  // TODO: Implement this.
  return best::none;
}

class path::component_impl final {
 private:
  friend path;
  friend best::iter<component_impl>;
  friend best::iter<component_impl&>;

  constexpr best::option<path> next() {
      
  }

  best::path rest_;
};
}  // namespace best

#endif