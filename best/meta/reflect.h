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

#ifndef BEST_META_REFLECT_H_
#define BEST_META_REFLECT_H_

#include "best/container/row.h"
#include "best/memory/span_sort.h"
#include "best/meta/internal/reflect.h"
#include "best/meta/traits/types.h"
#include "best/text/str.h"

//! Struct and enum reflection.
//!
//! `best` provides a mechanism for reflecting the fields and variants of
//! user-defined structs and enums, which must define the `BestReflect()`
//! FTADLE:
//!
//! ```
//! friend constexpr auto BestReflect(auto& mirror, MyStruct*) {
//!   return mirror.infer()
//!     .with(best::vals<&MyStruct::some_field>, tags...);
//! }
//! ```

namespace best {
/// # `best::reflected`, `best::is_reflected_struct`,
///   `best::is_reflected_enum`
///
/// Whether `T` can be reflected, i.e., it is a struct or enum type that has a
/// working `BestReflect()` FTADLE implementation.
template <typename T>
concept reflected = requires {
  {
    reflect_internal::desc<best::as_auto<T>>
  } -> reflect_internal::valid_reflection<best::as_auto<T>>;
};
template <typename T>
concept is_reflected_struct =
  best::is_struct<best::as_auto<T>> && best::reflected<T>;
template <typename T>
concept is_reflected_enum =
  best::is_enum<best::as_auto<T>> && best::reflected<T>;

/// # `best::reflect<T>`
///
/// Obtains a reflection of a reflected type. This will be a value of the type
/// `best::reflected_type`, although the exact specialization is not nameable.
template <best::reflected T>
inline constexpr auto reflect =
  best::reflected_type<reflect_internal::desc<best::as_auto<T>>>{};

/// # `best::fields()`
///
/// Extracts all of the fields out of a reflected struct and creates a row
/// of references of them.
constexpr auto fields(best::is_reflected_struct auto&& value) {
  return best::reflect<decltype(value)>.apply(
    [&](auto... f) { return best::row(best::bind, BEST_FWD(value)->*f...); });
}

/// # `best::mirror`
///
/// A value of this type is passed to the `BestReflect` FTADLE. This type
/// cannot be constructed by users; it is constructed for them by the reflection
/// framework.
///
/// A mirror can be used to construct reflections of a type. The actual type of
/// these reflections is an implementation detail, since they have complex type
/// parameters. The mirror provides a friendlier API for manipulating these
/// reflections.
template <typename I, typename W>
class mirror final {
 private:
  using info_t = best::unabridge<I>;
  using with_t = best::unabridge<W>;

 public:
  /// # `mirror::reflected`
  ///
  /// The type this mirror reflects.
  using reflected = info_t::type;

  /// # `mirror::infer()`
  ///
  /// Returns a mirror containing the default reflection for the type `T`.
  /// This will discard all tags added to the current mirror.
  ///
  /// When `T` is an enum, it is possible to explicitly specify the range of
  /// values to consider for finding enum values.
  constexpr auto infer() const
    requires best::is_struct<reflected> &&
             ((reflect_internal::total_fields<reflected> <=
               BEST_REFLECT_MAX_FIELDS_));
  template <best::bounds range = best::bounds{.end = 64}>
  constexpr auto infer() const
    requires best::is_enum<reflected> &&
             (range.try_compute_count({}).has_value());

  /// # `mirror.with(tags...)
  ///
  /// Returns an updated mirror that adds the given tags to this type. They can
  /// be retrieved later with `best::reflected_type::tags()`.
  constexpr auto with(auto... tags) const;

  /// # `mirror.with(best::vals<member>, tags...)`
  ///
  /// Reflects that `T` has the given member, returning an updated mirror.
  ///
  /// This can method may be called multiple times, and can be used to add tags
  /// to a member. Tags can later be retrieved via e.g.
  /// `best::reflected_field::tags()`.
  constexpr auto with(best::is_member_ptr auto field, auto... tags) const
    requires best::is_struct<reflected>;
  constexpr auto with(best::same<reflected> auto value, auto... tags) const
    requires best::is_enum<reflected>;

