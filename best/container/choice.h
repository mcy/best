#ifndef BEST_CONTAINER_CHOICE_H_
#define BEST_CONTAINER_CHOICE_H_

#include <compare>
#include <initializer_list>

#include "best/container/internal/choice.h"
#include "best/log/internal/crash.h"
#include "best/log/location.h"
#include "best/meta/concepts.h"
#include "best/meta/init.h"
#include "best/meta/ops.h"
#include "best/meta/tags.h"

//! A sum type, like `std::variant`.
//!
//! `best::choice` is the `best` tool for manipulating a value that could be one
//! of several unrelated types. It is the foundation that some of the `best`
//! containers, such as `best::option` and `best::result`, are based on.
//!
//! `best::choice` tries to approximate the spirit of Rust enums more than
//! `std::variant`. For example, `best::choice` is never default-constructible.

namespace best {
/// # `best::choice`
///
/// A runtime choice among a list of types.
///
/// ## Construction
///
/// To construct a choice, you may perform an implicit conversion to the unique
/// alternative convertible from that value.
///
/// ```
/// best::choice<int, best::str> thing = "my value";
/// ```
///
/// If there is no unique choice, the index of the alternative must be specifed
/// explicitly.
///
/// ```
/// best::choice<int, int> some_int{best::index<1>, 42};
/// ```
///
/// ## Access
///
/// There are four options for selecting a value out of a choice.
///
/// ```
/// best::choice<int, int&, best::str> my_choice = ...;
///
/// // Crashes on the wrong alternative.
/// my_choice[best::index<1>] = 42;
///
/// // Returns best::optional; none on wrong alternative.
/// *my_choice.at(best::index<1>) = 42;
///
/// // Unchecked access! UB on wrong alternative.
/// my_choice.at(best::unsafe, best::index<1>) = 42;
///
/// // Returns a pointer; null on wrong alternative.
/// *my_choice.as_ptr(best::index<1>) = 42;
/// ```
///
/// `best::choice::match()` is like `std::visit()`, and can be used to execute
/// code depending on which alternative is present. For example:
///
/// ```
/// best::choice<int, void*> data = ...;
/// auto result = ints.match(
///   [&](int x) { ... },
///   [&](void* p) { ... }
/// );
/// ```
///
/// Different choice types are equatable/comparable if they have the same number
/// of alternatives and each respective alternative is equatable/comparable.
/// Lower-index alternatives compare as lesser than higher-index ones.
template <typename... Alts>
class choice final {
 public:
  /// # `choice::types`
  ///
  /// A `tlist` of the alternatives in this choice.
  static constexpr auto types = best::types<Alts...>;

  /// # `choice::type<n>`
  ///
  /// Gets the nth type in this choice.
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

  /// # `choice::choice(choice)`.
  ///
  /// These forward to the appropriate move/copy constructor of the
  /// corresponding alternative.
  constexpr choice(const choice&) = default;
  constexpr choice& operator=(const choice&) = default;
  constexpr choice(choice&&) = default;
  constexpr choice& operator=(choice&&) = default;

 private:
  template <typename Arg>
  static constexpr auto convert_from = types.unique_index([]<typename T> {
    return !best::void_type<T> && best::convertible<T, best::as_rref<Arg>>;
  });

 public:
  /// # `choice::choice()`
  ///
  /// Choices cannot be default-constructed.
  choice() = delete;

  /// # `choice::choice(x)`
  ///
  /// Constructs the unique alternative that can be converted to from `Arg`.
  /// If there is no such alternative, this constructor is deleted.
  ///
  /// This will never select a `void` alternative.
  template <typename Arg>
  constexpr choice(Arg&& arg)
    requires(
        // XXX: This is a load-bearing pre-check. We do not want to call
        // convert_from, which executes the whole init_from machinery, if we
        // would choose another constructor: namely, a copy/move constructor.
        // This is a common case, and avoiding it improves compile times by at
        // least a second.
        !best::same<choice, std::remove_cvref_t<Arg>> &&
        convert_from<Arg>.has_value())
      : choice(best::index<*convert_from<Arg>>, BEST_FWD(arg)) {}

  /// # `choice::choice(uninit)`
  ///
  /// Constructs an uninitialized choice. The purpose of this constructor is to
  /// enabled delayed initialization in other constructors; any operation on the
  /// resulting choice, even destruction, is Undefined Behavior. It must be
  /// `best::construct_at`-ed, instead.
  constexpr choice(best::uninit_t) {}

