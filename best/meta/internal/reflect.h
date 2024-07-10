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
#include "best/base/port.h"
#include "best/meta/internal/names.h"
#include "best/meta/names.h"
#include "best/meta/taxonomy.h"
#include "best/text/str.h"

//! Reflection descriptors.
//!
//! This header defines the internal reflection descriptors that carry the
//! necessary information for constructing reflections of structs and enums.

namespace best::reflect_internal {
using ::best::names_internal::eyepatch;
using ::best::names_internal::materialize;

struct tag {};

// BestRowKeys for fdesc/vdesc.
template <auto>
struct fkey {};
template <auto>
struct vkey {};

// Fwd decls of the descriptor classes. The classes are completely private so
// users cannot tamper with them, but they befriend all of the reflection types.
// They have a lot of friends, so we define a macro to help keep them tidy.
template <typename, best::is_row = best::row<>, best::is_row = best::row<>>
class tdesc;  // Type descriptor.
template <auto, typename, typename, best::is_row = best::row<>>
class fdesc;  // Field descriptor.
template <auto, best::is_row = best::row<>>
class vdesc;  // Enum value descriptor.

// CTAD deduction guides for the descriptors.
template <typename S, best::is_row Items, best::is_row Tags>
tdesc(best::tlist<S>, Items, Tags) -> tdesc<S, Items, Tags>;
template <auto p, typename S, typename Get, best::is_row Tags>
fdesc(best::vlist<p>, best::tlist<S>, Get, Tags) -> fdesc<p, S, Get, Tags>;
template <auto e, best::is_row Tags>
vdesc(best::vlist<e>, Tags) -> vdesc<e, Tags>;

// Used in the reflection classes to validate that they are specialized
// correctly. They contain static asserts that check for the right kind value.
enum kind { Field, Value, Type };

// Concepts can't be befriended, so we indirect through this type.
template <typename Info, typename For>
struct validator {
  static constexpr bool value =
      Info::Kind == Type && best::same<For, typename Info::type>;
};
template <typename Info, typename For>
concept valid_reflection = validator<best::as_auto<Info>, For>::value;

// Friend declarations shared by all descriptors.
#define BEST_DESCRIPTOR_FRIENDS_                    \
  template <auto&>                                  \
  friend class best::reflected_type;                \
  template <auto&>                                  \
  friend class best::reflected_field;               \
  template <auto&>                                  \
  friend class best::reflected_value;               \
  template <typename>                               \
  friend class best::mirror;                        \
                                                    \
  template <typename, best::is_row, best::is_row>   \
  friend class reflect_internal::tdesc;             \
  template <auto, typename, typename, best::is_row> \
  friend class reflect_internal::fdesc;             \
  template <auto, best::is_row>                     \
  friend class reflect_internal::vdesc;             \
  template <typename, typename>                     \
  friend struct reflect_internal::validator;

// Discards anything it is constructed with. This is used for the inlined
// version of "select element" used in infer_struct().
struct discard {
  constexpr discard(auto&&) {}
};

/// Can't use best::equals, because that doesn't discriminate by type when
/// comparing pointers. This version does: pointers of distinct types compare
/// as unequal. This is necessary to deal with [[no_unique_address]] fields
/// correctly.
template <typename T, typename U>
constexpr bool typed_addr_eq(T* a, U* b)
  requires best::same<best::unqual<T>, best::unqual<U>>
{
  return a == b;
}
constexpr bool typed_addr_eq(...) { return false; }

/// A value that can convert to any other, used in the structured bindings
/// visitors.
inline constexpr struct any_t {
  constexpr any_t() = default;

  // We need to delete these to avoid tripping certain weird ambiguous
  // constructor cases. We want ALL construction to funnel through the operator
  // T().
  constexpr any_t(const any_t&) = delete;
  constexpr any_t(any_t&&) = delete;

  template <typename T>
    requires(!best::same<any_t, T>)
  operator T() const;
} any;
template <size_t>
inline constexpr any_t index_any{};

/// Counts the number of fields in the struct T by recursively trying to
/// construct a T with `n` arguments; once it hits a failure, we know we found
/// the largest number of arguments it can be initialized with.
template <best::is_struct T, size_t... i>
constexpr size_t count_fields() {
  if constexpr (!requires { T{index_any<i>...}; }) {
    return sizeof...(i) - 1;
  } else {
    return count_fields<T, i..., sizeof...(i)>();
  }
}
template <best::is_struct T>
inline constexpr size_t total_fields = count_fields<T, 0>();

/// # `best::reflect_internal::bind(struct, cb)`
///
/// The struct manipulation primitive: this explodes a struct into a tuple using
/// the power of structured bindings.
#include "best/meta/internal/reflect_bind.inc"
constexpr decltype(auto) bind(auto&& val, auto&& cb) {
  return reflect_internal::bind(BEST_FWD(val), BEST_FWD(cb),
                                best::rank<BEST_REFLECT_MAX_FIELDS_>{});
}

/// # `best::reflect_internal::bind(struct)`
///
/// Binds the `n`th field of `struct`.
template <size_t n>
constexpr decltype(auto) bind(auto&& val) {
  return best::indices<n>.apply([&](auto... i) -> decltype(auto) {
    return reflect_internal::bind(
        BEST_FWD(val),
        [&](best::dependent<discard, decltype(i)>..., auto&& the_one,
            auto&&... rest) -> decltype(auto) {
          // Structured bindings' type does not preserve value category
          // correctly, so we need to adjust its type to match the value
          // category of `val`.
          return best::refcopy<decltype(the_one), decltype(val)>(the_one);
        });
  });
}

template <auto p, typename S, typename Get, best::is_row Tags>
class fdesc final {
 public:
  using BestRowKey = fkey<p>;

