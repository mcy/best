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

#ifndef BEST_CONTAINER_INTERNAL_CHOICE_H_
#define BEST_CONTAINER_INTERNAL_CHOICE_H_

#include <array>
#include <cstddef>

#include "best/base/tags.h"
#include "best/container/internal/pun.h"
#include "best/container/object.h"
#include "best/container/pun.h"
#include "best/math/int.h"
#include "best/meta/init.h"
#include "best/meta/internal/init.h"

//! Internal implementation of best::choice.

namespace best::choice_internal {
template <typename... Ts>
class tagged {
 public:
  static constexpr auto types = best::types<Ts...>;
  template <size_t n>
  using type = decltype(types)::template type<n>;

  constexpr tagged() = default;
  constexpr tagged(const tagged&) = default;
  constexpr tagged& operator=(const tagged&) = default;
  constexpr tagged(tagged&&) = default;
  constexpr tagged& operator=(tagged&&) = default;

  template <size_t n, typename... Args>
  constexpr explicit tagged(best::index_t<n>, Args&&... args)
    requires best::constructible<type<n>, Args&&...>
      : union_(best::index<n>, BEST_FWD(args)...), tag_(n) {}

  constexpr size_t tag() const { return tag_; }

  template <size_t n>
  constexpr const best::object<type<n>>& get(unsafe u, best::index_t<n>) const {
    return union_.object(u, best::index<n>);
  }
  template <size_t n>
  constexpr best::object<type<n>>& get(unsafe u, best::index_t<n>) {
    return union_.object(u, best::index<n>);
  }

  best::pun<Ts...> union_;
  best::smallest_uint_t<sizeof...(Ts)> tag_{};
};

template <size_t swap, typename Empty, typename Niched>
class niched {
 public:
  static constexpr auto types = best::types<Empty, Niched>;
  template <size_t n>
  using type = decltype(types)::template type<swap ^ n>;

  // Avoid making a one-element pun if we can avoid it.
  static constexpr bool NeedPun = !best::destructible<Niched, trivially>;
  static constexpr auto ctor_tag() {
    if constexpr (NeedPun) {
      return best::index<0>;
    } else {
      return best::in_place;
    }
  }

  constexpr niched() = default;
  constexpr niched(const niched&) = default;
  constexpr niched& operator=(const niched&) = default;
  constexpr niched(niched&&) = default;
  constexpr niched& operator=(niched&&) = default;

  template <typename... Args>
  constexpr explicit niched(best::index_t<swap ^ 0>, Args&&... args)
    requires best::constructible<Empty, trivially, Args&&...>
      : niched_(ctor_tag(), best::niche{}) {}

  template <typename... Args>
  constexpr explicit niched(best::index_t<swap ^ 1>, Args&&... args)
    requires best::constructible<Niched, Args&&...>
      : niched_(ctor_tag(), BEST_FWD(args)...) {}

  constexpr size_t tag() const {
    return swap ^ !get(unsafe("we're checking for the niche, so we need "
                              "to pull out the non-empty side"),
                       best::index<swap ^ 1>)
                       .as_ptr()
                       .is_niche();
  }

  template <size_t n>
  constexpr const auto& get(unsafe, best::index_t<n>) const {
    if constexpr ((swap ^ n) == 0) {
      return empty_;
    } else if constexpr (NeedPun) {
      return niched_.object(unsafe("the non-empty variant is always engaged,"
                                   "except when after destructor runs"),
                            index<0>);
    } else {
      return niched_;
    }
  }
  template <size_t n>
  constexpr auto& get(unsafe, best::index_t<n>) {
    if constexpr ((swap ^ n) == 0) {
      return empty_;
    } else if constexpr (NeedPun) {
      return niched_.object(unsafe("the non-empty variant is always engaged,"
                                   "except when after destructor runs"),
                            index<0>);
    } else {
      return niched_;
    }
  }

  best::select<NeedPun, best::pun<Niched>, best::object<Niched>> niched_;
  [[no_unique_address]] best::object<Empty> empty_;
};

template <typename... Ts>
tagged<Ts...> which_storage(best::tlist<Ts...>, best::rank<0>);

template <typename A, typename B>
niched<1, B, A> which_storage(best::tlist<A, B>, best::rank<1>)
  requires(has_niche<A> && best::is_empty<B> &&
           best::constructible<B, trivially>);

template <typename A, typename B>
niched<0, A, B> which_storage(best::tlist<A, B>, best::rank<2>)
  requires(has_niche<B> && best::is_empty<A> &&
           best::constructible<A, trivially>);

template <typename... Ts>
using storage = decltype(which_storage(types<Ts...>, best::rank<2>{}));

template <typename... Ts>
class impl : public storage<Ts...> {
 private:
  static constexpr auto info = best::pun_internal::info<Ts...>;

