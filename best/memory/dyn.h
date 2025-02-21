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

#ifndef BEST_META_TRAITS_REFS_H2_
#define BEST_META_TRAITS_REFS_H2_

#include <cstddef>

#include "best/base/access.h"
#include "best/memory/internal/dyn.h"
#include "best/memory/layout.h"
#include "best/memory/ptr.h"
#include "best/meta/traits/quals.h"

namespace best {
/// # `best::vtable`
///
/// A complete, raw vtable for a `best::dyn`. This includes a custom
/// per-interface function pointer table, plus
template <typename Interface>
requires requires { typename Interface::BestFuncs; }
class vtable final {
 public:
  /// # `best::vtable::funcs`
  ///
  /// Function table information specific to `Interface`.
  using funcs = Interface::BestFuncs;

  /// # `best::vtable::vtable()`
  ///
  /// Constructs a new vtable witnessing that the type `T` implements
  /// `Interface`, with the given `Interface`-specific data.
  ///
  /// This constructor has no way of checking that `funcs` is actually
  /// appropriate for this vtable.
  template <typename T>
  constexpr vtable(best::tlist<T>, funcs funcs);

  /// # `best::vtable::layout()`
  ///
  /// Returns the layout for the type this vtable represents.
  constexpr best::layout layout() const { return layout_; }

  /// # `best::vtable::can_copy()`, `best::vtable::copy()`
  ///
  /// Runs the copy constructor for this vtable's type. Some types are not
  /// copyable; this can be queried with `can_copy()`. Calling `copy()` on a
  /// non-copyable type is undefined behavior.
  constexpr bool can_copy() const { return copy_ != nullptr; }
  constexpr void copy(void* dst, void* src) const { copy_(dst, src); }

  /// # `best::vtable::destroy`
  ///
  /// Runs the destructor for this vtable's type. `ptr` must be a pointer to
  /// an initialized value of the type this vtable was constructed with.
  constexpr void destroy(void* ptr) const { dtor_(ptr); }

  /// # `best::vtable::operator->`
  ///
  /// Provides access to this vtable's `funcs` value.
  constexpr const funcs* operator->() const { return best::addr(funcs_); }

 private:
  best::layout layout_;
  void (*dtor_)(void*);
  void (*copy_)(void* dst, void* src) = nullptr;
  funcs funcs_;
};

/// # `best::interface<I>`
///
/// Whether `I` is an interface type usable with `best::dyn<I>`. Such a type
/// is a class with the following interface:
///
/// ```
/// class MyIface final {
///  public:
///   friend best::access;
///
///   struct BestFuncs {
///     // ...
///   };
///
///   const best::vtable<MyIface>& vtable() { return *vt_; }
///
///  private:
///   constexpr my_iface(void* data, const best::vtable<MyIface>* vt)
///     : data_(data), vt_(vt) {}
///
///   void* data_;
///   const best::vtable<MyIface>* vt_;
/// };
/// ```
///
/// In other words, it must have a member type named `BestFuncs`, and it must
/// be privately constructible from a `best::vtable`, which will contain a
/// `BestFuncs`.
///
/// The class is expected to define member functions that call function pointers
/// stored in in `vtable().funcs`. The `BEST_INTERFACE()` macro can be used to
/// ease implementing such boilerplate.
template <typename I>
concept interface = requires(void* vp, const I& iface) {
  { iface.vtable() } -> best::same<const best::vtable<best::un_const<I>>&>;
  requires best::dyn_internal::access::can_wrap<best::un_const<I>>();
};

/// # `BEST_INTERFACE()`
///
/// Helper macro for defining a `best::interface` type.
///
/// This macro should be invoked in the body of a class type named `Interface_`.
/// The remaining arguments to this macro specify the virtual functions to
/// generate, in the format (Return, name, (args), options), where options is
/// a comma-delimited list of options for the function. The only supported
/// option is currently `const`, which const-qualifies the function.
///
/// For example:
///
/// ```
/// class IntHolder final {
///  public:
///   BEST_INTERFACE(IntHolder,
///                  (int, get, (), const),
///                  (void, set, (int x)));
/// };
/// ```
///
/// This will generate functions named `get` and `set`, the relevant
/// `best::vtable` boilerplate, and a `BestImplements` implementation that
/// causes any type with a matching member function interface to implement the
/// interface.
#define BEST_INTERFACE(Interface_, ...) BEST_INTERFACE_(Interface_, __VA_ARGS__)

/// # `best::implements<T, I>`
///
/// Checks whether a type implements the given interface type. To implement it,
/// the FTADLE `BestImplements(T&, I*)` must be findable by ADL. This function
/// must return an appropriate vtable type.
///
/// Both arguments actually passed to `BestImplements` will be null.
template <typename T, typename I>
concept implements = requires(T* ptr, I* iptr) {
  requires best::interface<I>;
  { BestImplements(ptr, iptr) } -> best::same<const best::vtable<I>&>;
};

/// # `best::dyn<T>`
///
/// A generic polymorphic type wrapper, for use with e.g. `best::box`.
/// `best::dyn` generalizes C++ virtual functions in a way that allows users to
/// define new interfaces for types the do not own.
template <best::interface I>
class dyn {
 public:
  // Non-constructible; only usable with e.g. best::box and best::ptr.
  constexpr dyn() = delete;

