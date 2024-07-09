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

#ifndef BEST_ITER_ITER_H_
#define BEST_ITER_ITER_H_

//! Iterators.
//!
//! C++ iterators are bad. Ranges are not much better, ergonomically speaking.
//! Rust iterators, however, are pretty great. Hence, we've done our best to

#include "best/container/option.h"
#include "best/func/call.h"
#include "best/meta/guard.h"
#include "best/meta/init.h"
#include "best/meta/taxonomy.h"

namespace best {
/// # `best::is_iter_impl`
///
/// An iterator implementation type. See `best::iter`.
template <typename T>
concept is_iter_impl = requires(T& t) {
  { best::iter<T&>(t).next() } -> best::is_option;
};

/// # `best::is_iter`
///
/// An iterator type, i.e. `best::iter<I>` for some type `I`. See `best::iter`.
template <typename T>
concept is_iter = requires {
  typename best::as_auto<T>::item;
  requires best::is_iter_impl<typename best::as_auto<T>::impl>;
  requires best::same<best::as_auto<T>,
                      best::iter<typename best::as_auto<T>::impl>>;
};

/// # `best::iter_type`
///
/// Extracts the type that an iterator yields.
template <is_iter T>
using iter_type = best::as_auto<T>::item;

/// # `best::has_extra_iter_methods`
///
/// An iterator implementation that allows callers to reach into it through a
/// best::iter. To opt in, define a type alias called `BestIterArrow`.
template <typename T>
concept has_extra_iter_methods = best::is_iter_impl<T> && requires {
  typename best::as_auto<T>::BestIterArrow;
};

/// # `best::size_hint`
///
/// Returned by an iterator implementation's `best::size_hint`, consisting of
/// a lower bound and an upper bound.
struct size_hint final {
  size_t lower = 0;
  best::option<size_t> upper = best::none;
};

/// # `best::iter`
///
/// A `best` iterator.
///
/// In Rust, you define an iterator by implementing the `Iterator` trait.
/// Unfortunately, C++ does not have traits. So instead, what you do is you
/// define an "iterator implementation" type, which only needs to implement one
/// or two functions, and then you wrap it in a `best::iter` to give it
/// superpowers.
///
/// If any of the functions on this type specifies that it is `/*default*/`,
/// it indicates that that function has "default" implementation that can be
/// "overriden" by the iterator implementation, if it provides a function with
/// the same name.
template <typename Impl>
class iter final {
 public:
  using impl = Impl;
  using item = best::option_type<decltype(best::lie<impl>.next())>;

  /// # `iter::iter()`
  ///
  /// Wraps an iterator implementation.
  constexpr explicit iter(impl impl) : impl_(BEST_FWD(impl)) {}

  /// # `iter::operator->`
  ///
  /// Provides access to the implementation of this iterator. This allows
  /// iterators to provide extra operations accessible through `->`.
  ///
  /// The iterator implementation must opt into this by defining a type alias
  /// called `BestIterArrow`.
  // clang-format off
  constexpr best::as_ptr<const impl> operator->() const requires has_extra_iter_methods<Impl> { return best::addr(impl_); }
  constexpr best::as_ptr<impl> operator->() requires has_extra_iter_methods<Impl> { return best::addr(impl_); }
  // clang-format on

  /// # `iter::next()`
  ///
  /// Drives the iterator.
  ///
  /// This causes the iterator to advance forward and yield its next value. When
  /// it retuns `best::none`, that means we're done, although there is no
  /// requirement that it continue to yield `best::none` forever
  constexpr best::option<item> next() { return impl_.next(); }

#define BEST_ITER_FWD(func_)                            \
  if constexpr (requires { BEST_MOVE(impl_).func_(); }) \
    return BEST_MOVE(impl_).func_();

  /// # `iter::size_hint()`
  ///
  /// Hints at how many elements this iterator will yield.
  constexpr /*default*/ best::size_hint size_hint() const {
    if constexpr (requires { impl_.size_hint(); }) {
      // This is so that types do not need to depend on the iterator header
      // to be made into iterators.
      auto [a, b] = impl_.size_hint();
      return best::size_hint{size_t(a), best::option<size_t>(b)};
    }
    return {};
  }