  /// # `mirror.hide()`
  ///
  /// Updates this mirror to make it forget about a particular member, so that
  /// it is not visible to reflection.
  constexpr auto hide(best::is_member_ptr auto field) const
    requires best::is_struct<reflected>;
  constexpr auto hide(best::same<reflected> auto value) const
    requires best::is_enum<reflected>;

  mirror(const mirror&) = delete;
  mirror& operator=(const mirror&) = delete;

 private:
  friend best::reflect_internal::reify;
  template <typename, typename>
  friend class mirror;

  constexpr explicit mirror(info_t info, with_t with)
    : info_(info), with_(with) {}
  info_t info_;
  with_t with_;
};
template <typename... Ts>
mirror(Ts...) -> mirror<best::abridge<Ts>...>;

/// # `best::reflected_field`
///
/// A field of some reflected struct. The type parameters are an implementation
/// detail; this type cannot be instantiated by users.
///
/// A `best::reflected_field` offers accessors for information about the field,
/// such as its name and tags. It can also be used like a pointer-to-member,
/// extracting the value of a field from a struct value with `value->*field`.
template <auto& desc_>
class reflected_field final {
 private:
  static_assert(desc_.Kind == reflect_internal::Field,
                "cannot instantiate best::reflected_field directly");
  using info_t = best::as_auto<decltype(desc_)>;

 public:
  /// # `reflected_field::reflected`
  ///
  /// The type this field was reflected from.
  using reflected = info_t::struct_;

  /// # `reflected_field::type`
  ///
  /// The type of this field.
  using type = info_t::type;

  /// # `reflected_field::name()`
  ///
  /// Returns the name of this field as specified in the `BestReflect()` call.
  constexpr best::str name() const { return desc_.name_; }

  /// # `reflected_field::tags()`
  ///
  /// Returns any tags on this field that can be selected by `Key`.
  template <typename Key>
  constexpr best::is_row auto tags(best::tlist<Key> key = {}) const {
    return desc_.tags_.select(key);
  }

  /// # `reflected_field::operator->*`
  ///
  /// Extracts the field described by this reflection from an actual struct
  /// value.
  constexpr friend decltype(auto) operator->*(auto&& struct_,
                                              reflected_field field)
    requires best::same<best::as_auto<decltype(struct_)>, reflected>
  {
    return desc_.get_(BEST_FWD(struct_));
  }
  constexpr friend decltype(auto) operator->*(auto* struct_,
                                              reflected_field field)
    requires best::same<best::as_auto<best::un_raw_ptr<decltype(struct_)>>,
                        reflected>
  {
    return desc_.get_(*struct_);
  }

 private:
  template <auto&>
  friend class reflected_type;
};

/// # `best::reflected_value`
///
/// A value of some reflected enum. The type parameter is an implementation
/// detail; this type cannot be instantiated by users.
///
/// A `best::reflected_value` offers accessors for information about the field,
/// such as its name, access to the enum value itself, and so on.
template <auto& desc_>
class reflected_value final {
 private:
  static_assert(desc_.Kind == reflect_internal::Value,
                "cannot instantiate best::reflected_value directly");
  using info_t = best::as_auto<decltype(desc_)>;

 public:
  /// # `reflected_field::reflected`
  ///
  /// The type this field was reflected from.
  using reflected = info_t::enum_;

  /// # `reflected_field::value`
  ///
  /// The actual reflected value.
  static constexpr reflected value = desc_.elem_;

  /// # `reflected_field::name()`
  ///
  /// Returns the name of this field as specified in the `BestReflect()` call.
  constexpr best::str name() const { return *best::value_name<desc_.elem_>; }

  /// # `reflected_field::tags()`
  ///
  /// Returns any tags on this field that can be selected by `Key`.
  template <typename Key>
  constexpr best::is_row auto tags(best::tlist<Key> key = {}) const {
    return desc_.tags_.select(key);
  }

