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

#ifndef BEST_MEMORY_PTR_H_
#define BEST_MEMORY_PTR_H_

#include <cstddef>
#include <type_traits>

#include "best/base/hint.h"
#include "best/base/niche.h"
#include "best/base/ord.h"
#include "best/base/port.h"
#include "best/base/tags.h"
#include "best/func/arrow.h"
#include "best/log/internal/crash.h"
#include "best/memory/internal/ptr.h"
#include "best/memory/layout.h"
#include "best/meta/init.h"
#include "best/meta/tlist.h"
#include "best/meta/traits/arrays.h"
#include "best/meta/traits/empty.h"
#include "best/meta/traits/types.h"

//! Raw pointers.
//!
//! This header provides an enhanced raw pointer type `best::ptr<T>`. `best`
//! pointers more closely resemble Rust pointers than C++ pointers: it is
//! possible for users to define custom metadata to attach to their pointer
//! types. For example, `best::ptr<T[]>` is a fat pointer that carries a length.
//!
//! # Views, Pointers, and Metadata
//!
//! A view type is a type through which a value of type `T` can be accessed. For
//! example, if `T` is an "ordinary" object type, `T&` is its view type. The
//! view type for `T&` is `T&` itself. The view type for `T[n]` is
//! `best::span<T, n>`, and for `T[]` it's `best::span<T>`. View types
//! generalize references in some way.
//!
//! A `best::ptr<T>` contains the necessary information for constructing `T`'s
//! view type, `best::ptr<T>::view`. For this, it carries a raw C++ pointer of
//! type `best::ptr<T>::pointee*`, and a metadata value `best::ptr<T>::meta`,
//! which contains any additional information not carried by the pointer. For
//! example, `best::ptr<T[]>::meta` is `size_t`, and this represents the length
//! of the corresponding span.
//!
//! Types which do not have extra metadata[1] are called thin pointer
//! types, which is detected by `best::is_thin<T>`. Pointers which are not thin
//! are called fat.
//!
//! Dereferencing a pointer produces its view type. This means that functions on
//! the view type can be called through `->`. For example, given a
//! `best::ptr<int[]>`, we can get its element count with `p->size()`.
//!
//! [1]: Formally, a thin pointer type is a type whose pointer metadata type
//! is trivial and empty, and for which `best::is_sized<T>` is true.
//!
//! # Defining Custom Fat Pointers
//!
//! Defining a custom fat pointer type for a user-defined type `T` is somewhat
//! involved.
//!
//! First, you must define a metadata struct `M` that satisfies
//! `best::is_ptr_metadata`. This specifies all kinds of necessary
//! functionality, such as the pointee, view, and metadata types for `T`, how
//! to dereference it, how to compute the layout, etc.
//!
//! THen, you just need to make sure that that type is findable as
//! `T::BestPtrMetadata`. This type can be private: you need only befriend
//! `best::access`. This type is never exposed by `best`; instead, users will
//! only be able to see `M::metadata`.
//!
//!