  /// # `iter::collect()`
  ///
  /// Collects the results of this iterator by constructing some container,
  /// such as a vector. If no type parameter is specified, this returns a value
  /// that will implicitly convert to any type convertible from this iterator
  /// type.
  constexpr auto collect() &&;
  template <best::constructible<iter> Out>
  constexpr Out collect() && {
    return Out(BEST_MOVE(*this));
  }

  /// # `iter::count()`
  ///
  /// Consumes this iterator and returns the number of times it returned a
  /// value.
  ///
  /// If the counter overflows a `size_t`, it wraps back to zero.
  constexpr /*default*/ size_t count() && {
    BEST_ITER_FWD(count);
    size_t total = 0;
    while (next().has_value()) ++total;
    return total;
  }

  /// # `iter::count()`
  ///
  /// Consumes this iterator and returns the last element yielded, if any.
  constexpr /*default*/ best::option<item> last() && {
    BEST_ITER_FWD(last);

    best::option<item> last;
    while (auto v = next()) {
      last = BEST_MOVE(v);
    }
    return last;
  }

  /// # `iter::map()`
  ///
  /// Returns an iterator that applies `cb` to each element yielded by this one.
  constexpr auto map(best::callable<void(item&&)> auto&& cb) &&;

  /// # `iter::inspect()`
  ///
  /// Like `map`, but discards the return value of `cb`. This is useful for
  /// "inspecting" values of an interator mid-iteration without producing a new
  /// value. The inspection callback need not take a value at all.
  constexpr auto inspect(best::callable<void()> auto&& cb) &&;
  constexpr auto inspect(best::callable<void(item&)> auto&& cb) &&;

  /// # `iter::enumerate()`
  ///
  /// Converts an iterator yielding `T` to an iterator yielding
  /// `best::row<size_t, T>`, where the first index is the index of the yielded
  /// value.
  constexpr auto enumerate() &&;

  /// # `iter::begin()`, `iter::end()`
  ///
  /// Bridges for C++'s range-for syntax. You should never call these directly.
  constexpr auto begin() { return best::iter_range<Impl&>(impl_); }
  constexpr auto end() { return best::iter_range_end{}; }

  /// # `iter::into_range()`
  ///
  /// Like `iter::begin()`, but moves out of the iterator implementation.
  constexpr auto into_range() && {
    return best::iter_range<Impl>(BEST_MOVE(impl_));
  }

 private:
  impl impl_;
};

#undef BEST_ITER_FWD

/// # `best::iter_range_end`
///
/// The sentinel returned by `iter::end()`.
struct iter_range_end;

/// # `best:;iter_range`
///
/// A wrapper around an iterator implementation that makes it usable in a C++
/// range-for loop. You should generally not need to construct this type, unless
/// you want to make your type iterable.
///
/// When `iter::begin()` is called, it captures the iterator by reference. That
/// is probably not what you want when making your own type directly iterable,
/// so you want to be returning `iter::into_range()` instead.
template <typename Impl>
class iter_range final {
 public:
  static_assert(best::is_iter_impl<Impl>);

  /// # `iter_range::impl`
  ///
  /// The underlying implementation type.
  using impl = Impl;

  /// # `iter_range::item`
  ///
  /// The type of values yielded by this iterator.
  using item = best::iter<Impl>::item;

  // This provides *precisely* the necessary operators needed for range-for, and
  // no others! So, why is this the right selection of operators? As of C++20,
  // when we write
  //
  // ```
  // for ($item : $iter) $expr;
  // ```
  //
  // it is desugared into the following when `$iter` is of class type. (Here,
  // 'foo is a fresh name.)
  //
  // ```
  // auto&& range = $iter;
  // auto begin = range.begin();
  // auto end = range.end();
  // for (; begin != end; ++begin) {
  //   $item = *begin;
  //   $expr;
  // }
  // ```
  //
  // This means we need exactly three operators: `begin != end`, `++begin`, and
  // `*begin`. Every loop iteration starts with `begin != end`, so that is where
  // we call `next()`. `*begin` has to unwrap the option, and is only called
  // when we yielded an actual value. `++begin` does not actually need to do
  // anything and can even safely return void.
  constexpr void operator++() {}
  constexpr item operator*() { return *BEST_MOVE(latest_); }
  constexpr bool operator!=(iter_range_end) {
    latest_ = best::iter<Impl&>(impl_).next();
    return latest_.has_value();
  }

