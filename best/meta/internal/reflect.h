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

#ifndef BEST_META_INTERNAL_REFLECT_H_
#define BEST_META_INTERNAL_REFLECT_H_

#include "best/base/fwd.h"
#include "best/meta/internal/names.h"
#include "best/meta/taxonomy.h"
#include "best/text/str.h"

namespace best::reflect_internal {
/// A value that can convert to any other, used in the structured bindings
/// visitors.
inline constexpr struct any_t {
  template <typename T>
  operator T() const;
} any;

/// # `best::reflect_internal::bind(cb, struct)`
///
/// The struct manipulation primitive: this explodes a struct into a tuple using
/// the power of structured bindings.
#include "best/meta/internal/reflect_bind.inc"
constexpr decltype(auto) bind(auto&& val, auto&& cb) {
  return reflect_internal::bind(BEST_FWD(val), BEST_FWD(cb),
                                best::rank<BEST_REFLECT_MAX_FIELDS_>{});
}

enum kind { Field, Struct, Value, Enum, NoFields };

struct validator {
  /// This needs to be in a struct so the info classes can befriend it.
  template <typename Info, typename For>
  static constexpr bool value =
      (Info::Kind == Struct && best::same<For, typename Info::struct_>) ||
      (Info::Kind == Enum && best::same<For, typename Info::enum_>) ||
      (Info::Kind == NoFields && (best::is_struct<For> || best::is_enum<For>));
};

template <typename Info, typename For>
concept valid_reflection = validator::value<Info, For>;

template <typename Info>
concept is_info = requires {
  { Info::Kind } -> best::same<kind>;
};

template <typename T, best::is_row Tags>
class field_info final {
  template <auto&>
  friend class best::reflected_field;
  template <auto&>
  friend class best::reflected_type;
  template <typename>
  friend class mirror;
  friend validator;

  using type = T;
  inline static constexpr auto Kind = Field;

 public:
  constexpr field_info(best::str name, Tags tags) : name_(name), tags_(tags) {}

 private:
  best::str name_;
  Tags tags_;
};

/// Can't use best::equals, because that doesn't discriminate by type when
/// comparing pointers. This version does: pointers of distinct types compare
/// as unequal. This is necessary to deal with [[no_unique_address]] fields
/// correctly.
template <typename T>
constexpr bool equals(const T* a, const T* b) {
  return a == b;
}
constexpr bool equals(const auto* a, const auto* b) { return false; }

template <typename S, best::is_row Fields, best::is_row Tags>
class struct_info final {
  template <auto&>
  friend class best::reflected_type;
  template <typename>
  friend class mirror;
  friend validator;

  using struct_ = S;
  using enum_ = void;
  inline static constexpr auto Kind = Struct;

 public:
  constexpr struct_info(Fields fields, Tags tags)
      : items_(fields), tags_(tags) {}

 //private:
  // Finds the index of a field, given a pointer-to-member.
  template <auto pm>
  constexpr auto index() const {
    // First, find the index of this member. The simplest way is to compare
    // pointers.
    size_t idx = 0;
    auto* ptr = best::addr(names_internal::v<S>.*pm);
    bind(names_internal::v<S>, [&](auto&&... fields) {
      // NB: If this is ever a problem for optimization, note that it can likely
      // be realized as binary search.
      ((reflect_internal::equals(best::addr(fields), ptr) ? true
                                                          : (idx++, false)) ||
       ...);
    });
    return idx;
  }

  // Adds tags to a field.
  template <auto pm>
  auto add(auto... tags) {
    
  }

  Fields items_;
  Tags tags_;
};

template <best::is_struct S>
constexpr auto infer_struct() {
  constexpr size_t num_fields = bind(
      names_internal::v<S>, [](auto&&... fields) { return sizeof...(fields); });
  constexpr auto raw_names =
      names_internal::raw_name<bind(names_internal::v<S>, [](auto&&... fields) {
        return std::array{(const void*)best::addr(fields)...};
      })>();
  constexpr auto names = [&] {
    auto offsets = names_internal::BulkFieldOffsets;
    auto to_parse =
        str(unsafe{"the compiler made this string, it better be UTF-8"},
            raw_names[{
                .start = offsets.prefix,
                .end = raw_names.size() - offsets.suffix,
            }]);

    size_t idx = 0;
    std::array<best::str, num_fields> names{};
    while (auto split = to_parse.split_on(offsets.separator)) {
      names[idx++] = split->first();
      to_parse = split->second();
    }
    names[idx] = to_parse;

    return names;
  }();

  auto fields = bind(names_internal::v<S>, [&](auto&&... fields) {
    size_t idx = 0;
    return best::row{field_info<best::unref<decltype(fields)>, best::row<>>(
        names[idx++], {})...};
  });

  return struct_info<S, decltype(fields), best::row<>>(fields, {});
}

template <typename E, typename... Tags>
class elem_info final {
  template <auto&>
  friend class best::reflected_value;
  template <auto&>
  friend class best::reflected_type;
  template <typename>
  friend class mirror;
  friend validator;

  using enum_ = E;
  inline static constexpr auto Kind = Value;

  constexpr elem_info(best::str name, enum_ elem, best::row<Tags...> tags)
      : name_(name), elem_(elem), tags_(tags) {}

  best::str name_;
  enum_ elem_;
  best::row<Tags...> tags_;
};

template <typename E, typename Tags, typename... Elems>
class enum_info final {
  template <auto&>
  friend class best::reflected_type;
  template <typename>
  friend class mirror;
  friend validator;

  using struct_ = void;
  using enum_ = E;
  inline static constexpr auto Kind = Enum;

  constexpr enum_info(best::str name, Tags tags, best::row<Elems...> values)
      : name_(name), tags_(tags), items_(values) {}

  best::str name_;
  Tags tags_;
  best::row<Elems...> items_;
};

template <typename Tags>
class no_fields final {
  template <auto&>
  friend class best::reflected_type;
  template <typename>
  friend class mirror;
  friend validator;

  using struct_ = void;
  using enum_ = void;
  inline static constexpr auto Kind = NoFields;

  constexpr no_fields(best::str name, Tags tags) : name_(name), tags_(tags) {}

  best::str name_;
  Tags tags_;
  best::row<> items_;
};

template <typename T, typename mirror = best::mirror<T>>
inline constexpr auto info = BestReflect(mirror::BEST_MIRROR_FTADLE_, (T*){});

};  // namespace best::reflect_internal

#define BEST_REFLECT_FIELD_ _private
#define BEST_REFLECT_STRUCT_ _private
#define BEST_REFLECT_VALUE_ _private
#define BEST_REFLECT_ENUM_ _private

#endif  // BEST_META_INTERNAL_REFLECT_H_
