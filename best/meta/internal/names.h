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

#ifndef BEST_META_INTERNAL_NAMES_H_
#define BEST_META_INTERNAL_NAMES_H_

#include <source_location>

#include "best/base/fwd.h"
#include "best/base/port.h"
#include "best/container/span.h"
#include "best/text/str.h"

// This needs to go in the global namespace, since its full name is relevant for
// substring operations that extract the names of things. These names are
// #defined away at the bottom of this header.
struct BEST_REFLECT_STRUCT_ {
  BEST_REFLECT_STRUCT_* BEST_REFLECT_FIELD1_;
  BEST_REFLECT_STRUCT_* BEST_REFLECT_FIELD2_;
  enum BEST_REFLECT_ENUM_ { BEST_REFLECT_VALUE_ };
};

namespace best::names_internal {
struct priv {};

// We work exclusively with spans here to avoid pulling in the full machinery
// of best::str encoding, which is not fast in constexpr.
template <auto x>
constexpr best::span<const char> raw_name() {
  return best::span<const char>::from_nul(
      std::source_location::current().function_name());
}
template <typename x>
constexpr best::span<const char> raw_name() {
  return best::span<const char>::from_nul(
      std::source_location::current().function_name());
}

// Similar to best::lie, but this materializes a real reference that can be
// manipulated (but not read/written) by constexpr code.
template <typename T>
extern T v;
template <typename T>
constexpr T& materialize() {
  BEST_PUSH_GCC_DIAGNOSTIC()
  BEST_IGNORE_GCC_DIAGNOSTIC("-Wundefined-var-template")
  return v<T>;
  BEST_POP_GCC_DIAGNOSTIC()
}

// Needles to search for that are *definitely* gonna be in the target compiler's
// pretty printed function names.
inline constexpr auto TypeNeedle =
    best::span<const char>::from_nul("BEST_REFLECT_STRUCT_");
inline constexpr auto FieldNeedle = best::span<const char>::from_nul(
    "&BEST_REFLECT_STRUCT_::BEST_REFLECT_FIELD1_");
inline constexpr auto FieldNeedle1 =
    best::span<const char>::from_nul("BEST_REFLECT_FIELD1_");
inline constexpr auto FieldNeedle2 =
    best::span<const char>::from_nul("BEST_REFLECT_FIELD2_");
inline constexpr auto ValueNeedle = best::span<const char>::from_nul(
    "BEST_REFLECT_STRUCT_::BEST_REFLECT_VALUE_");

struct raw_offsets {
  size_t prefix, suffix;
};

// Figure out how the compiler lays out the names of types, fields, and enum
// values in the names of function templates.
inline constexpr auto TypeOffsets = [] {
  auto name = raw_name<BEST_REFLECT_STRUCT_>();
  auto idx = *name.find(TypeNeedle);
  return raw_offsets{
      .prefix = idx,
      .suffix = name.size() - idx - TypeNeedle.size(),
  };
}();
inline constexpr auto FieldOffsets = [] {
  auto name = raw_name<&BEST_REFLECT_STRUCT_::BEST_REFLECT_FIELD1_>();
  auto idx = *name.find(FieldNeedle1);
  return raw_offsets{
      .prefix = idx,
      .suffix = name.size() - idx - FieldNeedle1.size(),
  };
}();
inline constexpr auto ValueOffsets = [] {
  auto name =
      raw_name<BEST_REFLECT_STRUCT_::BEST_REFLECT_ENUM_::BEST_REFLECT_VALUE_>();
  auto idx = *name.find(ValueNeedle);
  return raw_offsets{
      .prefix = idx,
      .suffix = name.size() - idx - ValueNeedle.size(),
  };
}();

// We can't stick a subobject reference into a template parameter, but we can
// stick a structural type that CONTAINS a subobject pointer into a template
// parameter. Ah, C++.
template <typename T>
struct eyepatch {
  T unwrap;
};
template <typename T>
eyepatch(T) -> eyepatch<T>;

// This is used for the variadic version of field_name, which is materialized by
// field_names<T> in internal/reflect.h
inline constexpr auto FieldPtrOffsets = [] {
  constexpr auto p = &materialize<BEST_REFLECT_STRUCT_>().BEST_REFLECT_FIELD1_;
  auto name = names_internal::raw_name<eyepatch(p)>();
  auto idx = *name.find(FieldNeedle1);
  return raw_offsets{
      .prefix = idx,
      .suffix = name.size() - idx - FieldNeedle1.size(),
  };
}();

constexpr best::str remove_namespace(best::str path) {
  while (auto split = path.split_once("::")) {
    path = split->second();
  }
  return path;
}

template <typename T, typename Names>
constexpr Names parse() {
  auto offsets = names_internal::TypeOffsets;
  auto raw = names_internal::raw_name<T>();
  return Names(
      priv{},
      best::str(unsafe("the compiler made this string, it better be UTF-8"),
                raw[{
                    .start = offsets.prefix,
                    .count = raw.size() - offsets.prefix - offsets.suffix,
                }]));
}

template <best::is_member_ptr auto pm>
constexpr best::str parse() {
  auto offsets = names_internal::FieldOffsets;
  auto raw = names_internal::raw_name<pm>();
  auto name =
      best::str(unsafe("the compiler made this string, it better be UTF-8"),
                raw[{
                    .start = offsets.prefix,
                    .count = raw.size() - offsets.prefix - offsets.suffix,
                }]);

  // `name` is going to be scoped, so we need to strip off a leading path.
  return names_internal::remove_namespace(name);
}

// This only works for offsets into materialize()!
template <eyepatch p>
constexpr best::str parse() {
  auto offsets = names_internal::FieldPtrOffsets;
  auto raw = names_internal::raw_name<p>();
  // This one's trickier than the others because the size of the prefix depends
  // on the type of `p`. To work around this, we chop off the suffix, and then
  // search for the last `.` or `->` (MSVC uses -> and it feels easier to just
  // search for *both*...).
  auto prefix = raw[{
      .end = raw.size() - offsets.suffix,
  }];

  // TODO(mcyoung): Use rfind() once we implement that.
  size_t i = prefix.size();
  for (; i > 0; --i) {
    unsafe u("already did the bounds check");
    if (prefix.at(u, i - 1) == '.') {
      break;
    }
    if (i > 1 && prefix.at(u, i - 1) == '>' && prefix.at(u, i - 2) == '-') {
      break;
    }
  }

  return best::str(unsafe("the compiler made this string, it better be UTF-8"),
                   prefix[{.start = i}]);
}

BEST_PUSH_GCC_DIAGNOSTIC()
BEST_IGNORE_GCC_DIAGNOSTIC("-Wenum-constexpr-conversion")
template <best::is_enum auto e>
constexpr best::option<best::str> parse() {
  auto offsets = names_internal::ValueOffsets;
  auto raw = names_internal::raw_name<e>();
  auto name =
      best::str(unsafe("the compiler made this string, it better be UTF-8"),
                raw[{
                    .start = offsets.prefix,
                    .count = raw.size() - offsets.prefix - offsets.suffix,
                }]);

  if (name.starts_with('(')) return best::none;
  // `name` is going to be scoped, so we need to strip off a leading path.
  return names_internal::remove_namespace(name);
};
BEST_POP_GCC_DIAGNOSTIC()

};  // namespace best::names_internal

#define BEST_REFLECT_FIELD1_ _private
#define BEST_REFLECT_FIELD2_ _private
#define BEST_REFLECT_STRUCT_ _private
#define BEST_REFLECT_VALUE_ _private
#define BEST_REFLECT_ENUM_ _private

#endif  // BEST_META_INTERNAL_NAMES_H_