  /// # `choice::choice(index<n>, x)`
  ///
  /// Constructs the `n`th alternative from the given arguments, in-place.
  template <size_t n, typename... Args>
  constexpr explicit(sizeof...(Args) == 0)
      choice(best::index_t<n> tag, Args&&... args)
    requires best::constructible<type<n>, Args&&...>
      : BEST_CHOICE_IMPL_(tag, BEST_FWD(args)...) {}
  template <size_t n, typename U, typename... Args>
  constexpr choice(best::index_t<n> tag, std::initializer_list<U> il,
                   Args&&... args)
    requires best::constructible<type<n>, decltype(il), Args&&...>
      : BEST_CHOICE_IMPL_(tag, il, BEST_FWD(args)...) {}

  /// # `choice::which()`.
  ///
  /// Returns the index of the current alternative.
  constexpr size_t which() const { return impl().tag(); }

  /// # `pun[index<n>]`
  ///
  /// Returns the `n`th alternative. If `which() != n`, crashes.
  // clang-format off
  template <size_t n> constexpr cref<n> operator[](best::index_t<n> idx) const&;
  template <size_t n> constexpr ref<n> operator[](best::index_t<n> idx) &;
  template <size_t n> constexpr crref<n> operator[](best::index_t<n> idx) const&&;
  template <size_t n> constexpr rref<n> operator[](best::index_t<n> idx) &&;
  // clang-format on

  /// # `choice::at(index<n>)`
  ///
  /// Returns the `n`th alternative. If `which() != n`, returns best::none.
  // clang-format off
  template <size_t n> constexpr best::option<cref<n>> at(best::index_t<n> = {}) const&;
  template <size_t n> constexpr best::option<ref<n>> at(best::index_t<n> = {}) &;
  template <size_t n> constexpr best::option<crref<n>> at(best::index_t<n> = {}) const&&;
  template <size_t n> constexpr best::option<rref<n>> at(best::index_t<n> = {}) &&;
  // clang-format on

  /// # `choice::at(unsafe, index<n>)`
  ///
  /// Returns the `n`th alternative, without checking the tag. If
  /// `which() != n`, Undefined Behavior.
  // clang-format off
  template <size_t n> constexpr cref<n> at(unsafe, best::index_t<n> = {}) const&;
  template <size_t n> constexpr ref<n> at(unsafe, best::index_t<n> = {}) &;
  template <size_t n> constexpr crref<n> at(unsafe, best::index_t<n> = {}) const&&;
  template <size_t n> constexpr rref<n> at(unsafe, best::index_t<n> = {}) &&;
  // clang-format on

  /// # `choice::as_ptr()`
  ///
  /// Extracts the `n`th alternative as a pointer. Returns `nullptr` if
  /// `which() != n`.
  template <size_t n>
  constexpr cptr<n> as_ptr(best::index_t<n> i = {}) const {
    return unsafe::in([&](auto u) {
      return which() != n ? nullptr : impl().template ptr<n>(u);
    });
  }
  template <size_t n>
  constexpr ptr<n> as_ptr(best::index_t<n> i = {}) {
    return unsafe::in([&](auto u) {
      return which() != n ? nullptr : impl().template ptr<n>(u);
    });
  }

  /// # `choice::emplace()`
  ///
  /// Constructs a new value, in place. Returns a reference to the newly
  /// constructed value.
  template <size_t n, typename... Args>
  constexpr ref<n> emplace(best::index_t<n> t = {}, Args&&... args)
    requires best::constructible<type<n>, Args&&...>
  {
    if constexpr (best::assignable<type<n>, Args&&...>) {
      impl().template emplace<n, best::Assign>(BEST_FWD(args)...);
      return *at(t);
    }
    impl().template emplace<n, best::Construct>(BEST_FWD(args)...);
    return *at(t);
  }

  /// # `choice::match()`
  ///
  /// Calls one of `cases` (chosen by overload resolution) on the currently
  /// chosen alternative.
  ///
  /// The chosen callback is, where `n` is the current alternative, the callback
  /// that matches the first concept in this list:
  ///
  /// - `best::callable<void(best::index_t<n>, ref<n>)>`
  /// - `best::callable<void(ref<n>)>`
  ///
  /// If `choice::type<n>` is void, the list is instead:
  ///
  /// - `best::callable<void(best::index_t<n>)>`
  /// - `best::callable<void()>`
  /// - `best::callable<void(best::index_t<n>, best::empty)>`
  /// - `best::callable<void(best::empty)>`
  constexpr decltype(auto) match(auto... cases) const&;
  constexpr decltype(auto) match(auto... cases) &;
  constexpr decltype(auto) match(auto... cases) const&&;
  constexpr decltype(auto) match(auto... cases) &&;