 public:
  template <size_t n>
  using type = storage<Ts...>::template type<n>;

  using Base = storage<Ts...>;
  using Base::Base;
  using Base::get;
  using Base::tag;

  constexpr impl() = default;

  constexpr impl(const impl&)
    requires(info.trivial_copy())
  = default;
  constexpr impl(const impl& that)
    requires(!info.trivial_copy())
  {
    that.index_match([&](auto tag, auto&... value) {
      std::construct_at(this, tag, value...);
    });
  }

  constexpr impl& operator=(const impl&)
    requires(info.trivial_copy())
  = default;
  constexpr impl& operator=(const impl& that)
    requires(!info.trivial_copy())
  {
    that.index_match(
        [&](auto tag, auto&... value) { emplace<tag.value>(value...); });
    return *this;
  }

  constexpr impl(impl&&)
    requires(info.trivial_move())
  = default;
  constexpr impl(impl&& that)
    requires(!info.trivial_move())
  {
    that.index_match([&](auto tag, auto&... value) {
      std::construct_at(this, tag, BEST_MOVE(value)...);
    });
  }

  constexpr impl& operator=(impl&&)
    requires(info.trivial_move())
  = default;
  constexpr impl& operator=(impl&& that)
    requires(!info.trivial_move())
  {
    that.index_match([&](auto tag, auto&... value) {
      emplace<tag.value>(BEST_MOVE(value)...);
    });
    return *this;
  }

  constexpr ~impl()
    requires(info.trivial_dtor())
  = default;
  constexpr ~impl()
    requires(!info.trivial_dtor())
  {
    match([](auto&... value) { (std::destroy_at(best::addr(value)), ...); });
  }

  template <size_t which, typename... Args>
  constexpr void emplace(Args&&... args) {
    if (which == tag()) {
      if constexpr (best::is_object<type<which>> &&
                    best::assignable<type<which>, Args&&...>) {
        get(unsafe{"checked tag() before this"}, best::index<which>)
            .as_ptr()
            .assign(BEST_FWD(args)...);
        return;
      }
    }
    std::destroy_at(this);
    std::construct_at(this, best::index<which>, BEST_FWD(args)...);
  }

  template <size_t n>
  BEST_INLINE_SYNTHETIC constexpr best::as_ref<const type<n>> deref(
      unsafe u, best::index_t<n> i = {}) const {
    return *get(u, i);
  }
  template <size_t n>
  BEST_INLINE_SYNTHETIC constexpr best::as_ref<type<n>> deref(
      unsafe u, best::index_t<n> i = {}) {
    return *get(u, i);
  }
  template <size_t n>
  BEST_INLINE_SYNTHETIC constexpr best::as_ptr<type<n>> ptr(
      unsafe u, best::index_t<n> i = {}) const {
    return get(u, i).operator->();
  }
  template <size_t n>
  BEST_INLINE_SYNTHETIC constexpr best::as_ptr<type<n>> ptr(
      unsafe u, best::index_t<n> i = {}) {
    return get(u, i).operator->();
  }

  template <size_t n>
  BEST_INLINE_SYNTHETIC constexpr const best::object<type<n>>& object(
      unsafe u, best::index_t<n> i = {}) const {
    return get(u, i);
  }
  template <size_t n>
  BEST_INLINE_SYNTHETIC constexpr best::object<type<n>>& object(
      unsafe u, best::index_t<n> i = {}) {
    return get(u, i);
  }

  template <typename F>
  BEST_INLINE_SYNTHETIC constexpr decltype(auto) match(F&& callback) const& {
    return jump_table(make_match_arm(*this, BEST_FWD(callback)));
  }
  template <typename F>
  BEST_INLINE_SYNTHETIC constexpr decltype(auto) match(F&& callback) & {
    return jump_table(make_match_arm(*this, BEST_FWD(callback)));
  }
  template <typename F>
  BEST_INLINE_SYNTHETIC constexpr decltype(auto) match(F&& callback) const&& {
    return jump_table(make_match_arm(BEST_MOVE(*this), BEST_FWD(callback)));
  }
  template <typename F>
  BEST_INLINE_SYNTHETIC constexpr decltype(auto) match(F&& callback) && {
    return jump_table(make_match_arm(BEST_MOVE(*this), BEST_FWD(callback)));
  }

