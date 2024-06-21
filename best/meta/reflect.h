#ifndef BEST_META_REFLECT_H_
#define BEST_META_REFLECT_H_

#include "best/container/row.h"
#include "best/meta/internal/reflect.h"
#include "best/meta/taxonomy.h"
#include "best/text/str.h"

//! Struct and enum reflection.
//!
//! `best` provides a mechanism for reflecting the fields and variants of
//! user-defined structs and enums, which must define the `BestReflect()`
//! FTADLE:
//!
//! ```
//! friend constexpr auto BestReflect(auto& mirror, MyStruct*) {
//!   return mirror.reflect(
//!     "MyStruct",
//!     mirror.field("foo", &MyStruct::foo),
//!     mirror.field("bar", &MyStruct::bar),
//!   );
//! }
//! ```

namespace best {

/// # `best::reflected`, `best::is_reflected_struct`,
///   `best::is_reflected_enum`
///
/// Whether `T` can be reflected, i.e., it is a struct or enum type that has a
/// working `BestReflect()` FTADLE implementation.
template <typename T>
concept reflected = requires(const best::mirror& m, best::as_auto<T>* ptr) {
  {
    BestReflect(m, ptr)
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
    best::reflected_type<reflect_internal::info<best::as_auto<T>>>{};

/// # `best::mirror`
///
/// A value of this type is passed to the `BestReflect` FTADLE. This type
/// cannot be constructed by users and exists only for exposition.
class mirror final {
 public:
  /// # `mirror()`
  ///
  /// Reflects that there exists a type with the given name, the given members,
  /// and optionally, a row of tag types.
  constexpr auto operator()(best::str name, auto... members) const;
  constexpr auto operator()(best::str name, best::is_row auto tags,
                            auto... members) const;

  /// # `mirror.field()`
  ///
  /// Reflects that `Struct` has a data member with the given name, the
  /// given type, and optionally, a row of tag types. The result of this call
  /// should be passed to `reflect()`.
  template <best::is_struct Struct, best::is_object Type>
  constexpr auto field(best::str name, Type Struct::*member,
                       auto... tags) const;

  /// # `mirror.value()`
  ///
  /// Reflects that Enum `Struct` has an enumerator with the given name, the
  /// given value, and optionally, a row of tag types. The result of this call
  /// should be passed to `reflect()`.
  template <best::is_enum Enum>
  constexpr auto value(best::str name, Enum value, auto... tags) const;

  mirror(const mirror&) = delete;
  mirror& operator=(const mirror&) = delete;

 private:
  constexpr mirror() = default;

 public:
  static const mirror BEST_MIRROR_FTADLE_;
};
inline constexpr mirror mirror::BEST_MIRROR_FTADLE_{};
#define BEST_MIRROR_FTADLE_ _private

/// # `best::reflected_field`
///
/// A field of some reflected struct. The type parameter is an implementation
/// detail; this type cannot be instantiated by users.
///
/// A `best::reflected_field` offers accessors for information about the field,
/// such as its name, access to a member-to-ptr, and so on.
///
/// A field can also be used to offset into a struct, with the syntax
/// `my_struct->*field`. `my_struct` can be a reference or pointer to
/// the reflected type, or a dereferenceable type that returns one.
template <auto& info_>
class reflected_field final {
 private:
  static_assert(info_.Kind == reflect_internal::Field,
                "cannot instantiate best::reflected_field directly");
  using info_t = best::as_auto<decltype(info_)>;

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
  constexpr best::str name() const { return info_.name_; }

  /// # `reflected_field::tags()`
  ///
  /// Returns any tags on this field that can be selected by `Key`.
  template <typename Key>
  constexpr best::is_row auto tags(best::tlist<Key> key = {}) const {
    return info_.tags_.select(key);
  }

  /// # `reflected_field::as_offset()`
  ///
  /// Returns a pointer-to-member that represents this field.
  constexpr type reflected::*as_offset() const { return info_.ptr_; }

  constexpr friend decltype(auto) operator->*(auto&& r, reflected_field f)
    requires best::same<best::as_auto<decltype(r)>, reflected>
  {
    return BEST_FWD(r).*(f.as_offset());
  }
  constexpr friend decltype(auto) operator->*(
      best::is_deref<reflected> auto&& r, reflected_field f) {
    return (*BEST_FWD(r)).*(f.as_offset());
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
template <auto& info_>
class reflected_value final {
 private:
  static_assert(info_.Kind == reflect_internal::Value,
                "cannot instantiate best::reflected_value directly");
  using info_t = best::as_auto<decltype(info_)>;

 public:
  /// # `reflected_field::reflected`
  ///
  /// The type this field was reflected from.
  using reflected = info_t::enum_;

  /// # `reflected_field::value`
  ///
  /// The actual reflected value.
  static constexpr reflected value = info_.elem_;

  /// # `reflected_field::name()`
  ///
  /// Returns the name of this field as specified in the `BestReflect()` call.
  constexpr best::str name() const { return info_.name_; }

  /// # `reflected_field::tags()`
  ///
  /// Returns any tags on this field that can be selected by `Key`.
  template <typename Key>
  constexpr best::is_row auto tags(best::tlist<Key> key = {}) const {
    return info_.tags_.select(key);
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
template <auto& info_>
class reflected_type final {
 private:
  static_assert(info_.Kind == reflect_internal::Struct ||
                    info_.Kind == reflect_internal::Enum ||
                    info_.Kind == reflect_internal::NoFields,
                "cannot instantiate best::field directly");
  using info_t = best::as_auto<decltype(info_)>;

 public:
  /// # `reflected_type::reflected`
  ///
  /// The type this is a reflection of.
  using reflected =
      best::select<info_.Kind == reflect_internal::Struct,
                   typename info_t::struct_, typename info_t::enum_>;

  /// # `reflected_type::name()`
  ///
  /// Returns the name of this type as specified in the `BestReflect()` call.
  constexpr best::str name() const { return info_.name_; }

  /// # `reflected_type::find(member)`
  ///
  /// Looks up the field or value corresponding to a particular member. This
  /// the member should be a pointer-to-member if this is a reflects a struct,
  /// or a value of the underlying enum if this reflects an enum.
  ///
  /// It will then call `cb` with the appropriate value. Internally, it has the
  /// semantics of calling `best::choice::match()` on a choice that can contain
  /// any of the possible reflected field/value types produced by fields() or
  /// values(), respectively. If no element matches, it is as if the selected
  /// element has type `void`.
  ///
  /// Hence, the best way to use this function is to write something like this.
  ///
  /// ```
  /// auto name = best::reflect<T>.find(xyz,
  ///   [](auto value) { return value.name() },
  ///   [] { return best::str("<unknown>"); });
  /// ```
  constexpr decltype(auto) find(auto member, auto&&... cases) const;

  /// # `reflected_type::tags()`
  ///
  /// Returns any tags on this type that can be selected by `Key`.
  template <typename Key>
  constexpr best::is_row auto tags(best::tlist<Key> key = {}) const {
    return info_.tags_.select(key);
  }

  /// # `reflected_type::fields()`
  ///
  /// Calls `cb` with a pack of `best::reflected_field`s for this type.
  constexpr decltype(auto) fields(auto&& cb) const
    requires best::is_struct<reflected>;

  /// # `reflected_type::each_field(...)`
  ///
  /// Zips together the fields of a bunch of references-to-`reflected`, and
  /// calls `cb` (the last argument of `args...`) on each row of fields.
  constexpr void each_field(auto&&... args) const
    requires best::is_struct<reflected>;

  /// # `reflected_type::values()`
  ///
  /// Calls `cb` with a pack of `best::reflected_values`s for this type.
  constexpr decltype(auto) values(auto&& cb) const
    requires best::is_enum<reflected>;

  constexpr reflected_type() = default;

 private:
  /// Workaround for a dumb Clang 17 conformance bug.
  template <size_t i>
  static constexpr auto item = info_.items_[best::index<i>];
};

/// # `best::fields()`
///
/// Extracts all of the fields out of a reflected struct and creates a row
/// of references of them.
constexpr auto fields(best::is_reflected_struct auto&& value) {
  return best::reflect<decltype(value)>.fields(
      [&](auto... f) { return best::row(best::bind, value->*f...); });
}
}  // namespace best

/******************************************************************************/

///////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! /////////////////////

/******************************************************************************/

namespace best {
constexpr auto mirror::operator()(best::str name, best::is_row auto tags,
                                  auto... members) const {
  if constexpr (sizeof...(members) == 0) {
    return reflect_internal::no_fields<best::as_auto<decltype(tags)>>{name,
                                                                      tags};
  } else if constexpr (((members.Kind == reflect_internal::Field) && ...)) {
    static_assert(
        best::same<typename best::as_auto<decltype(members)>::struct_...>,
        "all fields passed to a best::mirror must have the same "
        "struct type");
    using Struct =
        best::tlist<typename best::as_auto<decltype(members)>::struct_...>::
            template type<0>;
    return best::reflect_internal::struct_info<
        Struct, best::as_auto<decltype(tags)>,
        best::as_auto<decltype(members)>...>{name, tags, {members...}};
  } else if constexpr (((members.Kind == reflect_internal::Value) && ...)) {
    static_assert(
        best::same<typename best::as_auto<decltype(members)>::enum_...>,
        "all values passed to best::mirror must have the same "
        "enum type");
    using Enum = best::tlist<
        typename best::as_auto<decltype(members)>::enum_...>::template type<0>;
    return best::reflect_internal::enum_info<
        Enum, best::as_auto<decltype(tags)>,
        best::as_auto<decltype(members)>...>{name, tags, {members...}};
  } else {
    static_assert(sizeof...(members) == 0,
                  "passed invalid member values into the mirror");
  }
}
constexpr auto mirror::operator()(best::str name, auto... members) const {
  return operator()(name, row(), members...);
}

template <best::is_struct Struct, best::is_object Type>
constexpr auto mirror::field(best::str name, Type Struct::*member,
                             auto... tags) const {
  return best::reflect_internal::field_info<Struct, Type,
                                            best::as_auto<decltype(tags)>...>{
      name, member, {tags...}};
}

template <best::is_enum Enum>
constexpr auto mirror::value(best::str name, Enum value, auto... tags) const {
  return best::reflect_internal::elem_info<Enum,
                                           best::as_auto<decltype(tags)>...>{
      name, value, {tags...}};
}

template <auto& info_>
constexpr decltype(auto) reflected_type<info_>::find(auto member,
                                                     auto&&... cases) const {
  if constexpr (best::is_struct<reflected>) {
    return fields([&](auto... fields) {
      using Ch = best::choice<void, decltype(fields)...>;
      Ch ch(best::index<0>);

      ((best::equal(member, fields.as_offset()) ? void(ch = Ch(fields))
                                                : void()),
       ...);
      return ch.match(BEST_FWD(cases)...);
    });
  } else {
    return values([&](auto... values) {
      using Ch = best::choice<void, decltype(values)...>;
      Ch ch(best::index<0>);

      ((best::equal(member, values.value) ? void(ch = Ch(values)) : void()),
       ...);
      return ch.match(BEST_FWD(cases)...);
    });
  }
}

template <auto& info_>
constexpr decltype(auto) reflected_type<info_>::fields(auto&& cb) const
  requires best::is_struct<reflected>
{
  return best::indices<info_.items_.types.size()>.apply([&]<typename... I> {
    return best::call(BEST_FWD(cb), best::reflected_field<item<I::value>>{}...);
  });
}

template <auto& info_>
constexpr void reflected_type<info_>::each_field(auto&&... args) const
  requires best::is_struct<reflected>
{
  if constexpr (sizeof...(args) == 1) {
    fields([&](auto... fields) { (best::call(args..., fields), ...); });
  } else {
    best::row<decltype(args)...> row(BEST_FWD(args)...);
    each_field([&](auto field) {
      best::indices<info_.items_.types.size() - 1>.apply([&]<typename... I> {
        best::call(BEST_MOVE(row).last(),
                   BEST_MOVE(row)[best::index<I::value>]->*field...);
      });
    });
  }
}

template <auto& info_>
constexpr decltype(auto) reflected_type<info_>::values(auto&& cb) const
  requires best::is_enum<reflected>
{
  return best::indices<info_.items_.types.size()>.apply([&]<typename... I> {
    return best::call(BEST_FWD(cb), best::reflected_value<item<I::value>>{}...);
  });
}
}  // namespace best
#endif  // BEST_META_REFLECT_H_