namespace best {
/// # `best::is_ptr_metadata`
///
/// Whether `M` is metadata for some pointer type `best::ptr<T>`. A metadata
/// type must have the following interface. (Note that functions may be either
/// `static` or `const`; they only need to be callable through a `const M&`.)
///
/// ```
/// struct my_meta {
///   /// The underlying pointee type; best::ptr<T> will contain a pointee*.
///   /// This must be either an object type or a void type.
///   using pointee;
///   /// The user-visible metadata type. `my_meta` does not appear in
///   /// `best::ptr<T>`'s public interface, but `my_meta::metadata` does.
///   using metadata;
///   /// The const version of `T`.
///   using as_const;
///
///   /// Conversion to/from the user-visible metadata type.
///   my_meta(metadata);
///   const metadata& to_metadata() const;
///
///   /// Upcasts from a best::ptr<U> by converting its metadata.
///   /// Whether this constructor exists dictates whether a `best::ptr<T>` is
///   /// convertible from `P = best::ptr<U>`, and specifies how to convert the
///   /// metadata.
///   ///
///   /// If the selected constructor is `explicit`, it is "lossy": the
///   /// conversion produces a  pointer that may not provide information about
///   /// the true complete type. For example includes a conversion to any base
///   /// subobject.
///   template <typename P>
///   my_meta(best::tlist<P>, const typename P::metadata&);
///
///   /// Returns the layout of the whole pointed-to object. If this function is
///   /// static, `T` is said to be "sized", because the size is a compile-time
///   /// constant.
///   best::layout layout() const;
///
///   /// Dereferences the pointer. This should be either a pointer or a class
///   /// type. If the former, `get()` will return `deref()` and
///   /// `operator*()` will return `*deref()`. Otherwise, `get()` will
///   /// return `best::arrow(deref())` and `operator*()` will be deleted.
///   auto deref(pointee*) const;
///
///   /// Constructs a new value at the pointed-to location with some particular
///   /// set of arguments.
///   ///
///   /// `meta_for` constructs a `my_meta` object for the given arguments. For
///   /// example, for a `best::ptr<T[]>`, you can construct from a span, and
///   /// this calculates the necessary layout for that.
///   ///
///   /// `construct()` performs the construction. If `assign` is true, the
///   /// implementation can assume that `*dst` is initialized.
///   ///
///   /// Calling `construct()` on a destination that is too small is UB.
///   static my_meta meta_for(auto&&...);
///   void construct(pointee* dst, bool assign, auto&&... args) const;
///
///   /// Returns whether the pointed-to type is known to be copyable at
///   /// compile time.
///   static bool is_statically_copyable();
///   /// Returns whether the pointed-to type is known to be copyable at
///   /// runtime. Must return `true` if `is_statically_copyable()` returns
///   /// true.
///   bool is_dynamically_copyable() const;
///
///   /// Copies this value to the given destination, which must have a layout
///   /// at least as large as `layout()`. If `assign` is true, the
///   /// implementation can assume that `*dst` is initialized.
///   ///
///   /// If the destination is too small or `is_dynamically_copyable()` returns
///   /// false, this is UB. This function is intended for making copies of
///   /// types which are not known to be copyable at compile time.
///   void copy(pointee* dst, pointee* src, bool assign) const;
///
///   /// Destroys the pointed-to value.
///   void destroy(pointee*) const;
/// };
/// ```
///
/// User-defined types can customize their pointer metadata by providing a
/// member type named `BestPtrMetadata`.
template <typename M>
concept is_ptr_metadata =
  requires(const M& meta, typename M::pointee* ptr, typename M::metadata data,
           best::ptr<typename M::type> dst) {
    typename M::as_const;
    requires best::is_object<typename M::pointee> ||
               best::is_void<typename M::pointee>;
    { M(data) };
    { meta.to_metadata() } -> best::same<const typename M::metadata&>;

    { meta.layout() } -> best::same<best::layout>;

    { meta.deref(ptr) };

    { M::is_statically_copyable() } -> best::same<bool>;
    { meta.is_dynamically_copyable() } -> best::same<bool>;
    { meta.copy(ptr, ptr, bool{}) } -> best::same<void>;

    { meta.destroy(ptr) } -> best::same<void>;
  };

/// # `best::ptr_constructible`
///
/// Whether `T` can be constructed through a `best::ptr<T>` with the given
/// arguments. If `best::is_thin<T>`, this concept holds if (but not only if)
/// `best::constructible<T, Args...>` holds. For fat types, this can be
/// anything: for example, `best::ptr_constructable<int[], int[]>` is false.
template <typename T, typename... Args>
concept ptr_constructible = requires(best::ptr<T> p, Args... args) {
  requires (!best::is_unsized_array<Args> && ...);
  p.construct(BEST_FWD(args)...);
};

/// # `best::ptr_losslessly_converts_to`
///
/// Whether a pointer conversion is "lossless", i.e., converting
/// from `best::ptr<T>` to `best::ptr<U>` produces a pointer whose core
/// operations (layout, copy, and destroy) are unchanged. This is requires for
/// e.g. being able to destroy and dealloc correctly through the resulting
/// pointer.
template <typename T, typename U>
concept ptr_losslessly_converts_to =
  requires(best::ptr_internal::meta<U>& to,
           const best::ptr_internal::meta<T>::metadata& from) {
    {
      to = {best::types<best::ptr<T>>, from}
    };
  };

/// # `best::ptr<T>`
///
/// A pointer to a possibly non-object `T`. See the header documentation for
/// more information.
template <typename T>
class ptr final {
 public:
  /// # `ptr::type`.
  ///
  /// The wrapped type; `ptr<T>` is nominally a `T*`.
  using type = T;

  static_assert(best::is_ptr_metadata<best::ptr_internal::meta<T>>,
                "BestPtrMetadata must satisfy best::is_ptr_metadata");

  /// # `ptr::pointee`, `ptr::metadata`.
  ///
  /// The "true" pointee type. Internally, an `ptr` stores a `pointee*` and a
  /// `metadata`.
  using pointee = best::ptr_internal::meta<T>::pointee;
  using metadata = best::ptr_internal::meta<T>::metadata;

 private:
  using arrow =
    decltype(best::lie<best::ptr_internal::meta<T>>.deref((pointee*)nullptr));
  static_assert(!best::is_ref<arrow>,
                "BestPtrMetadata::deref() must not return a reference");

