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

#ifndef BEST_CONTAINER_PUN_H_
#define BEST_CONTAINER_PUN_H_

#include "best/base/unsafe.h"
#include "best/container/internal/pun.h"
#include "best/container/object.h"
#include "best/meta/init.h"
#include "best/meta/tags.h"
#include "best/meta/tlist.h"

//! Untagged unions that are somewhat sensible.
//!
//! Unions in C++ are kind of messed up. Quoth cppreference:
//!
//! > If a union contains a non-static data member with a non-trivial special
//! > member function (copy/move constructor, copy/move assignment, or
//! > destructor), that function is deleted by default in the union and needs to
//! > be defined explicitly by the programmer.
//! >
//! > If a union contains a non-static data member with a non-trivial default
//! > constructor, the default constructor of the union is deleted by default
//! > unless a variant member of the union has a default member initializer.
//! >
//! > At most one variant member can have a default member initializer.
//!
//! These restrictions get in the way of metaprogramming over unions. This
//! file provides a type, `best::pun`, that implements "reasonable" behaviors
//! for a union. In particular, if any of the member types are not trivially
//! default constructable or trivially destructible, resp, `best::pun` is still
//! defaultable/destructable, but the implementations are constexpr no-ops.

namespace best {
/// # `best::pun<...>`
///
/// An untagged union, for type-punning crimes.
///
/// To construct a pun, you may either default construct it, which produces a
/// union with no selected alternative, or you must explicitly select the
/// alternative like so:
///
/// ```
/// best::pun<int, best::str> my_pun(best::index<1>, "hello!");
/// ```
///
/// Alternatives within the pun can be accessed with `pun::get()`.
///
/// Like all best containers, Alts... may be any types, which is automatically
/// wrapped in a best::object.
///
/// A `best::pun` is copyable if and only if all of its alternatives are
/// trivially copyable; otherwise, its copy constructors are deleted. The
/// destructor is always a no-op, and trivial if all of the alternatives are
/// trivially destructible.
///
/// You must manually destroy the contained alternative. This means that
/// a `best::pun<T>`, for a single T, can be used as a destructor-inhibitor.
template <typename... Alts>
class pun final {
 public:
  /// # `pun::types`
  ///
  /// A `tlist` of the alternatives in this pun.
  static constexpr auto types = best::types<Alts...>;

  /// # `pun::type<n>`
  ///
  /// Gets the nth type in this union.
  template <size_t n>
  using type = decltype(types)::template type<n>;

  // clang-format off
  template <size_t n> using cref = best::as_ref<const type<n>>;
  template <size_t n> using ref = best::as_ref<type<n>>;
  template <size_t n> using crref = best::as_rref<const type<n>>;
  template <size_t n> using rref = best::as_rref<type<n>>;
  template <size_t n> using cptr = best::as_ptr<const type<n>>;
  template <size_t n> using ptr = best::as_ptr<type<n>>;
  // clang-format on

  /// # `pun::pun()`.
  ///
  /// Constructs a union with no alternative selected.
  constexpr pun() = default;

  /// # `pun::pun(pun)`.
  ///
  /// These are deleted, unless all of `Alts` are trivially copyable, in which
  /// case this type is also trivially copyable.
  constexpr pun(const pun&) = default;
  constexpr pun& operator=(const pun&) = default;
  constexpr pun(pun&&) = default;
  constexpr pun& operator=(pun&&) = default;

  /// Constructs the `n`th alternative of this union with the given arguments.
  template <size_t n, typename... Args>
  constexpr explicit pun(best::index_t<n>, Args&&... args)
    requires best::constructible<type<n>, Args&&...>
      : BEST_PUN_IMPL_(best::index<n>, BEST_FWD(args)...) {}

  template <size_t n>
  constexpr explicit pun(best::index_t<n>, niche nh)
    requires best::has_niche<type<n>>
      : BEST_PUN_IMPL_(best::index<n>, nh) {}

  /// # `pun::get()`
  ///
  /// Gets the `n`th alternative of this union.
  ///
  /// This operation performs no checking, and can result in type confusion if
  /// not used carefully.
  template <size_t n>
  constexpr cref<n> get(unsafe u, best::index_t<n> = {}) const& {
    return *object(u, best::index<n>);
  }
  template <size_t n>
  constexpr ref<n> get(unsafe u, best::index_t<n> = {}) & {
    return *object(u, best::index<n>);
  }
  template <size_t n>
  constexpr crref<n> get(unsafe u, best::index_t<n> = {}) const&& {
    return BEST_MOVE(*object(u, best::index<n>));
  }
  template <size_t n>
  constexpr rref<n> get(unsafe u, best::index_t<n> = {}) && {
    return BEST_MOVE(*object(u, best::index<n>));
  }

  /// # `pun::object()`
  ///
  /// Like `get()`, but returns a `best::object<T>` instead.
  template <size_t n>
  constexpr const best::object<type<n>>& object(unsafe,
                                                best::index_t<n> = {}) const& {
    return impl().get(best::index<n>);
  }
  template <size_t n>
  constexpr best::object<type<n>>& object(unsafe, best::index_t<n> = {}) & {
    return impl().get(best::index<n>);
  }

  friend void BestFmt(auto& fmt, const pun& opt) { fmt.write("pun(...)"); }

 private:
  constexpr const auto& impl() const { return BEST_PUN_IMPL_; }
  constexpr auto& impl() { return BEST_PUN_IMPL_; }

 public:
  // Public so that best::pun can be structural.
  pun_internal::impl<pun_internal::info<Alts...>, Alts...> BEST_PUN_IMPL_;
#define BEST_PUB_IMPL_ _private
};
}  // namespace best

#endif  // BEST_CONTAINER_PUN_H_