 private:
  BEST_DESCRIPTOR_FRIENDS_

  using struct_ = S;
  using type = best::unptr<decltype(p.unwrap)>;
  inline static constexpr auto Kind = Field;

  constexpr fdesc(best::vlist<p>, best::tlist<S>, Get get, Tags tags)
      : get_(get), tags_(tags) {}

  // Adds tags to this field.
  constexpr auto add(auto... tags) const {
    return reflect_internal::fdesc(best::vals<p>, best::types<S>, get_,
                                   tags_ + best::row(tags...));
  }

  // A (wrapped) pointer into materialized<T>.
  static constexpr auto materialized = p;

  static constexpr best::str name_ = names_internal::parse<p>();
  Get get_;
  Tags tags_;
};

template <auto e, best::is_row Tags>
class vdesc final {
 public:
  using BestRowKey = vkey<e>;

 private:
  BEST_DESCRIPTOR_FRIENDS_

  using enum_ = decltype(e);
  inline static constexpr auto Kind = Value;

  constexpr vdesc(best::vlist<e>, Tags tags) : tags_(tags) {}

  // Adds tags to this value.
  constexpr auto add(auto... tags) const {
    return reflect_internal::vdesc(best::vals<e>, tags_ + best::row(tags...));
  }

  inline static constexpr enum_ elem_ = e;
  Tags tags_;
};

template <typename T, best::is_row Items, best::is_row Tags>
class tdesc final {
 private:
  BEST_DESCRIPTOR_FRIENDS_

  using type = T;
  inline static constexpr auto Kind = Type;

  constexpr tdesc(best::tlist<T>, Items items, Tags tags)
      : items_(items), tags_(tags) {}

  // Adds tags to this type.
  constexpr auto add(auto... tags) const {
    return reflect_internal::tdesc(best::types<T>, items_,
                                   tags_ + best::row(tags...));
  }

  // Finds the index of a field, given a pointer-to-member.
  template <best::is_member_ptr auto pm>
  constexpr auto find() const {
    return items_.select_indices(
        best::types<fkey<eyepatch(best::addr(materialize<T>().*pm))>>);
  }