 public:
  /// # `ptr::view`
  ///
  /// The type returned by `operator*`. This can be either a reference, if
  /// the metadata type returns a pointer out of `deref`, or whatever view type
  /// `best::deref` returns.
  using view = best::select<best::is_raw_ptr<arrow>,
                            best::as_ref<best::un_raw_ptr<arrow>>, arrow>;

  /// # `best::is_sized()`
  ///
  /// Whether `layout()` depends on the actual pointer value.
  static constexpr bool is_sized() {
    return requires { best::ptr_internal::meta<T>::layout(); };
  }

  /// # `best::is_thin()`
  ///
  /// Returns whether this is a "thin pointer", i.e., a pointer whose metadata
  /// is a trivial, empty type.
  static constexpr bool is_thin() {
    return std::is_trivial_v<metadata> && best::is_empty<metadata> &&
           is_sized();
  }

  /// # `best::is_const()`
  ///
  /// Returns whether this is a const pointer, i.e., a pointer that cannot be
  /// mutated through.
  static constexpr bool is_const() {
    return requires(ptr p) {
      requires best::same<ptr, decltype(p.as_const())>;
    };
  }

 private:
  // Helpers for making requires clauses cleaner.
  static constexpr bool thin = is_thin();

 public:
  /// # `ptr::ptr()`
  ///
  /// Constructs a null pointer. Requires that `ptr::metadata` is
  /// default-constructible.
  constexpr ptr() = default;
  constexpr ptr(std::nullptr_t) requires best::constructible<ptr>
  {}

  /// # `ptr::ptr(ptr)`
  ///
  /// Trivially copyable if `ptr::metadata` is too.
  constexpr ptr(const ptr&) = default;
  constexpr ptr& operator=(const ptr&) = default;
  constexpr ptr(ptr&&) = default;
  constexpr ptr& operator=(ptr&&) = default;

  /// # `ptr::ptr(ptr)`
  ///
  /// Wraps a C++ pointer. Requires `best::ptr<T>` to be a thin pointer.
  constexpr ptr(pointee* ptr) requires thin
    : BEST_PTR_(ptr) {}

  template <typename U>
  constexpr ptr(U* p) requires (best::ptr<U>::is_thin() &&
                                best::constructible<best::ptr<T>, best::ptr<U>>)
    : ptr(best::ptr<U>(p)) {}

  /// # `ptr::ptr(ptr<U>)`
  ///
  /// Upcasts a `ptr` of a different type.
  template <typename U>
  constexpr ptr(ptr<U> ptr)
    requires requires(const best::ptr_internal::meta<U>& m) {
      best::ptr_internal::meta<T>(best::types<best::ptr<U>>, m.to_metadata());
    }
    : BEST_PTR_((pointee*)ptr.raw()),
      BEST_PTR_META_(best::types<best::ptr<U>>, ptr.meta()) {}

  /// # `ptr::ptr(ptr, meta)`
  ///
  /// Constructs a pointer from a raw, pointee-pointer, and a metadata value.
  constexpr ptr(ptr<pointee> ptr, metadata meta)
    : BEST_PTR_(ptr.raw()), BEST_PTR_META_(BEST_MOVE(meta)) {}

  /// # `ptr::dangling()`
  ///
  /// Returns a non-null but invalid pointer, which is unique for the choice of
  /// `T`.
  static ptr dangling() requires best::constructible<metadata>
  {
    return {
      best::ptr<pointee>::from_addr(best::align_of<pointee>),
      metadata{},
    };
  }

  /// # `ptr::to_pointee()`
  ///
  /// Converts this pointer to a thin `best::ptr` pointing to its pointee type.
  constexpr best::ptr<pointee> to_pointee() const { return *this; }

  /// # `ptr::as_const()`
  ///
  /// Converts this to the corresponding const pointer type.
  constexpr auto as_const() const {
    return best::ptr<typename best::ptr_internal::meta<T>::as_const>(*this);
  }

  /// # `ptr::cast()`
  ///
  /// Performs an arbitrary pointer cast to some thin pointer type.
  template <typename U>
  constexpr best::ptr<U> cast(best::tlist<U> = {}) const
    requires best::ptr<U>::thin
  {
    return (best::pointee<U>*)raw();
  }

  /// # `ptr::to_addr()`, `ptr::from_addr()`
  ///
  /// Converts this pointer to/from a raw address.
  uintptr_t to_addr() const { return reinterpret_cast<uintptr_t>(raw()); }
  static ptr from_addr(uintptr_t addr, metadata meta = {}) requires thin
  {
    return {reinterpret_cast<pointee*>(addr), BEST_MOVE(meta)};
  }

  /// # `ptr::raw()`, `ptr::meta()`
  ///
  /// Returns the raw underlying pointer and its metadata.
  constexpr pointee* const& raw() const { return BEST_PTR_; }
  constexpr const metadata& meta() const { return meta_().to_metadata(); }