 private:
  template <typename>
  friend class iter;

  constexpr explicit iter_range(impl impl) : impl_(BEST_FWD(impl)) {}

  constexpr iter_range(const iter_range&) = delete;
  constexpr iter_range& operator=(const iter_range&) = delete;

  Impl impl_;
  best::option<item> latest_ = best::none;
};

}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
namespace iter_internal {
template <typename Impl>
class collect final {
 public:
  template <best::constructible<best::iter<Impl>> Out>
  constexpr operator Out() && {
    return BEST_MOVE(iter_).template collect<Out>();
  }

 private:
  constexpr explicit collect(best::iter<Impl> iter) : iter_(BEST_MOVE(iter)) {}

  friend best::iter<Impl>;
  best::iter<Impl> iter_;
};
};  // namespace iter_internal

template <typename Impl>
constexpr auto iter<Impl>::collect() && {
  return iter_internal::collect<Impl>{BEST_MOVE(*this)};
}

namespace iter_internal {
template <typename Impl, typename Cb>
class map final {
 public:
  constexpr auto next() { return iter_.next().map(cb_); }

  constexpr best::size_hint size_hint() const { return iter_.size_hint(); }

 private:
  constexpr explicit map(best::iter<Impl> iter, Cb cb)
      : iter_(BEST_MOVE(iter)), cb_(BEST_FWD(cb)) {}

  friend best::iter<Impl>;
  [[no_unique_address]] iter<Impl> iter_;
  [[no_unique_address]] Cb cb_;
};
template <typename I, typename F>
map(iter<I>&&, F&&) -> map<I, F>;
}  // namespace iter_internal

template <typename Impl>
constexpr auto iter<Impl>::map(best::callable<void(item&&)> auto&& cb) && {
  return best::iter(iter_internal::map{std::move(*this), BEST_FWD(cb)});
}

template <typename Impl>
constexpr auto iter<Impl>::inspect(best::callable<void()> auto&& cb) && {
  return best::iter(iter_internal::map{
      std::move(*this),
      [cb = BEST_FWD(cb)](auto&& value) -> decltype(auto) {
        best::call(cb);
        return BEST_FWD(value);
      },
  });
}

template <typename Impl>
constexpr auto iter<Impl>::inspect(best::callable<void(item&)> auto&& cb) && {
  return best::iter(iter_internal::map{
      std::move(*this),
      [cb = BEST_FWD(cb)](auto&& value) -> decltype(auto) {
        best::call(cb, value);
        return item(value);
      },
  });
}

namespace iter_internal {
template <typename Impl>
class enumerate final {
 public:
  constexpr best::option<best::row<size_t, typename iter<Impl>::item>> next() {
    auto next = iter_.next();
    BEST_GUARD(next);
    return {{idx_++, *BEST_MOVE(next)}};
  }

  constexpr best::size_hint size_hint() const { return iter_.size_hint(); }
  constexpr size_t count() && { return BEST_MOVE(iter_).count(); }

 private:
  constexpr explicit enumerate(best::iter<Impl> iter)
      : iter_(BEST_MOVE(iter)) {}

  friend best::iter<Impl>;
  [[no_unique_address]] iter<Impl> iter_;
  size_t idx_ = 0;
};
template <typename I>
enumerate(iter<I>&&) -> enumerate<I>;
}  // namespace iter_internal

template <typename I>
constexpr auto iter<I>::enumerate() && {
  return best::iter(iter_internal::enumerate(std::move(*this)));
}

}  // namespace best

#endif  // BEST_ITER_ITER_H_
