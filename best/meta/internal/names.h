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

// Helpers for creating a structural value that will contain the name of a
// field symbol.
template <typename T>
const T v{};
template <typename T>
struct w {
  const T* p;
};
template <typename T>
w(const T*) -> w<T>;

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
  best::str separator;
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

inline constexpr auto BulkFieldOffsets = [] {
  auto name = names_internal::raw_name<std::array{
      (const void*)&v<BEST_REFLECT_STRUCT_>.BEST_REFLECT_FIELD1_,
      (const void*)&v<BEST_REFLECT_STRUCT_>.BEST_REFLECT_FIELD2_}>();
  auto idx = *name.find(FieldNeedle1);
  auto idx2 = *name.find(FieldNeedle2);
  return raw_offsets{
      .prefix = idx,
      .suffix = name.size() - idx2 - FieldNeedle2.size(),
      .separator =
          str(unsafe{"the compiler made this string, it better be UTF-8"},
              name[{.start = idx + FieldNeedle1.size(), .end = idx2}]),
  };
}();

constexpr best::str remove_namespace(best::str path) {
  while (auto split = path.split_on("::")) {
    path = split->second();
  }
  return path;
}
};  // namespace best::names_internal

#define BEST_REFLECT_FIELD1_ _private
#define BEST_REFLECT_FIELD2_ _private
#define BEST_REFLECT_STRUCT_ _private
#define BEST_REFLECT_VALUE_ _private
#define BEST_REFLECT_ENUM_ _private

#endif  // BEST_META_INTERNAL_REFLECT_H_
