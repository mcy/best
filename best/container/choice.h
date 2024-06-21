#ifndef BEST_CONTAINER_CHOICE_H_
#define BEST_CONTAINER_CHOICE_H_

#include <initializer_list>

#include "best/base/ord.h"
#include "best/container/internal/choice.h"
#include "best/log/internal/crash.h"
#include "best/log/location.h"
#include "best/meta/init.h"
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
  static constexpr auto convert_from = types.find_unique([]<typename T> {
    return !best::is_void<T> && best::convertible<T, best::as_rref<Arg>>;
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
        // least a second().
        !best::same<choice, best::as_auto<Arg>> &&
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

  /// # `choice::object(index<n>)`
  ///
  /// Returns the `n`th alternative as a reference to a `best::object`. If
  /// `which() != n`, returns best::none.
  // clang-format off
  template <size_t n> constexpr best::option<const best::object<type<n>>&> object(best::index_t<n> = {}) const&;
  template <size_t n> constexpr best::option<best::object<type<n>>&> object(best::index_t<n> = {}) &;
  template <size_t n> constexpr best::option<const best::object<type<n>>&&> object(best::index_t<n> = {}) const&&;
  template <size_t n> constexpr best::option<best::object<type<n>>&&> object(best::index_t<n> = {}) &&;
  // clang-format on

  /// # `choice::as_ptr()`
  ///
  /// Extracts the `n`th alternative as a pointer. Returns `nullptr` if
  /// `which() != n`.
  template <size_t n>
  constexpr cptr<n> as_ptr(best::index_t<n> i = {}) const {
    return which() == n ? impl().template ptr<n>(
                              unsafe("which() is checked right before this"))
                        : nullptr;
  }
  template <size_t n>
  constexpr ptr<n> as_ptr(best::index_t<n> i = {}) {
    return which() == n ? impl().template ptr<n>(
                              unsafe("which() is checked right before this"))
                        : nullptr;
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
      impl().template emplace<n>(BEST_FWD(args)...);
      return *at(t);
    }
    impl().template emplace<n>(BEST_FWD(args)...);
    return *at(t);
  }

  /// # `choice::match()`, `choice::index_match()`
  ///
  /// Calls one of `cases` (chosen by overload resolution) on the currently
  /// chosen alternative.
  ///
  /// The chosen callback is, where `n` is the current alternative, the callback
  /// that matches the concept `best::callable<void(ref<n>)>`. However, if
  /// `choice::type<n>` is void, it instead tests for `best::callable<void()>`,
  /// if that fails, `best::callable<void(best::empty)>`.
  ///
  /// `best::index_match()` is identical, except that callbacks are expected to
  /// accept a `best::index_t` as their first argument.
  constexpr decltype(auto) match(auto... cases) const&;
  constexpr decltype(auto) match(auto... cases) &;
  constexpr decltype(auto) match(auto... cases) const&&;
  constexpr decltype(auto) match(auto... cases) &&;
  constexpr decltype(auto) index_match(auto... cases) const&;
  constexpr decltype(auto) index_match(auto... cases) &;
  constexpr decltype(auto) index_match(auto... cases) const&&;
  constexpr decltype(auto) index_match(auto... cases) &&;

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
    return index_match([&](auto tag, auto&&... value) {
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
    return index_match([&](auto tag, auto&&... value) {
      return best::choice<type<p>...>(best::index<(*inverse)[tag.value]>,
                                      BEST_FWD(value)...);
    });
  }

  friend void BestFmt(auto& fmt, const choice& ch)
    requires requires(best::object<Alts>... alts) { (fmt.format(alts), ...); }
  {
    ch.index_match([&](auto idx) { fmt.format("choice<{}>(void)", idx.value); },
                   [&](auto idx, const auto& value) {
                     fmt.format("choice<{}>({:!})", idx.value, value);
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
           index_match(
               [&](auto) { return true; },  // void case.
               [&](auto tag, const auto& value) { return value == that[tag]; });
  }

  template <typename... Us>
  BEST_INLINE_ALWAYS constexpr best::common_ord<best::order_type<Alts, Us>...>
  operator<=>(const choice<Us...>& that) const
    requires(best::comparable<Alts, Us> && ...)
  {
    return (which() <=> that.which())->*best::or_cmp([&] {
      using Output = best::common_ord<best::order_type<Alts, Us>...>;
      return index_match(
          [&](auto) -> Output { return best::Equal; },  // void case.
          [&](auto tag, const auto& value) -> Output {
            return value <=> that[tag];
          });
    });
  }

 private:
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
  constexpr const auto&& impl() const&& { return BEST_MOVE(BEST_CHOICE_IMPL_); }
  constexpr auto&& impl() && { return BEST_MOVE(BEST_CHOICE_IMPL_); }

 public:
  choice_internal::impl<Alts...> BEST_CHOICE_IMPL_;

  // This effectively makes the above field private by making it impossible to
  // name.
#define BEST_CHOICE_IMPL_ private_
};
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename... A>
template <size_t n>
constexpr choice<A...>::cref<n> choice<A...>::operator[](
    best::index_t<n> idx) const& {
  check_ok(idx.value);
  return impl().deref(unsafe{"check_ok() called before this"}, idx);
}
template <typename... A>
template <size_t n>
constexpr choice<A...>::ref<n> choice<A...>::operator[](
    best::index_t<n> idx) & {
  check_ok(idx.value);
  return impl().deref(unsafe{"check_ok() called before this"}, idx);
}
template <typename... A>
template <size_t n>
constexpr choice<A...>::crref<n> choice<A...>::operator[](
    best::index_t<n> idx) const&& {
  check_ok(idx.value);
  return static_cast<crref<n>>(
      impl().deref(unsafe{"check_ok() called before this"}, idx));
}
template <typename... A>
template <size_t n>
constexpr choice<A...>::rref<n> choice<A...>::operator[](
    best::index_t<n> idx) && {
  check_ok(idx.value);
  return static_cast<rref<n>>(
      impl().deref(unsafe{"check_ok() called before this"}, idx));
}

template <typename... A>
template <size_t n>
constexpr best::option<typename choice<A...>::template cref<n>>
choice<A...>::at(best::index_t<n> i) const& {
  if (which() != n) return {};
  return best::call_devoid([&]() -> decltype(auto) {
    return impl().deref(unsafe{"checked which() before this"}, i);
  });
}
template <typename... A>
template <size_t n>
constexpr best::option<typename choice<A...>::template ref<n>> choice<A...>::at(
    best::index_t<n> i) & {
  if (which() != n) return {};
  return best::call_devoid([&]() -> decltype(auto) {
    return impl().deref(unsafe{"checked which() before this"}, i);
  });
}
template <typename... A>
template <size_t n>
constexpr best::option<typename choice<A...>::template crref<n>>
choice<A...>::at(best::index_t<n> i) const&& {
  if (which() != n) return {};
  return best::call_devoid([&]() -> decltype(auto) {
    return best::option(best::bind,
                        static_cast<crref<n>>(impl().deref(
                            unsafe{"checked which() before this"}, i)));
  });
}
template <typename... A>
template <size_t n>
constexpr best::option<typename choice<A...>::template rref<n>>
choice<A...>::at(best::index_t<n> i) && {
  if (which() != n) return {};
  return best::call_devoid([&]() -> decltype(auto) {
    return best::option(best::bind,
                        static_cast<rref<n>>(impl().deref(
                            unsafe{"checked which() before this"}, i)));
  });
}

template <typename... A>
template <size_t n>
constexpr best::option<
    const best::object<typename choice<A...>::template type<n>>&>
choice<A...>::object(best::index_t<n> i) const& {
  if (which() != n) return {};
  return impl().object(unsafe{"checked which() before this"}, i);
}
template <typename... A>
template <size_t n>
constexpr best::option<best::object<typename choice<A...>::template type<n>>&>
choice<A...>::object(best::index_t<n> i) & {
  if (which() != n) return {};
  return impl().object(unsafe{"checked which() before this"}, i);
}
template <typename... A>
template <size_t n>
constexpr best::option<
    const best::object<typename choice<A...>::template type<n>>&&>
choice<A...>::object(best::index_t<n> i) const&& {
  if (which() != n) return {};
  return BEST_MOVE(impl().object(unsafe{"checked which() before this"}, i));
}
template <typename... A>
template <size_t n>
constexpr best::option<best::object<typename choice<A...>::template type<n>>&&>
choice<A...>::object(best::index_t<n> i) && {
  if (which() != n) return {};
  BEST_MOVE(impl().object(unsafe{"checked which() before this"}, i));
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
  return best::assume(which() == n), static_cast<crref<n>>(impl().deref(u, i));
}
template <typename... A>
template <size_t n>
constexpr choice<A...>::rref<n> choice<A...>::at(best::unsafe u,
                                                 best::index_t<n> i) && {
  return best::assume(which() == n), static_cast<rref<n>>(impl().deref(u, i));
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
  return BEST_MOVE(*this).impl().match(
      choice_internal::Overloaded(BEST_FWD(cases)...));
}
template <typename... A>
constexpr decltype(auto) choice<A...>::match(auto... cases) && {
  return BEST_MOVE(*this).impl().match(
      choice_internal::Overloaded(BEST_FWD(cases)...));
}

template <typename... A>
constexpr decltype(auto) choice<A...>::index_match(auto... cases) const& {
  return impl().index_match(choice_internal::Overloaded(BEST_FWD(cases)...));
}
template <typename... A>
constexpr decltype(auto) choice<A...>::index_match(auto... cases) & {
  return impl().index_match(choice_internal::Overloaded(BEST_FWD(cases)...));
}
template <typename... A>
constexpr decltype(auto) choice<A...>::index_match(auto... cases) const&& {
  return BEST_MOVE(*this).impl().index_match(
      choice_internal::Overloaded(BEST_FWD(cases)...));
}
template <typename... A>
constexpr decltype(auto) choice<A...>::index_match(auto... cases) && {
  return BEST_MOVE(*this).impl().index_match(
      choice_internal::Overloaded(BEST_FWD(cases)...));
}
}  // namespace best

#endif  // BEST_CONTAINER_CHOICE_H_