 private:
  class meta;
  friend best::access;
  using BestPtrMetadata = meta;
};

/// # `best::dynbox<I>`
///
/// A pointer to a `best::dyn`.
template <best::interface I>
using dynptr = best::ptr<best::dyn<I>>;
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename I>
requires requires { typename I::BestFuncs; }
template <typename T>
constexpr vtable<I>::vtable(best::tlist<T>, funcs funcs)
  : layout_(best::layout::of<T>()),
    dtor_(+[](void* vp) { best::ptr<T>((T*)vp).destroy(); }),
    funcs_(funcs) {
  if constexpr (best::copyable<T>) {
    copy_ = +[](void* dst_, void* src_) {
      best::ptr<T> dst((T*)dst_), src((T*)src_);
      dst.copy(src);
    };
  }
}

template <best::interface I>
class dyn<I>::meta {
  using Interface = best::un_const<I>;
 public:
  using type = I;
  using pointee = best::copy_quals<void, I>;
  using metadata = const best::vtable<Interface>*;
  using as_const = dyn<const Interface>;

  constexpr meta() = default;
  constexpr meta(metadata vt) : vt_(vt) {}
  constexpr const metadata& to_metadata() const { return vt_; }

  constexpr meta(best::tlist<best::ptr<dyn>>, const metadata& vt) : vt_(vt) {}
  constexpr meta(best::tlist<best::ptr<dyn<Interface>>>,
                 const metadata& vt) requires best::is_const<I>
    : vt_(vt) {}

  template <typename P>
  constexpr meta(best::tlist<P>, const typename P::metadata&)
    requires (P::is_thin() && best::implements<typename P::pointee, Interface>)
    : vt_(&BestImplements((typename P::pointee*)nullptr, (Interface*)nullptr)) {}

  constexpr best::layout layout() const { return vt_->layout(); }
  constexpr auto deref(pointee* ptr) const {
    return best::arrow<I>(
      best::dyn_internal::access::wrap<Interface>(ptr, vt_));
  }

  template <best::implements<I> T>
  static constexpr meta meta_for(T&&) {
    return BestImplements((T*)nullptr, (Interface*)nullptr);
  }
  template <typename T>
  static constexpr void construct(pointee* dst, bool assign, T&& arg) {
    new (dst) T(BEST_FWD(arg));
  }

  static constexpr bool is_statically_copyable() { return false; }
  constexpr bool is_dynamically_copyable() const { return vt_->can_copy(); }

  constexpr void copy(pointee* dst, pointee* src, bool assign) const {
    if (assign) { destroy(dst); }
    vt_->copy(dst, src);
  }

  constexpr void destroy(pointee* ptr) const { vt_->destroy(ptr); }

 private:
  metadata vt_ = nullptr;
};
}  // namespace best

#endif