  /// # `choice::permute()`
  ///
  /// Permutes the order of the elements in this choice according to the given
  /// permutation.
  ///
  /// For example, if we call `best::choice<A, B, C>::permute<2, 0, 1>()`, we'll
  /// get a `best::choice<C, A, B>`.
  template <
      auto... p,  //
      auto inverse = choice_internal::InvertedPermutation<types.size(), p...>>
  constexpr best::choice<type<p>...> permute(
      best::vlist<p...> permutation = {}) const&
    requires(inverse.has_value())
  {
    return match([&](auto tag, auto&&... value) {
      return best::choice<type<p>...>(best::index<(*inverse)[tag.value]>,
                                      BEST_FWD(value)...);
    });
  }
  template <
      auto... p,  //
      auto inverse = choice_internal::InvertedPermutation<types.size(), p...>>
  constexpr best::choice<type<p>...> permute(best::vlist<p...> permutation = {})
#define BEST_CHOICE_CLANG_FORMAT_HACK_ &&
      // This macro is to work around what
      // appears to be a weird clang-format bug.
      BEST_CHOICE_CLANG_FORMAT_HACK_
#undef BEST_CHOICE_CLANG_FORMAT_HACK_
    requires(inverse.has_value())
  {
    return match([&](auto tag, auto&&... value) {
      return best::choice<type<p>...>(best::index<(*inverse)[tag.value]>,
                                      BEST_FWD(value)...);
    });
  }

  friend void BestFmt(auto& fmt, const choice& ch)
    requires requires(best::object<Alts>... alts) { (fmt.format(alts), ...); }
  {
    ch.match(
        [&]<size_t n>(best::index_t<n>) { fmt.format("choice<{}>(void)", n); },
        [&](auto idx, const auto& value) {
          fmt.format("choice<{}>(", idx.value);
          fmt.format(value);
          fmt.write(")");
        });
  }

  template <typename Q>
  friend constexpr void BestFmtQuery(Q& query, choice*) {
    query.supports_width = (query.template of<Alts>.supports_width || ...);
    query.supports_prec = (query.template of<Alts>.supports_prec || ...);
    query.uses_method = [](auto r) {
      return (Q::template of<Alts>.uses_method(r) && ...);
    };
  }

  // Comparisons.
  template <typename... Us>
  BEST_INLINE_ALWAYS constexpr bool operator==(const choice<Us...>& that) const
    requires(best::equatable<Alts, Us> && ...)
  {
    return which() == that.which() &&  //
           match(
               [] { return true; },  // void case.
               [&](auto tag, const auto& value) { return value == that[tag]; });
  }

  template <typename... Us>
  BEST_INLINE_ALWAYS constexpr std::common_comparison_category_t<
      best::order_type<Alts, Us>...>
  operator<=>(const choice<Us...>& that) const
    requires(best::comparable<Alts, Us> && ...)
  {
    if (auto tags = which() <=> that.which(); tags != 0) {
      return tags;
    }

    using Output =
        std::common_comparison_category_t<best::order_type<Alts, Us>...>;

    return match(
        []() -> Output { return std::strong_ordering::equal; },  // void case.
        [&](auto tag, const auto& value) -> Output {
          return value <=> that[tag];
        });
  }

 private:
  constexpr const choice&& moved() const {
    return static_cast<const choice&&>(*this);
  }
  constexpr choice&& moved() { return static_cast<choice&&>(*this); }

  constexpr void check_ok(size_t n, best::location loc = best::here) const {
    if (best::unlikely(n != which())) {
      crash_internal::crash(
          {"attempted access of incorrect variant of best::choice; %zu != %zu",
           loc},
          n, which());
    }
  }

  constexpr const auto& impl() const& { return BEST_CHOICE_IMPL_; }
  constexpr auto& impl() & { return BEST_CHOICE_IMPL_; }
  constexpr const auto&& impl() const&& {
    return static_cast<const choice_internal::impl<Alts...>&&>(
        BEST_CHOICE_IMPL_);
  }
  constexpr auto&& impl() && {
    return static_cast<choice_internal::impl<Alts...>&&>(BEST_CHOICE_IMPL_);
  }

 public:
  choice_internal::impl<Alts...> BEST_CHOICE_IMPL_;