  // Adds a field or updates it by appending tags
  template <best::is_member_ptr auto pm>
  constexpr auto add(auto... tags) const {
    auto found = find<pm>();
    if constexpr (found.is_empty()) {
      return reflect_internal::tdesc(
          best::types<T>,
          items_.push(fdesc(
              best::vals<eyepatch(best::addr(materialize<T>().*pm))>,
              best::types<T>,
              [](auto&& value) -> decltype(auto) {
                return BEST_FWD(value).*pm;
              },
              best::row(tags...))),
          tags_);
    } else {
      return reflect_internal::tdesc(
          best::types<T>,
          items_.update(
              items_[best::index<found.template value<0>>].add(tags...),
              best::index<found.template value<0>>),
          tags_);
    }
  }

  // Finds the index of an enum value, given the value.
  template <best::is_enum auto e>
  constexpr auto find() const {
    return items_.select_indices(best::types<vkey<e>>);
  }

  // Adds a value or updates it by appending tags
  template <best::is_enum auto e>
  constexpr auto add(auto... tags) const {
    auto found = find<e>();
    if constexpr (found.is_empty()) {
      return reflect_internal::tdesc(
          best::types<T>, items_.push(vdesc(best::vals<e>, best::row(tags...))),
          tags_);
    } else {
      return reflect_internal::tdesc(
          best::types<T>,
          items_.update(
              items_[best::index<found.template value<0>>].add(tags...),
              best::index<found.template value<0>>),
          tags_);
    }
  }

  // Hides an item.
  template <auto x>
  constexpr auto hide() const {
    auto found = find<x>();
    return reflect_internal::tdesc(
        best::types<T>, items_.remove(best::index<found.template value<0>>),
        tags_);
  }

  // Infers the default reflection descriptor for a struct.
  static constexpr auto infer_struct()
    requires best::is_struct<T>
  {
    auto fields = best::indices<total_fields<T>>.apply([&]<size_t... i> {
      return best::row{fdesc(
          best::vals<eyepatch(
              best::addr(reflect_internal::bind<i>(materialize<T>())))>,
          best::types<T>,
          [](auto&& v) -> decltype(auto) {
            return reflect_internal::bind<i>(BEST_FWD(v));
          },
          best::row())...};
    });
    return reflect_internal::tdesc(best::types<T>, fields, best::row());
  }

  // Infers the default reflection descriptor for an enum.
  template <size_t start, size_t count>
  static constexpr auto infer_enum()
    requires best::is_enum<T>
  {
    BEST_PUSH_GCC_DIAGNOSTIC()
    BEST_IGNORE_GCC_DIAGNOSTIC("-Wenum-constexpr-conversion")
    constexpr auto ok = best::indices<count>.apply([]<size_t... i> {
      constexpr size_t ok_count =
          (0 + ... + best::value_name<T(start + i)>.has_value());

      std::array<T, ok_count> ok = {};
      size_t idx = 0;

      ((best::value_name<T(start + i)>.has_value()
            ? void(ok[idx++] = T(start + i))
            : void()),
       ...);
      return ok;
    });
    BEST_POP_GCC_DIAGNOSTIC()

    auto values = best::indices<ok.size()>.apply([&]<size_t... i> {
      return best::row{vdesc{best::vals<ok[i]>, best::row()}...};
    });
    return reflect_internal::tdesc(best::types<T>, values, best::row());
  }

  Items items_;
  Tags tags_;
};

template <typename mirror>
inline constexpr mirror make_mirror = mirror::BEST_MIRROR_EMPTY_();

template <typename T,
          typename mirror = best::mirror<best::seal<tdesc<T, row<>, row<>>>>>
  requires requires { BestReflect(make_mirror<mirror>, (T*){}); }
inline constexpr auto desc =
    BestReflect(make_mirror<mirror>, (T*){}).BEST_MIRROR_UNWRAP_(tag{});
};  // namespace best::reflect_internal

#undef BEST_DESCRIPTOR_FRIENDS_
#endif  // BEST_META_INTERNAL_REFLECT_H_