  /// # `ptr::layout()`
  ///
  /// Returns the layout of the pointed-to value.
  constexpr best::layout layout() const { return meta_().layout(); }

  /// # `ptr::is_niche()`
  ///
  /// Whether this value is a niche representation.
  constexpr bool is_niche() const requires thin;

  /// # `ptr::operator*`, `ptr::operator->`, `ptr::deref()`, `ptr::get()`.
  ///
  /// Dereferences this pointer. `deref()` and `get()` are equivalent to
  /// `operator*` and `operator->`.
  ///
  /// For example, this will dereference the wrapping pointer of a `T&`, so
  /// if `T = U&`, then `raw = U**` and this dereferences that twice.
  constexpr view deref() const;
  constexpr auto get() const;
  constexpr view operator*() const { return deref(); }
  constexpr auto operator->() const {
    if constexpr (best::can_arrow<decltype(get())>) {
      return get();
    } else {
      return best::arrow(get());
    }
  }

  /// # `ptr[idx]`, `ptr(idx)`
  ///
  /// If the view type of this `ptr` is indexable or callable, these functions
  /// will forward to it.
  ///
  /// NOTE: is distinct from the indexing operator of raw pointers. E.g.
  /// `best::ptr<int>` is NOT indexable! You should instead write
  /// `offset(i).deref()`.
  // clang-format off
  constexpr decltype(auto) operator[](size_t i) const requires (!thin) && requires { deref()[i]; } { return deref()[i]; }
  constexpr decltype(auto) operator[](bounds i) const requires (!thin) && requires { deref()[i]; } { return deref()[i]; }
  constexpr decltype(auto) operator[](auto&& i) const requires (!thin) && requires { deref()[BEST_FWD(i)]; } { return deref()[BEST_FWD(i)]; }
  constexpr decltype(auto) operator()(auto&&... args) const requires requires { best::call(deref(), args...); } {
    return best::call(deref(), BEST_FWD(args)...);
  }
  // clang-format on

  /// # `ptr::operator+`, `ptr::operator-`
  ///
  /// Performs unchecked pointer arithmetic using `ptr::offset()`.
  constexpr ptr operator+(ptrdiff_t idx) const requires thin;
  constexpr ptr operator+=(ptrdiff_t idx) requires thin;
  constexpr ptr operator++() requires thin;
  constexpr ptr operator++(int) requires thin;
  constexpr ptr operator-(ptrdiff_t idx) const requires thin;
  constexpr ptr operator-=(ptrdiff_t idx) requires thin;
  constexpr ptr operator--() requires thin;
  constexpr ptr operator--(int) requires thin;

  template <typename U>
  constexpr ptrdiff_t operator-(ptr<U> that) const
    requires thin && best::same<const volatile T, const volatile U>
  {
    return raw() - that.raw();
  }

  /// # `ptr::offset()`
  ///
  /// Offsets this pointer by the given index.
  ///
  /// This operation will automatically multiply `idx` by the value returned by
  /// `layout().size()`.
  constexpr ptr offset(ptrdiff_t idx) const requires thin
  {
    if (std::is_constant_evaluated() && size_of<pointee> == layout().size()) {
      return raw() + idx;
    }
    return to_pointee().scaled_offset(idx, layout().size());
  }

  /// # `ptr::scaled_offset()`
  ///
  /// Performs explicitly scaled pointer arithmetic: this function behaves as
  /// if we cast to a `size`-sized type, offset by `idx`, and cast back to `T`.
  ///
  /// Beware: this may create unaligned pointers, which is Undefined Behavior.
  ptr scaled_offset(ptrdiff_t idx, size_t size = 1) const requires thin
  {
    if (idx == 0) { return *this; }
    auto offset = (const char*)raw() + (idx * size);
    return (pointee*)offset;
  }

  /// # `ptr::byte_offset_from()`
  ///
  /// Computes the difference between the addresses of two pointers.
  template <typename U>
  ptrdiff_t byte_offset_from(best::ptr<U> that) const {
    return to_addr() - that.to_addr();
  }

  /// # `ptr::operator==`, `ptr::operator<=>`
  ///
  /// All thin pointers may be compared by address, regardless of type.
  /// Fat pointers may only be compared with pointers whose metadata types are
  /// comparable.
  ///
  /// All pointers may be compared with `nullptr`, which compares as equal with
  /// any null pointer and less than all other pointers.
  constexpr bool operator==(std::nullptr_t) const;
  template <typename U>
  constexpr bool operator==(ptr<U> that) const
    requires requires { meta() == that.meta(); };
  template <typename U>
  constexpr bool operator==(U* that) const requires thin;

  constexpr best::ord operator<=>(std::nullptr_t) const;
  template <typename U>
  constexpr best::ord operator<=>(ptr<U> that) const
    requires requires { meta() <=> that.meta(); };
  template <typename U>
  constexpr best::ord operator<=>(U* that) const requires thin;