 private:
  template <auto&>
  friend class reflected_type;
};

/// # `best::reflected_type`
///
/// The result of reflecting a type. The type parameter is an implementation
/// detail; this type cannot be instantiated by users.
///
/// To obtain an instance of this type, use `best::reflect<T>`.
template <auto& desc_>
class reflected_type final {
 private:
  static_assert(desc_.Kind == reflect_internal::Type,
                "cannot instantiate best::field directly");
  using info_t = best::as_auto<decltype(desc_)>;

 public:
  /// # `reflected_type::reflected`
  ///
  /// The type this is a reflection of.
  using reflected = info_t::type;

  /// # `reflected_type::name()`
  ///
  /// Returns the short name of this type.
  constexpr best::str name() const { return best::type_name<reflected>; }

  /// # `reflected_type::names()`
  ///
  /// Returns access to fully-detailed versions of the name of this type.
  constexpr best::type_names names() const {
    return best::type_names::of<reflected>;
  }

  /// # `reflected_type::tags()`
  ///
  /// Returns any tags on this type that can be selected by `Key`.
  template <typename Key>
  constexpr best::is_row auto tags(best::tlist<Key> key = {}) const {
    return desc_.tags_.select(key);
  }

  /// # `reflected_type::apply()`
  ///
  /// Calls `cb` with a pack of `best::reflected_field`s or
  /// `best::reflected_value`s for this type.
  constexpr decltype(auto) apply(auto&& cb) const;

  /// # `reflected_type::each()`
  ///
  /// Like `reflected_type::apply`, but `cb` is called one per field/value.
  constexpr void each(auto&& cb) const;

  /// # `reflected_type::match()`
  ///
  /// Finds the corresponding reflection for the item corresponding to `key` and
  /// calls `cases` with it, with the same semantics as `choice::match()`. If
  /// not found, this will call `cases` with no arguments.
  ///
  /// `key` may either be the exact name of a field, or an enum value.
  constexpr decltype(auto) match(best::str key, auto&&... cases) const;
  constexpr decltype(auto) match(reflected key, auto&&... cases) const
    requires best::is_enum<reflected>;

  /// # `reflected_type::zip_fields(...)`
  ///
  /// Zips together the fields of a bunch of references-to-`reflected`, and
  /// calls `cb` (the last argument of `args...`) on each row of fields.
  constexpr void zip_fields(auto&&... args) const
    requires best::is_struct<reflected>;

  // private:
  /// Workaround for a dumb Clang 17 conformance bug.
  template <size_t i>
  static constexpr auto item = desc_.items_[best::index<i>];

  template <typename T = reflected_type>
  static constexpr auto ChoiceTable = T{}.apply([](auto... values) {
    return std::array{best::choice<void, decltype(values)...>(best::index<0>),
                      best::choice<void, decltype(values)...>(values)...};
  });

  struct entry {
    best::str k;
    size_t v;
  };
  template <typename T = reflected_type>
  static constexpr auto Name2Index = [] {
    auto array = T{}.apply([](auto... values) {
      size_t i = 0;
      return std::array{entry{values.name(), ++i}...};
    });
    best::span(array).sort(&entry::k);
    return array;
  }();

