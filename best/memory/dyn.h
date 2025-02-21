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

#ifndef BEST_MEMORY_DYN_H_
#define BEST_MEMORY_DYN_H_

#include <cstddef>

#include "best/base/access.h"
#include "best/memory/internal/dyn.h"
#include "best/memory/layout.h"
#include "best/memory/ptr.h"
#include "best/meta/init.h"
#include "best/meta/traits/ptrs.h"
#include "best/meta/traits/quals.h"
#include "best/meta/traits/refs.h"

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

  /// # `best::vtable::of()`
  ///
  /// Returns the vtable for the given type, if it implements `Interface`.
  template <typename T>
  requires requires {
    { BestImplements((T*)nullptr, (Interface*)nullptr) };
  }
  static constexpr const vtable& of(best::tlist<T> = {}) {
    return BestImplements((T*)nullptr, (Interface*)nullptr);
  }

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
/// Although not required, it is recommended to add a static `of` function with
/// the following signature:
///
/// ```
/// template <best::as_dyn<MyIface> T>
/// static constexpr auto of(T&& value) {
///   return best::dyn<MyIface>::of(BEST_FWD(value));
/// }
/// ```
///
/// That way, it's easy to access interface methods generically by writing
/// `MyIface::of(value)->foo()`.
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
///
/// ## Default Implementations
///
/// A default implementation for an interface function can be provided by
/// defining a private member functions whose signature matches the function to
/// be defaulted, but with a leading `best::defaulted` argument.
///
/// For example, the following interface is conformed to by all types, because
/// its one method is defaulted:
///
/// ```
/// class Any final {
///  public:
///   BEST_INTERFACE(Any, (uint64_t, type_id, (), const));
///  private:
///   uint64_t type_id(best::defaulted) const {
///     return reinterpret_cast<uintptr_t>(&vtable());
///   }
/// };
/// ```
///
/// Thus, this allows writing `best::dyn<Any>::of(x)->type_id()` to obtain a
/// unique, integer value per-type.
#define BEST_INTERFACE(Interface_, ...) BEST_IFACE_(Interface_, __VA_ARGS__)

/// # `best::defaulted`
///
/// A tag for specifying a default impl in a `BEST_INTERFACE`-generated
/// function. Given an interface function with a signature `F(a, b)`, if the
/// interface type also defines (a possibly private) `F(defaulted, a, b)`,
/// implementations do not need to provide this function to conform.
struct defaulted final {};

/// # `best::implements<T, I>`
///
/// Checks whether a type implements the given interface type. To implement it,
/// the FTADLE `BestImplements(T&, I*)` must be findable by ADL. This function
/// must return an appropriate vtable type.
///
/// Both arguments actually passed to `BestImplements` will be null.
template <typename T, typename Interface>
concept implements = requires {
  { best::vtable<Interface>::of(best::types<T>) };
};

template <best::interface>
class dyn;

/// # `best::as_dyn<Interface>`
///
/// A type that can be accessed as a `best::dyn<Interface>`. This includes all
/// types which implement it, as well as pointer types that convert to
/// `best::dynptr<Interface>`. Use `best::dyn<Interface>::of()` to obtain a
/// `best::dynptr`.
template <typename T, typename Interface>
concept as_dyn = requires(T&& value) {
  { best::dyn<Interface>::of(value) };
};

/// # `best::dynptr<I>`
///
/// A pointer to a `best::dyn`. This is a convenience shorthand.
template <best::interface I>
using dynptr = best::ptr<best::dyn<I>>;

/// # `best::dyn<Interface>`
///
/// A generic polymorphic type wrapper, for use with e.g. `best::box`.
/// `best::dyn` generalizes C++ virtual functions in a way that allows users to
/// define new interfaces for types the do not own.
template <best::interface Interface>
class dyn final {
 public:
  // Non-constructible; only usable with e.g. best::box and best::ptr.
  constexpr dyn() = delete;

  /// # `best::dyn::of()`
  ///
  /// Wraps a value which satisfies `Interface` in an appropriate way, producing
  /// a `best::dynptr<Interface>` for the contained value. A type which can be
  /// passed to `of` satisfies `best::as_dyn<Interface>`.
  ///
  /// This function returns the least-qualified of `best::dynptr<Interface>`
  /// or `best::dynptr<const Interface>` possible.
  template <typename P>
  static constexpr auto of(P&& ptr)
    requires (best::converts_to<P &&, dynptr<Interface>> ||
              best::converts_to<P &&, dynptr<const Interface>> ||
              best::converts_to<best::as_raw_ptr<P>, dynptr<Interface>> ||
              best::converts_to<best::as_raw_ptr<P>, dynptr<const Interface>>)
  {
    if constexpr (best::converts_to<best::as_raw_ptr<P>, dynptr<Interface>>) {
      return dynptr<Interface>(best::addr(ptr));
    } else if constexpr (best::converts_to<best::as_raw_ptr<P>,
                                           dynptr<const Interface>>) {
      return dynptr<const Interface>(best::addr(ptr));
    } else if constexpr (best::converts_to<P&&, dynptr<Interface>>) {
      return dynptr<Interface>(BEST_FWD(ptr));
    } else if constexpr (best::converts_to<P&&, dynptr<const Interface>>) {
      return dynptr<const Interface>(BEST_FWD(ptr));
    }
  }

 private:
  class meta;
  friend best::access;
  using BestPtrMetadata = meta;
};
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

  if constexpr (requires {
                  { I::template BestFuncDefaults<T>(funcs_) };
                }) {
    I::template BestFuncDefaults<T>(funcs_);
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
  constexpr meta(best::tlist<best::ptr<dyn<Interface>>>, const metadata& vt)
    requires best::is_const<I>
    : vt_(vt) {}

  template <typename P>
  constexpr meta(best::tlist<P>, const typename P::metadata&)
    requires (P::is_thin() && best::implements<typename P::pointee, Interface>)
    : vt_(&BestImplements((typename P::pointee*)nullptr, (Interface*)nullptr)) {
  }

  constexpr best::layout layout() const { return vt_->layout(); }
  constexpr auto deref(pointee* ptr) const {
    return best::arrow<I>(
      best::dyn_internal::access::wrap<Interface>(ptr, vt_));
  }

  template <best::implements<I> T>
  static constexpr meta meta_for(T&&) {
    return &best::vtable<I>::of(best::types<T>);
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

#endif  // BEST_MEMORY_DYN_H_