 private:
  template <typename... Args>
  static constexpr bool constructible =
    requires(best::ptr_internal::meta<T> m, pointee* dst, Args... args) {
      m.construct(dst, bool{}, BEST_FWD(args)...);
    };

 public:
  /// # `ptr::construct()`, `ptr::assign()`
  ///
  /// Constructs in place according to the constructors valid per
  /// `best::constructible`.
  ///
  /// Note that this function implicitly casts away an untracked const if called
  /// on an `ptr<T&>` or `ptr<T()>`.
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void construct(Args&&... args) const
    requires constructible<Args&&...>;
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void construct(best::args<Args...> args) const
    requires constructible<Args...>;
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void assign(Args&&... args) const
    requires constructible<Args&&...>;
  template <typename... Args>
  BEST_INLINE_SYNTHETIC constexpr void assign(best::args<Args...> args) const
    requires constructible<Args...>;

  BEST_INLINE_SYNTHETIC constexpr void construct(niche) const
    requires thin && best::has_niche<T>;

  /// # `ptr::meta_for()`
  ///
  /// Computes the metadata that must be attached to a pointer that contains
  /// `args`.
  template <typename... Args>
  BEST_INLINE_SYNTHETIC static constexpr metadata meta_for(Args&&... args)
    requires constructible<Args&&...>
  {
    return best::ptr_internal::meta<T>::meta_for(BEST_FWD(args)...)
      .to_metadata();
  }

  /// # `ptr::destroy()`
  ///
  /// Destroys the pointed to object in place.
  BEST_INLINE_SYNTHETIC constexpr void destroy() const
    requires best::destructible<T>;

  /// # `ptr::fill()`
  ///
  /// Fills `count` `T`s with the given byte (i.e., `memset()`).
  BEST_INLINE_ALWAYS void fill(uint8_t byte, size_t count = 1) const
    requires thin
  {
    best::ptr_internal::memset(raw(), byte, count * layout().size());
  }

  /// # `ptr::copy()`, `ptr::move()`, ptr::relo()`
  ///
  /// Copies `count` `T`s from `*that`. The two regions must not overlap.
  /// Beware: overwriting values that *are* initialized will not call their
  /// destructors.
  ///
  /// `ptr::relo()` will leave the copied-from region uninitialized.
  ///
  /// Note that none of these functions will update the metadata of `this`; the
  /// caller is responsible for doing that separately.
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void copy(ptr<U> that, size_t count = 1) const
    requires thin && best::constructible<T, best::as_ref<const U>>
  {
    copy_impl<how{.kind = how::Copy}>(that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void move(ptr<U> that, size_t count = 1) const
    requires thin && best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Move}>(that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void relo(ptr<U> that, size_t count = 1) const
    requires thin && best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Relo}>(that, count);
  }
  BEST_INLINE_ALWAYS constexpr void copy(ptr that) const
    requires (!thin) && (best::ptr_internal::meta<T>::is_statically_copyable())
  {
    meta_().copy(raw(), that.raw(), false);
  }

  /// # `ptr::copy_assign()`, `ptr::move_assign()`, ptr::relo_assign()`
  ///
  /// Copies `count` `T`s from `*that`. The two regions may not overlap, and
  /// the destination must be initialized.
  ///
  /// `ptr::relo_assign()` will leave the copied-from region uninitialized.
  ///
  /// Note that none of these functions will update the metadata of `this`; the
  /// caller is responsible for doing that separately.
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void copy_assign(ptr<U> that,
                                                size_t count = 1) const
    requires thin && best::constructible<T, best::as_ref<const U>>
  {
    copy_impl<how{.kind = how::Copy, .assign = true}>(that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void move_assign(ptr<U> that,
                                                size_t count = 1) const
    requires thin && best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Move, .assign = true}>(that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void relo_assign(ptr<U> that,
                                                size_t count = 1) const
    requires thin && best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Relo, .assign = true}>(that, count);
  }
  BEST_INLINE_ALWAYS constexpr void copy_assign(ptr that) const
    requires (!thin) && (best::ptr_internal::meta<T>::is_statically_copyable())
  {
    meta_().copy(raw(), that.raw(), true);
  }