  template <typename F>
  BEST_INLINE_SYNTHETIC constexpr decltype(auto) index_match(
      F&& callback) const& {
    return jump_table(make_index_match_arm(*this, BEST_FWD(callback)));
  }
  template <typename F>
  BEST_INLINE_SYNTHETIC constexpr decltype(auto) index_match(F&& callback) & {
    return jump_table(make_index_match_arm(*this, BEST_FWD(callback)));
  }
  template <typename F>
  BEST_INLINE_SYNTHETIC constexpr decltype(auto) index_match(
      F&& callback) const&& {
    return jump_table(make_index_match_arm(*this, BEST_FWD(callback)));
  }
  template <typename F>
  BEST_INLINE_SYNTHETIC constexpr decltype(auto) index_match(F&& callback) && {
    return jump_table(make_index_match_arm(*this, BEST_FWD(callback)));
  }

 private:
  template <typename F, size_t... i>
  constexpr static auto make_jump_table(std::index_sequence<i...>) {
    // TODO(mcyoung): It'd be nice to use common_type here...
    using Output = best::call_result<F, best::index_t<0>>;

    return std::array<Output (*)(F&&), sizeof...(Ts)>{
        {+[](F&& callback) -> Output {
          return best::call(BEST_FWD(callback), best::index<i>);
        }...}};
  }

  template <typename F>
  static constexpr auto JumpTable =
      make_jump_table<F>(std::make_index_sequence<sizeof...(Ts)>{});

  constexpr decltype(auto) jump_table(auto&& arms) const
    requires(sizeof...(Ts) > 2)
  {
    return JumpTable<decltype(arms)>[tag()](BEST_FWD(arms));
  }

  constexpr decltype(auto) jump_table(auto&& arms) const
    requires(sizeof...(Ts) == 2)
  {
    switch (tag()) {
      case 0:
        return arms(best::index<0>);
      case 1:
        return arms(best::index<1>);
      default:
        best::unreachable();
    }
  }

  constexpr decltype(auto) jump_table(auto&& arms) const
    requires(sizeof...(Ts) == 1)
  {
    return arms(best::index<0>);
  }

  constexpr static auto make_match_arm(auto&& self, auto&& cb) {
    return [&]<size_t n>(best::index_t<n> tag) -> decltype(auto) {
      using Type = best::refcopy<type<n>, decltype(self)&&>;

      // This needs to be dependent on tag, even trivially, so that the call
      // of callback(empty) in one of the if statements below uses two-phase
      // lookup.
      using Empty = best::dependent<best::empty, decltype(tag)>;

      if constexpr (best::is_void<Type>) {
        if constexpr (best::callable<decltype(cb), void()>) {
          return best::call(BEST_FWD(cb));
        } else {
          best::empty arg;
          return best::call(BEST_FWD(cb), static_cast<Empty>(arg));
        }
      } else {
        unsafe u(
            "this function is only called after"
            "checking tag() via the jump table.");
        return best::call(BEST_FWD(cb), static_cast<Type>(*self.get(u, tag)));
      }
    };
  }

  constexpr static auto make_index_match_arm(auto&& self, auto&& cb) {
    return [&]<size_t n>(best::index_t<n> tag) -> decltype(auto) {
      using Type = best::refcopy<type<n>, decltype(self)&&>;

      // This needs to be dependent on tag, even trivially, so that the call
      // of callback(empty) in one of the if statements below uses two-phase
      // lookup.
      using Empty = best::dependent<best::empty, decltype(tag)>;

      if constexpr (best::is_void<Type>) {
        if constexpr (best::callable<decltype(cb), void(decltype(tag))>) {
          return best::call(BEST_FWD(cb), tag);
        } else {
          best::empty arg;
          return best::call(BEST_FWD(cb), tag, static_cast<Empty>(arg));
        }
      } else {
        unsafe u(
            "this function is only called after"
            "checking tag() via the jump table.");
        return best::call(BEST_FWD(cb), tag,
                          static_cast<Type>(*self.get(u, tag)));
      }
    };
  }
};

template <typename... Fs>
struct Overloaded : public Fs... {
  using Fs::operator()...;
};
template <typename... Fs>
Overloaded(Fs&&...) -> Overloaded<Fs...>;

template <size_t n, auto... p>
inline constexpr auto InvertedPermutation =
    []() -> container_internal::option<std::array<size_t, n>> {
  std::array<size_t, n> inverse;
  for (auto& elem : inverse) {
    elem = ~size_t{0};
  }
  std::array perm{p...};
  for (size_t i = 0; i < perm.size(); ++i) {
    if (perm[i] >= n) {
      return {};
    }

    if (inverse[perm[i]] != ~size_t{0}) {
      return {};
    }
    inverse[perm[i]] = i;
  }
  for (auto& elem : inverse) {
    if (elem == ~size_t{0}) {
      return {};
    }
  }
  return inverse;
}();

}  // namespace best::choice_internal

#endif  // BEST_CONTAINER_INTERNAL_CHOICE_H_
