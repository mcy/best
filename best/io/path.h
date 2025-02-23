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
#include "best/container/vec.h"
#include "best/func/defer.h"
#include "best/text/str.h"
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

  /// # `path::path()`
  ///
  /// Returns the empty path.
  constexpr path() = default;

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

  /// # `path::is_empty()`
  ///
  /// Returns whether this is the empty (default) path.
  constexpr bool is_empty() const { return str_.is_empty(); }

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

  /// # `path::parent()`
  ///
  /// Returns the path with its final component removed. Returns `best::none`
  /// if the path terminates in a root, or if it's the empty path.
  constexpr best::option<best::path> parent() const;

  /// # `path::name()`, `path::stem()`, `path::extension()`.
  ///
  /// `name()` returns the final "normal" component of this path, if it has one.
  ///
  /// `extension()` returns everything after the final `.` in the name, unless
  /// the name starts in a `.`. `stem()` returns everything up until the `.`
  /// that delimits the extension.
  constexpr best::option<str> name() const;
  constexpr best::option<str> stem() const;
  constexpr best::option<str> extension() const;

  best::pathbuf with_name(str name) const;
  best::pathbuf with_extension(str extension) const;

  best::pathbuf join(str component) const;

  constexpr best::option<best::path> relative_to(best::path base) const;

 private:
  class component_impl;

 public:
  /// # `path::component_iter`, `path::components()`
  ///
  /// An iterator over the path components of this path. This will automatically
  /// strip a Windows drive prefix, and will not yield it as part of iteration.
  ///
  /// When parsing, the semantics are the same as for Rust's
  /// `Path::components()`:
  ///
  /// 1. Repeated separators are ignored: `a/b` and `a//b` are the same.
  /// 2. Current dir dots are ignored, except at the start: `a/./b` and `a/b`,
  ///     and `a/b/.` are the same.
  /// 3. Trailing separators are ignored: `a/b` and `a/b/` are the same.
  using component_iter = best::iter<component_impl>;
  constexpr component_iter components() const;

  constexpr bool operator==(best::converts_to<path> auto&& that) const {
    return str_ == path(BEST_FWD(that)).str_;
  }

  friend void BestFmt(auto& fmt, path path) { fmt.format(path.str_); }

  constexpr friend void BestFmtQuery(auto& query, path*) {
    query.requires_debug = false;
    query.uses_method = [](best::rune r) { return r == 'q'; };
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
 public:
  using BestIterArrow = void;

  /// # `component_iter::rest()`
  ///
  /// Returns the rest of the path not yet yielded.
  constexpr path rest() { return rest_; }

 private:
  static constexpr str DotSlash = BEST_IS_WINDOWS ? ".\\" : "./";
  static constexpr str SlashDot = BEST_IS_WINDOWS ? "\\." : "/.";

  friend path;
  friend best::iter<component_impl>;
  friend best::iter<component_impl&>;

  constexpr best::option<path> next() {
    best::defer trim_ = [&] { trim(); };

    if (!started_) {
      started_ = true;

      if (rest_.is_empty() || rest_ == str{"."}) { return best::path(rest_); }

      if (rest_.starts_with(str("/").as_codes()) ||
          rest_.starts_with(DotSlash.as_codes())) {
        return best::path(rest_[{.count = 1}]);
      }
    }

    if (auto split = rest_.split_once(Separator.as_codes())) {
      auto [chunk, rest] = *split;
      rest_ = rest;
      return best::path(chunk);
    }

    if (rest_.is_empty()) { return best::none; }

    auto chunk = rest_;
    rest_ = {};
    return best::path(chunk);
  }

  constexpr best::option<path> next_back() {
    if (rest_.is_empty()) {
      if (!started_) {
        started_ = true;
        return best::path(rest_);
      }
      return best::none;
    }

    trim_back();
    if (rest_ == Separator) {
      auto chunk = rest_;
      rest_ = {};
      started_ = true;
      return best::path(chunk);
    }

    // Don't have rfind yet.
    size_t idx = -1;
    for (size_t i = 0; i < rest_.size(); ++i) {
      size_t j = rest_.size() - i - 1;
      if (rest_[j] == Separator.as_codes()[0]) {
        idx = j;
        break;
      }
    }

    if (idx == -1) {
      auto chunk = rest_;
      rest_ = {};
      started_ = true;
      return best::path(chunk);
    }

    auto chunk = rest_[{.start = idx + 1}];
    rest_ = rest_[{.end = idx + (idx == 0)}];
    return best::path(chunk);
  }

  constexpr void trim() {
    while (true) {
      if (rest_.consume_prefix(Separator.as_codes())) { continue; }
      if (rest_.consume_prefix(DotSlash.as_codes())) { continue; }
      break;
    }

    if (rest_ == str{"."}) { rest_ = {}; }
  }

  constexpr void trim_back() {
    while (true) {
      if (rest_ == str(".") || rest_ == Separator) { break; }
      if (rest_.consume_suffix(Separator.as_codes())) { continue; }
      if (rest_.consume_suffix(SlashDot.as_codes())) { continue; }
      break;
    }
  }

  constexpr explicit component_impl(best::path rest)
    : rest_(rest.str_.as_codes()) {}

  best::span<const str::code> rest_;
  bool started_ = false;
};

constexpr best::path::component_iter best::path::components() const {
  path rest = *this;
  if (auto prefix = windows_prefix()) {
    rest.str_ = rest.str_[{.start = prefix->size()}];
  }
  return component_iter(component_impl(rest));
};

constexpr best::option<best::path> best::path::parent() const {
  if (is_empty()) { return best::none; }
  auto it = components();
  if (it.next_back() == Separator) { return best::none; }
  return it->rest();
}

constexpr best::option<best::path::str> best::path::name() const {
  return components().next_back().then(
    [](best::path name) -> best::option<str> {
      if (name == Separator || name == Cwd || name == Parent) {
        return best::none;
      }
      return name.as_os_str();
    });
}

namespace path_internal {
inline constexpr best::row<best::path::str, best::option<best::path::str>>
dot_split(best::path::str name) {
  if (name.starts_with(".")) { return {name, best::none}; }

  // Don't have rfind yet.
  size_t idx = -1;
  for (size_t i = 0; i < name.size(); ++i) {
    size_t j = name.size() - i - 1;
    if (name.as_codes()[j] == '.') {
      idx = j;
      break;
    }
  }

  if (idx == -1) { return {name, best::none}; }

  return {name[{.end = idx}], name[{.start = idx + 1}]};
}
}  // namespace path_internal

constexpr best::option<best::path::str> best::path::stem() const {
  return name().map(
    [](auto name) { return path_internal::dot_split(name).first(); });
}
constexpr best::option<best::path::str> best::path::extension() const {
  return name().then(
    [](auto name) { return path_internal::dot_split(name).second(); });
}
}  // namespace best

#endif