  /// # `ptr::try_copy()`, `ptr::try_copy_assign()`
  ///
  /// This is like `copy()`, but it fails if there is no copy constructor
  /// available, returning `nullptr`. There are some pointee types that are only
  /// known to be copyable at runtime due to the contents of their vtables.
  ///
  /// On success, returns a new `best::ptr<T>` with the correct metadata
  /// attached.
  [[nodiscard("ptr::try_copy() returns whether or not it succeeded")]]  //
  BEST_INLINE_ALWAYS constexpr ptr
  try_copy(ptr from) const {
    if (!from.can_copy()) { return nullptr; }
    from.meta_().copy(raw(), from.raw(), false);
    return ptr{raw(), from.meta()};
  }
  [[nodiscard(
    "ptr::try_copy_assign() returns whether or not it succeeded")]]  //
  BEST_INLINE_ALWAYS constexpr ptr
  try_copy_assign(ptr from) const {
    if (!from.can_copy()) { return nullptr; }
    from.meta_().copy(raw(), from.raw(), true);
    return ptr{raw(), from.meta()};
  }

  /// # `ptr::can_copy()`
  ///
  /// Checks whether or not the pointee is dynamically copyable. This may be
  /// true even when `ptr::copy()` fails overload resolution.
  constexpr bool can_copy() const { return meta_().is_dynamically_copyable(); }
  static constexpr bool can_statically_copy() {
    return best::ptr_internal::meta<T>::is_statically_copyable();
  }

  /// # `ptr::copy_overlapping()`, `ptr::move_overlapping()`
  ///
  /// Copies `count` `T`s from `*that`. The two regions may overlap.
  /// Beware: overwriting values that *are* initialized will not call their
  /// destructors.
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void copy_overlapping(ptr<U> that,
                                                     size_t count) const
    requires thin && best::constructible<T, best::as_ref<const U>>
  {
    copy_impl<how{.kind = how::Copy, .overlapping = true}>(that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void move_overlapping(ptr<U> that,
                                                     size_t count) const
    requires thin && best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Move, .overlapping = true}>(that, count);
  }
  BEST_INLINE_ALWAYS constexpr void relo_overlapping(ptr<T> that,
                                                     size_t count) const
    requires thin;

  /// # `ptr::copy_assign_overlapping()`, `ptr::move_assign_overlapping()`
  ///
  /// Copies `count` `T`s from `*that`. The two regions may overlap.
  /// Beware: overwriting values that *are* initialized will not call their
  /// destructors.
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void copy_assign_overlapping(ptr<U> that,
                                                            size_t count) const
    requires thin && best::constructible<T, best::as_ref<const U>>
  {
    copy_impl<how{.kind = how::Copy, .overlapping = true, .assign = true}>(
      that, count);
  }
  template <typename U>
  BEST_INLINE_ALWAYS constexpr void move_assign_overlapping(ptr<U> that,
                                                            size_t count) const
    requires thin && best::constructible<T, best::as_rref<U>>
  {
    copy_impl<how{.kind = how::Move, .overlapping = true, .assign = true}>(
      that, count);
  }

  friend void BestFmt(auto& fmt, ptr value) {
    if constexpr (!thin && requires { fmt.format(value.meta()); }) {
      fmt.format("{:#x}@{}", value.to_addr(), value.meta());
      return;
    }

    fmt.format("{:#x}", value.to_addr());
  }
  constexpr friend void BestFmtQuery(auto& query, ptr*) {
    query.requires_debug = false;
    query.uses_method = [](auto r) { return r == 'p'; };
  }

 private:
  template <typename>
  friend class ptr;
  template <typename...>
  friend class vptr;

  constexpr explicit ptr(best::in_place_t, pointee* p,
                         best::ptr_internal::meta<T>&& m)
    : BEST_PTR_(p), BEST_PTR_META_(BEST_MOVE(m)) {}

  constexpr const best::ptr_internal::meta<T>& meta_() const {
    return BEST_PTR_META_;
  }

  BEST_INLINE_ALWAYS constexpr void check() const {
    if (!best::is_debug() || std::is_constant_evaluated()) { return; }
    if (*this == nullptr) {
      best::crash_internal::crash("dereferenced a null `best::ptr`");
    }
    if (to_pointee() < best::ptr<void>::from_addr(0x1000)) {
      best::crash_internal::crash("dereferenced a dangling `best::ptr`");
    };
  }

  struct how final {
    enum { Copy, Move, Relo } kind;
    bool overlapping = false;
    bool assign = false;

    template <typename U>
    constexpr bool can_memcpy() const {
      return best::same<const best::pointee<T>, const best::pointee<U>> &&
             (best::copyable<T, trivially> ||
              (best::relocatable<T, trivially> && kind == how::Relo));
    }

    bool should_memcpy() const {
      return !std::is_constant_evaluated() ||
             // Constexpr support for memcpy() is only valid for trivially
             // copyable types, *not* trivially relocatable types.
             (BEST_CONSTEXPR_MEMCPY_ && best::copyable<T, trivially>);
    }
  };
  template <how how, typename U>
  BEST_INLINE_SYNTHETIC constexpr void copy_impl(ptr<U> that,
                                                 size_t count) const;

  static constexpr uint8_t Magic = 0xcd;

