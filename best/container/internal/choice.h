#ifndef BEST_CONTAINER_INTERNAL_CHOICE_H_
#define BEST_CONTAINER_INTERNAL_CHOICE_H_

#include <array>
#include <cstddef>
#include <type_traits>

#include "best/container/internal/pun.h"
#include "best/container/object.h"
#include "best/container/pun.h"
#include "best/math/int.h"
#include "best/meta/init.h"
#include "best/meta/internal/init.h"
#include "best/meta/tags.h"

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

template <typename... Ts>
class niched {
 public:
  static_assert(sizeof...(Ts) == 2);

  static constexpr auto types = best::types<Ts...>;
  template <size_t n>
  using type = decltype(types)::template type<n>;

  static constexpr size_t NonEmpty =
      *types.find([]<typename T> { return best::has_niche<T>; });
  static constexpr size_t Empty = NonEmpty == 0 ? 1 : 0;

  constexpr niched() = default;
  constexpr niched(const niched&) = default;
  constexpr niched& operator=(const niched&) = default;
  constexpr niched(niched&&) = default;
  constexpr niched& operator=(niched&&) = default;

  template <typename... Args>
  constexpr explicit niched(best::index_t<Empty>, Args&&... args)
    requires best::constructible<type<Empty>, trivially, Args&&...>
      : non_empty_(best::index<0>, best::niche{}) {}

  template <typename... Args>
  constexpr explicit niched(best::index_t<NonEmpty>, Args&&... args)
    requires best::constructible<type<NonEmpty>, Args&&...>
      : non_empty_(best::index<0>, BEST_FWD(args)...) {}

  constexpr size_t tag() const {
    return non_empty_
                   .object(unsafe("the non-empty variant is always engaged,"
                                  "except when after destructor runs"),
                           index<0>)
                   .as_ptr()
                   .is_niche()
               ? Empty
               : NonEmpty;
  }

  constexpr const best::object<type<Empty>>& get(unsafe,
                                                 best::index_t<Empty>) const {
    return empty_;
  }
  constexpr best::object<type<Empty>>& get(unsafe, best::index_t<Empty>) {
    return empty_;
  }
  constexpr const best::object<type<NonEmpty>>& get(
      unsafe u, best::index_t<NonEmpty>) const {
    return non_empty_.object(u, index<0>);
  }
  constexpr best::object<type<NonEmpty>>& get(unsafe u,
                                              best::index_t<NonEmpty>) {
    return non_empty_.object(u, index<0>);
  }

  best::pun<type<NonEmpty>> non_empty_;
  [[no_unique_address]] best::object<type<Empty>> empty_;
};

template <typename... Ts>
tagged<Ts...> which_storage(best::tlist<Ts...>, best::rank<0>);

template <typename A, typename B>
niched<A, B> which_storage(best::tlist<A, B>, best::rank<1>)
  requires(has_niche<A> && best::is_empty<B> &&
           best::constructible<B, trivially>) ||
          (has_niche<B> && best::is_empty<A> &&
           best::constructible<A, trivially>);

template <typename... Ts>
using storage = decltype(which_storage(types<Ts...>, best::rank<1>{}));

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
    requires(info.trivial_copy)
  = default;
  constexpr impl(const impl& that)
    requires(!info.trivial_copy)
  {
    that.match([&](auto tag, auto&... value) {
      std::construct_at(this, tag, value...);
    });
  }

  constexpr impl& operator=(const impl&)
    requires(info.trivial_copy)
  = default;
  constexpr impl& operator=(const impl& that)
    requires(!info.trivial_copy)
  {
    that.match([&](auto tag, auto&... value) { emplace<tag.value>(value...); });
    return *this;
  }

  constexpr impl(impl&&)
    requires(info.trivial_move)
  = default;
  constexpr impl(impl&& that)
    requires(!info.trivial_move)
  {
    that.match([&](auto tag, auto&... value) {
      std::construct_at(this, tag, BEST_MOVE(value)...);
    });
  }

  constexpr impl& operator=(impl&&)
    requires(info.trivial_move)
  = default;
  constexpr impl& operator=(impl&& that)
    requires(!info.trivial_move)
  {
    that.match([&](auto tag, auto&... value) {
      emplace<tag.value>(BEST_MOVE(value)...);
    });
    return *this;
  }

  constexpr ~impl()
    requires(info.trivial_dtor)
  = default;
  constexpr ~impl()
    requires(!info.trivial_dtor)
  {
    match([](auto, auto&... value) {
      (std::destroy_at(best::addr(value)), ...);
    });
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
    return JumpTable<decltype(make_match_arm(*this,
                                             BEST_FWD(callback)))>[tag()](
        make_match_arm(*this, BEST_FWD(callback)));
  }
  template <typename F>
  BEST_INLINE_SYNTHETIC constexpr decltype(auto) match(F&& callback) & {
    return JumpTable<decltype(make_match_arm(*this,
                                             BEST_FWD(callback)))>[tag()](
        make_match_arm(*this, BEST_FWD(callback)));
  }
  template <typename F>
  BEST_INLINE_SYNTHETIC constexpr decltype(auto) match(F&& callback) const&& {
    return JumpTable<decltype(make_match_arm(BEST_MOVE(*this),
                                             BEST_FWD(callback)))>[tag()](
        make_match_arm(BEST_MOVE(*this), BEST_FWD(callback)));
  }
  template <typename F>
  BEST_INLINE_SYNTHETIC constexpr decltype(auto) match(F&& callback) && {
    return JumpTable<decltype(make_match_arm(BEST_MOVE(*this),
                                             BEST_FWD(callback)))>[tag()](
        make_match_arm(BEST_MOVE(*this), BEST_FWD(callback)));
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
  constexpr static auto make_match_arm(auto&& self, F&& callback) {
    return [&]<size_t n>(best::index_t<n> tag) -> decltype(auto) {
      using I = decltype(tag);
      using Type = best::refcopy<type<n>, decltype(self)&&>;

      // This needs to be dependent on tag, even trivially, so that the call
      // of callback(empty) in one of the if statements below uses two-phase
      // lookup.
      using Empty = best::refcopy<
          std::conditional_t<sizeof(tag) == 0, best::empty, best::empty>,
          decltype(self)&&>;

      if constexpr (best::is_void<Type>) {
        if constexpr (best::callable<F, void(I)>) {
          return best::call(BEST_FWD(callback), tag);
        } else if constexpr (best::callable<F, void()>) {
          return best::call(BEST_FWD(callback));
        } else if constexpr (best::callable<F, void(I, Empty)>) {
          best::empty arg;
          return best::call(BEST_FWD(callback), tag, static_cast<Empty>(arg));
        } else {
          best::empty arg;
          return best::call(BEST_FWD(callback), static_cast<Empty>(arg));
        }
      } else {
        unsafe u(
            "this function is only called after"
            "checking tag() via the jump table.");
        if constexpr (best::callable<F, void(I, Type)>) {
          return best::call(BEST_FWD(callback), tag,
                            static_cast<Type>(*self.get(u, tag)));
        } else {
          return best::call(BEST_FWD(callback),
                            static_cast<Type>(*self.get(u, tag)));
        }
      }
    };
  }

  template <typename F>
  static constexpr auto JumpTable =
      make_jump_table<F>(std::make_index_sequence<sizeof...(Ts)>{});
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