  template <typename T = reflected_type>
  static constexpr size_t index_of(best::str name) {
    best::mark_sort_header_used();
    return best::span(Name2Index<T>)
      .bisect(name, &entry::k)
      .ok()
      .map([](auto i) { return Name2Index<T>[i].v; })
      .value_or(0);
  }
};
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
// The default BestReflect() impl.
constexpr auto BestReflect(auto& mirror, auto*)
  requires requires { mirror.infer(); }
{
  return mirror.infer();
}

template <typename I, typename W>
constexpr auto mirror<I, W>::infer() const
  requires best::is_struct<reflected> &&
           ((reflect_internal::total_fields<reflected> <=
             BEST_REFLECT_MAX_FIELDS_))
{
  return best::mirror{
    reflect_internal::tdesc<reflected>::infer_struct(),
    best::row(),
  };
}

template <typename I, typename W>
template <best::bounds range>
constexpr auto mirror<I, W>::infer() const
  requires best::is_enum<reflected> && (range.try_compute_count({}).has_value())
{
  return best::mirror{
    reflect_internal::tdesc<reflected>::template infer_enum<
      range.start, *range.try_compute_count({})>(),
    best::row(),
  };
}

template <typename I, typename W>
constexpr auto mirror<I, W>::with(auto... tags) const {
  return best::mirror{info_.add(tags...), with_};
}

template <typename I, typename W>
constexpr auto mirror<I, W>::with(best::is_member_ptr auto mp,
                                  auto... tags) const
  requires best::is_struct<reflected>
{
  return best::mirror{info_, with_.push(best::row(mp, tags...))};
}

template <typename I, typename W>
constexpr auto mirror<I, W>::with(best::same<reflected> auto e,
                                  auto... tags) const
  requires best::is_enum<reflected>
{
  return best::mirror{info_, with_.push(best::row(e, tags...))};
}

template <typename I, typename W>
constexpr auto mirror<I, W>::hide(best::is_member_ptr auto mp) const
  requires best::is_struct<reflected>
{
  return best::mirror{info_,
                      with_.push(best::row(mp, reflect_internal::hide{}))};
}

template <typename I, typename W>
constexpr auto mirror<I, W>::hide(best::same<reflected> auto e) const
  requires best::is_enum<reflected>
{
  return best::mirror{info_,
                      with_.push(best::row(e, reflect_internal::hide{}))};
}

template <auto& desc_>
constexpr decltype(auto) reflected_type<desc_>::apply(auto&& cb) const {
  if constexpr (best::is_struct<reflected>) {
    return best::indices<desc_.items_.types.size()>.apply([&]<size_t... i> {
      return best::call(BEST_FWD(cb), best::reflected_field<item<i>>{}...);
    });
  } else {
    return best::indices<desc_.items_.types.size()>.apply([&]<size_t... i> {
      return best::call(BEST_FWD(cb), best::reflected_value<item<i>>{}...);
    });
  }
}

template <auto& desc_>
constexpr void reflected_type<desc_>::each(auto&& cb) const {
  if constexpr (best::is_struct<reflected>) {
    return best::indices<desc_.items_.types.size()>.each([&]<size_t i> {
      best::call(BEST_FWD(cb), best::reflected_field<item<i>>{});
    });
  } else {
    return best::indices<desc_.items_.types.size()>.each([&]<size_t i> {
      best::call(BEST_FWD(cb), best::reflected_value<item<i>>{});
    });
  }
}

template <auto& desc_>
constexpr decltype(auto) reflected_type<desc_>::match(best::str key,
                                                      auto&&... cases) const {
  auto choice = ChoiceTable<>[index_of(key)];
  return choice.match(BEST_FWD(cases)...);
}

template <auto& desc_>
constexpr decltype(auto) reflected_type<desc_>::match(reflected value,
                                                      auto&&... cases) const
  requires best::is_enum<reflected>
{
  auto choice = apply([&](auto... values) {
    size_t idx = 0;
    ((value == values.value ? true : (++idx, false)) || ...);
    return ChoiceTable<>[idx + 1];
  });
  return choice.match(BEST_FWD(cases)...);
}

template <auto& desc_>
constexpr void reflected_type<desc_>::zip_fields(auto&&... args) const
  requires best::is_struct<reflected>
{
  auto row = best::row(best::bind, BEST_FWD(args)...);
  auto&& cb = BEST_MOVE(row).last();
  auto rest =
    BEST_MOVE(row).at(best::vals<best::bounds{.end = sizeof...(args) - 1}>);

  return each([&](auto field) -> decltype(auto) {
    return BEST_MOVE(rest).apply([&](auto&&... structs) -> decltype(auto) {
      best::call(BEST_FWD(cb), BEST_FWD(structs)->*field...);
    });
  });
}
}  // namespace best
#endif  // BEST_META_REFLECT_H_