 public:
  // Public for structural-ness.
  pointee* BEST_PTR_ = nullptr;
  [[no_unique_address]] best::ptr_internal::meta<T> BEST_PTR_META_;
};
template <typename T>
ptr(ptr<T>) -> ptr<T>;
template <typename T>
ptr(T*) -> ptr<T>;
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename T>
constexpr bool ptr<T>::is_niche() const requires thin
{
  if constexpr (best::has_niche<T>) {
    if constexpr (best::is_ref<T>) {
      return *raw() == nullptr;
    } else {
      return **this == niche{};
    }
  }
  return false;
}

template <typename T>
BEST_INLINE_SYNTHETIC constexpr auto ptr<T>::deref() const -> view {
  if constexpr (best::is_raw_ptr<decltype(get())>) {
    if constexpr (best::is_void<best::un_raw_ptr<decltype(get())>>) {
      (void)get();
    } else {
      return *get();
    }
  } else {
    check();
    return meta_().deref(raw());
  }
}
template <typename T>
BEST_INLINE_SYNTHETIC constexpr auto ptr<T>::get() const {
  check();
  return meta_().deref(raw());
}

template <typename T>
constexpr ptr<T> ptr<T>::operator+(ptrdiff_t idx) const requires thin
{
  return offset(idx);
}
template <typename T>
constexpr ptr<T> ptr<T>::operator+=(ptrdiff_t idx) requires thin
{
  *this = offset(idx);
  return *this;
}
template <typename T>
constexpr ptr<T> ptr<T>::operator++() requires thin
{
  return *this += 1;
}
template <typename T>
constexpr ptr<T> ptr<T>::operator++(int) requires thin
{
  auto prev = *this;
  ++*this;
  return prev;
}

template <typename T>
constexpr ptr<T> ptr<T>::operator-(ptrdiff_t idx) const requires thin
{
  return offset(-idx);
}
template <typename T>
constexpr ptr<T> ptr<T>::operator-=(ptrdiff_t idx) requires thin
{
  *this = offset(-idx);
  return *this;
}
template <typename T>
constexpr ptr<T> ptr<T>::operator--() requires thin
{
  return *this -= 1;
}
template <typename T>
constexpr ptr<T> ptr<T>::operator--(int) requires thin
{
  auto prev = *this;
  --*this;
  return prev;
}

template <typename T>
constexpr bool ptr<T>::operator==(std::nullptr_t) const {
  return raw() == nullptr;
}
template <typename T>
template <typename U>
constexpr bool ptr<T>::operator==(ptr<U> that) const
  requires requires { meta() == that.meta(); }
{
  return best::equal(raw(), that.raw()) && meta() == that.meta();
}
template <typename T>
template <typename U>
constexpr bool ptr<T>::operator==(U* that) const requires thin
{
  return best::equal(raw(), that);
}

template <typename T>
constexpr best::ord ptr<T>::operator<=>(std::nullptr_t) const {
  return raw() <=> nullptr;
}
template <typename T>
template <typename U>
constexpr best::ord ptr<T>::operator<=>(ptr<U> that) const
  requires requires { meta() <=> that.meta(); }
{
  return best::compare(raw(), that.raw())->*best::or_cmp([&] {
    return meta() <=> that.meta();
  });
}
template <typename T>
template <typename U>
constexpr best::ord ptr<T>::operator<=>(U* that) const requires thin
{
  return best::compare(raw(), that);
}

template <typename T>
template <typename... Args>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::construct(Args&&... args) const
  requires constructible<Args&&...>
{
  check();
  meta_().construct(raw(), false, BEST_FWD(args)...);
}
template <typename T>
template <typename... Args>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::construct(
  best::args<Args...> args) const requires constructible<Args...>
{
  args.row.apply([&](auto&&... args) { construct(BEST_FWD(args)...); });
}
template <typename T>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::construct(niche) const
  requires thin && best::has_niche<T>
{
  check();
  if constexpr (best::is_object<T>) {
    new (raw()) T(niche{});
  } else if constexpr (best::is_ref<T>) {
    *const_cast<best::un_qual<pointee>*>(raw()) = nullptr;
  }
}

template <typename T>
template <typename... Args>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::assign(Args&&... args) const
  requires constructible<Args&&...>
{
  check();
  meta_().construct(raw(), true, BEST_FWD(args)...);
}
template <typename T>
template <typename... Args>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::assign(
  best::args<Args...> args) const requires constructible<Args...>
{
  args.row.apply([&](auto&&... args) { assign(BEST_FWD(args)...); });
}

template <typename T>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::destroy() const
  requires best::destructible<T>
{
  check();
  meta_().destroy(raw());
  if (!std::is_constant_evaluated() && best::is_debug()) {
    BEST_PUSH_GCC_DIAGNOSTIC()
    BEST_IGNORE_GCC_DIAGNOSTIC("-Wdynamic-class-memaccess")
    // If T is polymorphic, Clang will whine that we're clobbering the vtable.
    // However, we literally just called the destructor, so it's simply wrong.
    // Hence, we disable the warning.
    cast(best::types<char>).fill(Magic, layout().size());
    BEST_POP_GCC_DIAGNOSTIC()
  }
}

template <typename T>
template <ptr<T>::how how, typename U>
BEST_INLINE_SYNTHETIC constexpr void ptr<T>::copy_impl(ptr<U> that,
                                                       size_t count) const {
  if (count == 0) { return; }
  check();
  that.check();
  if constexpr (how.template can_memcpy<U>()) {
    if (how.should_memcpy()) {
      auto len = count * sizeof(pointee);
      if constexpr (how.overlapping) {
        best::ptr_internal::memmove(raw(), that.raw(), len);
      } else {
        best::ptr_internal::memcpy(raw(), that.raw(), len);
      }

      // Even when relocating we shouldn't run destructors here. However, we
      // can still clobber the moved-from bytes.
      if constexpr (how.kind == how::Relo) {
        if (!std::is_constant_evaluated() && best::is_debug()) {
          best::ptr_internal::memset(that.raw(), Magic, len);
        }
      }

      return;
    }
  }

  for (size_t i = 0; i < count; ++i) {
    if constexpr (how.assign) {
      if constexpr (how.kind == how::Copy) {
        offset(i).assign(that.offset(i).deref());
      } else {
        offset(i).assign(BEST_MOVE(that.offset(i).deref()));
      }
    } else if constexpr (how.kind == how::Copy) {
      offset(i).construct(that.offset(i).deref());
    } else {
      offset(i).construct(BEST_MOVE(that.offset(i).deref()));
    }

    if constexpr (how.kind == how::Relo) { that.offset(i).destroy(); }
  }
}

template <typename T>
BEST_INLINE_ALWAYS constexpr void ptr<T>::relo_overlapping(ptr<T> that,
                                                           size_t count) const
  requires thin
{
  auto dst = *this;
  auto src = that;

  if (dst == src) { return; }

  // We need to handle the following cases.
  //
  // Non-overlapping shift. Happens when src + count <= dst or dst + count <=
  // src, need to destroy {.start = src, .count = count }.
  // | xxxx | yyyyyyyyyyyy | xxxxxxxxxxxx | ------------ | xxxx |
  //        src            src + count    dst            dst + count
  //
  // Overlapping forward shift. Happens when src < dst < src + count, need to
  // destroy {.start = src, .end = dest }
  // | xxxx | yyyyyyyyyyyy | yyyyyy | ------------ | xxxx |
  //        src            dst      src + count    dst + count
  //
  // The moved part is subdivided as follows according to how it needs to be
  // moved:
  // | aaaa | bbbbbbbbbbbb | cccc |
  // src    src + overlap  dst    src + count
  //
  // Where overlap = src + count - dst. The c part is move-constructed
  // but not destroyed; then the b part is relocated, and the a part is
  // move-assigned and then destroyed.
  //
  // Overlapping backward shift. Happens when dst < src < dest + count, need to
  // destroy {.start = dst + count, .end = src + count }
  // | xxxx | ------------ | yyyyyy | yyyyyyyyyyyy | xxxx |
  //        dst            src      dst + count    src + count
  //
  // The moved part is divided in the analogous way, but the move/relocate/
  // assign regions are in the opposite order.

  // Non-overlapping case.
  if (src + count <= dst || dst + count <= src) {
    dst.relo(src, count);
    return;
  }

  constexpr how how{.kind = how::Relo};
  if constexpr (how.template can_memcpy<T>()) {
    if (how.should_memcpy()) {
      auto len = count * sizeof(pointee);
      best::ptr_internal::memmove(raw(), that.raw(), len);
      return;
    }
  }

  // Forward case.
  if (src < dst && dst < src + count) {
    size_t overlap0 = src + count - dst;
    size_t overlap1 = dst - src;
    // Need to make the copies in backward order to avoid trampling.
    (dst + overlap1).move(src + overlap1, count - overlap1);
    (dst + overlap0).relo(src + overlap0, overlap1 - overlap0);
    dst.relo_assign(src, overlap0);
    return;
  }

  // Backward case.
  if (dst < src && src < dst + count) {
    size_t overlap0 = dst + count - src;
    size_t overlap1 = src - dst;

    dst.move(src, overlap0);
    (dst + overlap0).relo(src + overlap0, overlap1 - overlap0);
    (dst + overlap1).relo_assign(src + overlap1, count - overlap1);

    return;
  }
}
}  // namespace best

#define BEST_PTR_ _private
#define BEST_PTR_META_ _private

#endif  // BEST_MEMORY_PTR_H_