  // This effectively makes the above field private by making it impossible to
  // name.
#define BEST_CHOICE_IMPL_ private_
};

/// --- IMPLEMENTATION DETAILS BELOW ---

template <typename... A>
template <size_t n>
constexpr choice<A...>::cref<n> choice<A...>::operator[](
    best::index_t<n> idx) const& {
  return unsafe::in([&](auto u) -> decltype(auto) {
    return check_ok(idx.value), impl().deref(u, idx);
  });
}
template <typename... A>
template <size_t n>
constexpr choice<A...>::ref<n> choice<A...>::operator[](
    best::index_t<n> idx) & {
  return unsafe::in([&](auto u) -> decltype(auto) {
    return check_ok(idx.value), impl().deref(u, idx);
  });
}
template <typename... A>
template <size_t n>
constexpr choice<A...>::crref<n> choice<A...>::operator[](
    best::index_t<n> idx) const&& {
  return unsafe::in([&](auto u) -> decltype(auto) {
    return check_ok(idx.value), impl().move(u, idx);
  });
}
template <typename... A>
template <size_t n>
constexpr choice<A...>::rref<n> choice<A...>::operator[](
    best::index_t<n> idx) && {
  return unsafe::in([&](auto u) -> decltype(auto) {
    return check_ok(idx.value), impl().move(u, idx);
  });
}

template <typename... A>
template <size_t n>
constexpr best::option<typename choice<A...>::template cref<n>>
choice<A...>::at(best::index_t<n> i) const& {
  if (which() != n) return {};

  return unsafe::in([&](auto u) -> decltype(auto) {
    return best::invoke([&]() -> decltype(auto) { return impl().deref(u, i); });
  });
}
template <typename... A>
template <size_t n>
constexpr best::option<typename choice<A...>::template ref<n>> choice<A...>::at(
    best::index_t<n> i) & {
  if (which() != n) return {};

  return unsafe::in([&](auto u) -> decltype(auto) {
    return best::invoke([&]() -> decltype(auto) { return impl().deref(u, i); });
  });
}
template <typename... A>
template <size_t n>
constexpr best::option<typename choice<A...>::template crref<n>>
choice<A...>::at(best::index_t<n> i) const&& {
  if (which() != n) return {};

  return best::option<crref<n>>(unsafe::in([&](auto u) -> decltype(auto) {
    return best::invoke([&]() -> decltype(auto) { return impl().move(u, i); });
  }));
}
template <typename... A>
template <size_t n>
constexpr best::option<typename choice<A...>::template rref<n>>
choice<A...>::at(best::index_t<n> i) && {
  if (which() != n) return {};

  return best::option<rref<n>>(unsafe::in([&](auto u) -> decltype(auto) {
    return best::invoke([&]() -> decltype(auto) { return impl().move(u, i); });
  }));
}

template <typename... A>
template <size_t n>
constexpr choice<A...>::cref<n> choice<A...>::at(best::unsafe u,
                                                 best::index_t<n> i) const& {
  return best::assume(which() == n), impl().deref(u, i);
}
template <typename... A>
template <size_t n>
constexpr choice<A...>::ref<n> choice<A...>::at(best::unsafe u,
                                                best::index_t<n> i) & {
  return best::assume(which() == n), impl().deref(u, i);
}
template <typename... A>
template <size_t n>
constexpr choice<A...>::crref<n> choice<A...>::at(best::unsafe u,
                                                  best::index_t<n> i) const&& {
  return best::assume(which() == n), impl().move(u, i);
}
template <typename... A>
template <size_t n>
constexpr choice<A...>::rref<n> choice<A...>::at(best::unsafe u,
                                                 best::index_t<n> i) && {
  return best::assume(which() == n), impl().move(u, i);
}

template <typename... A>
constexpr decltype(auto) choice<A...>::match(auto... cases) const& {
  return impl().match(choice_internal::Overloaded(BEST_FWD(cases)...));
}
template <typename... A>
constexpr decltype(auto) choice<A...>::match(auto... cases) & {
  return impl().match(choice_internal::Overloaded(BEST_FWD(cases)...));
}
template <typename... A>
constexpr decltype(auto) choice<A...>::match(auto... cases) const&& {
  return moved().impl().match(choice_internal::Overloaded(BEST_FWD(cases)...));
}
template <typename... A>
constexpr decltype(auto) choice<A...>::match(auto... cases) && {
  return moved().impl().match(choice_internal::Overloaded(BEST_FWD(cases)...));
}
}  // namespace best

#endif  // BEST_CONTAINER_CHOICE_H_