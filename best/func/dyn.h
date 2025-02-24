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

#ifndef BEST_FUNC_DYN_H_
#define BEST_FUNC_DYN_H_

#include <cstddef>

#include "best/base/access.h"
#include "best/func/arrow.h"
#include "best/func/internal/dyn.h"
#include "best/memory/layout.h"
#include "best/memory/ptr.h"
#include "best/meta/tlist.h"
#include "best/meta/traits/ptrs.h"
#include "best/meta/traits/quals.h"
#include "best/meta/traits/refs.h"

namespace best {
/// # `best::interface<I>`
///
/// Whether `I` is an interface type usable with `best::dyn<I>`. Such a type
/// is a class with the following interface:
///
/// ```
/// class MyIface final : public best::interface_base<MyIface> {
///  public:
///   friend best::access;
///
///   struct BestVtable {
///     // ...
///   };
///
///  private:
///   constexpr my_iface(void* data, const BestVtable* vt)
///     : data_(data), vt_(vt) {}
///
///   void* data_;
///   const BestVtable* vt_;
/// };
/// ```
///
/// In other words, it must have a member type named `BestVtable`, and it must
/// be privately constructible from a const pointer to one.
///
/// The class is expected to define member functions that call function pointers
/// stored in in `vtable().funcs`. The `BEST_INTERFACE()` macro can be used to
/// ease implementing such boilerplate.
template <typename Interface>
concept interface = requires {
  typename best::as_auto<Interface>::BestVtable;
  requires best::derives<Interface,
                         best::interface_base<best::as_auto<Interface>>>;
  requires best::dyn_internal::access::can_wrap<Interface>();
};

/// # `best::implements<T, I>`
///
/// Checks whether a type implements the given interface type. To implement it,
/// the FTADLE `BestImplements(T&, I*)` must be findable by ADL. This function
/// must return an appropriate vtable type.
///
/// Both arguments actually passed to `BestImplements` will be null.
template <typename T, typename Interface>
concept implements = requires {
  // Force this constraint to resolve before we name best::vtable to avoid a
  // requirement cycle. This might be a clang bug, but concepts are so
  // impossible to understand it might also be nominal. :)
  typename best::as_auto<Interface>::BestVtable;

  {
    BestImplements((best::as_raw_ptr<T>)nullptr,
                   (best::as_auto<Interface>*)nullptr)
  } -> best::same<typename best::as_auto<Interface>::BestVtable>;
};

/// # `best::vtable`
///
/// The per-interface vtable type for some `best::interface`.
template <typename Interface>
using vtable = Interface::BestVtable;

/// # `best::itable`
///
/// A complete, raw "interface table" for a `best::dyn`. This provides all of
/// the necessary information for handling a type that implements a collection
/// of interfaces.
template <typename... Interfaces>
class itable final {
 private:
  static constexpr bool singular = sizeof...(Interfaces) == 1;
  using vtables = best::row<best::vtable<Interfaces>...>;

 public:
  /// # `best::itable::vtable()`
  ///
  /// Constructs a new vtable witnessing that the type `T` implements
  /// `Interface`, with the given `Interface`-specific data.
  ///
  /// This constructor has no way of checking that `funcs` is actually
  /// appropriate for this vtable.
  template <typename T>
  constexpr itable(best::tlist<T>, best::vtable<Interfaces>... vtables);

  /// # `best::itable::of()`
  ///
  /// Returns the itable for the given type, if it implements all of
  /// `Interfaces`.
  template <typename T>
  static constexpr const itable& of(best::tlist<T> = {})
    requires (best::implements<T, Interfaces> && ...);

  /// # `best::itable::layout()`
  ///
  /// Returns the layout for the type this itable represents.
  constexpr best::layout layout() const { return layout_; }

  /// # `best::itable::can_copy()`, `best::itable::copy()`
  ///
  /// Runs the copy constructor for this itable's type. Some types are not
  /// copyable; this can be queried with `can_copy()`. Calling `copy()` on a
  /// non-copyable type is undefined behavior.
  constexpr bool can_copy() const { return copy_ != nullptr; }
  constexpr void copy(void* dst, void* src) const { copy_(dst, src); }

  /// # `best::itable::destroy`
  ///
  /// Runs the destructor for this itable's type. `ptr` must be a pointer to
  /// an initialized value of the type this itable was constructed with.
  constexpr void destroy(void* ptr) const { dtor_(ptr); }

  /// # `best::itable::operator->`
  ///
  /// If this itable contains exactly vtable, `operator->` will yield it.
  constexpr const auto* operator->() const requires (sizeof...(Interfaces) == 1)
  {
    return best::addr(vtables_[best::index<0>]);
  }

  /// # `best::itable::operator[]`
  ///
  /// Yields the vtable for the given type, if it is among those in this
  /// itable.
  template <typename I>
  constexpr const auto& operator[](best::tlist<I>) const
    requires (!decltype(best::lie<vtables>.select(
      best::types<best::vtable<I>>))::types.is_empty())
  {
    return vtables_.as_ref().select(
      best::types<const best::vtable<I>&>)[best::index<0>];
  }

 private:
  best::layout layout_;
  void (*dtor_)(void*);
  void (*copy_)(void* dst, void* src) = nullptr;
  vtables vtables_;
};

/// # `best::vtable_binder`
///
/// A wrapper over a function pointer with an extra `void*` argument,
/// representing a type-erased this pointer. This is used as the type for
/// function types in a `best::interface`'s vtable.
template <typename Signature>
class vtable_binder final
  : best::traits_internal::tame<Signature>::template apply<
      dyn_internal::binder_impl> {
 private:
  using impl_t = best::traits_internal::tame<Signature>::template apply<
    dyn_internal::binder_impl>;

 public:
  /// # `vtable_binder::fnptr`
  ///
  /// The underlying fnptr type.
  using fnptr = impl_t::fnptr;

  /// # `vtable_binder::vtable_binder()`
  ///
  /// Constructs a new binder. This can be either from `nullptr`, an appropriate
  /// function pointer with a leading `void*` argument, or a capture-less
  /// closure whose first argument is a reference.
  ///
  /// In the last case, the constructor  will automatically erase the first
  /// argument. This allows initializing a `vtable_binder<int(int)>` from
  /// something like `[](MyClass& x, int y) { return x.field += y; }`
  using impl_t::impl_t;

  /// # `vtable_binder()`
  ///
  /// Calls the function. The first argument is the this pointer.
  using impl_t::operator();

  /// # `vtable_binder::operator==`
  ///
  /// `vtable_binder`s may be compared to `nullptr` (and no other pointer).
  using impl_t::operator==;
  using impl_t::operator bool;

  using impl_t::operator fnptr;
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

/// # `best::dyn<Interfaces>`
///
/// A generic polymorphic type wrapper, for use with e.g. `best::box`.
/// `best::dyn` generalizes C++ virtual functions in a way that allows users to
/// define new interfaces for types the do not own.
template <best::interface... Interfaces>
class dyn final {
 public:
  // Non-constructible; only usable with e.g. best::box and best::ptr.
  constexpr dyn() = delete;

 private:
  class meta;
  class accessor;
  friend best::access;
  using BestPtrMetadata = meta;
};

/// # `best::dynptr<Interfaces>`
///
/// A pointer to a `best::dyn`. This is a convenience shorthand.
template <best::interface... Interfaces>
using dynptr = best::ptr<best::dyn<Interfaces...>>;

/// # `best::as_dyn()`
///
/// Extracts an accessor for some interface from a type that implements it,
/// or a pointer to it. It returns a special accessor, rather than a value of
/// the interface itself, in order to ensure const-correctness. The returned
/// value can be used to access interface methods via `operator->`.
///
/// This function is extremely general: it is the primary mechanism for calling
/// interface functions in a generic context, such as situations which wish to
/// be polymorphic over interface storage. For example, if `T` implements `I`,
/// this function will uniformly handle `T&`, `T*`, `best::ptr<T>`,
/// `best::box<T>`, `best::dynptr<I, ...>`, and `best::dynbox<I, ...>`.
///
/// Every interface provides a static `of` function that calls `as_dyn`.
template <best::interface Interface, typename T>
constexpr auto as_dyn(T&& impl)
  requires (best::implements<T &&, Interface> ||
            best::implements<T &&, const Interface> ||
            best::implements<decltype(*impl), Interface> ||
            best::implements<decltype(*impl), const Interface> ||
            requires {
              {
                impl[best::types<Interface>].operator->()
              } -> best::one_of<const Interface*, Interface*>;
            })
{
  if constexpr (best::implements<T&&, Interface>) {
    return best::dynptr<Interface>(best::addr(impl)).operator->();
  }  //
  else if constexpr (best::implements<T&&, const Interface>) {
    return best::dynptr<const Interface>(best::addr(impl)).operator->();
  }

  else if constexpr (best::implements<decltype(*impl), Interface>) {
    return best::dynptr<Interface>(best::addr(*impl)).operator->();
  }  //
  else if constexpr (best::implements<decltype(*impl), const Interface>) {
    return best::dynptr<const Interface>(best::addr(*impl)).operator->();
  }

  else if constexpr (requires {
                       {
                         impl[best::types<Interface>].operator->()
                       } -> best::one_of<const Interface*, Interface*>;
                     }) {
    return impl[best::types<Interface>];
  }
};

/// # `best::dyn_of<...>`
///
/// Returns whether `value` is suitable for extracting accessors for the given
/// interfaces. This is useful as a template parameter constraint for being
/// polymorphic over all types which implement (or contain) a particular
/// interface.
template <typename T, typename... Interfaces>
concept dyn_of = (requires(T&& value) {
  { best::as_dyn<Interfaces>(value) };
} && ...);

/// # `best::interface_base`
///
/// Base class for interfaces, which provides functions common to all
/// interfaces.
template <typename Interface>
class interface_base /* open */ {
 public:
  static constexpr auto of(best::dyn_of<Interface> auto&& impl) {
    return best::as_dyn<Interface>(impl);
  }
};
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

namespace best {
template <typename... I>
template <typename T>
constexpr itable<I...>::itable(best::tlist<T>, best::vtable<I>... vtables)
  : layout_(best::layout::of<T>()),
    dtor_(+[](void* vp) { best::ptr<T>((T*)vp).destroy(); }),
    vtables_(best::dyn_internal::apply_vtable_defaults<I, T>(vtables)...) {
  if constexpr (best::ptr<T>::can_statically_copy()) {
    copy_ = +[](void* dst_, void* src_) {
      best::ptr<T> dst((T*)dst_), src((T*)src_);
      dst.copy(src);
    };
  }
}

namespace dyn_internal {
template <typename T, typename... I>
inline constexpr itable<I...> impl{
  best::types<T>,
  BestImplements((T*)nullptr, (I*)nullptr)...,
};
}  // namespace dyn_internal

template <typename... I>
template <typename T>
constexpr const itable<I...>& itable<I...>::of(best::tlist<T>)
  requires (best::implements<T, I> && ...)
{
  return best::dyn_internal::impl<best::un_ref<T>, best::as_auto<I>...>;
}

template <best::interface... I>
class dyn<I...>::meta {
 public:
  using type = dyn;
  using pointee = best::select<(best::is_const<I> && ...), const void, void>;
  using metadata = const best::itable<best::un_const<I>...>*;
  using as_const = dyn<const I...>;

  constexpr meta() = default;
  constexpr meta(metadata vt) : vt_(vt) {}
  constexpr const metadata& to_metadata() const { return vt_; }

  template <typename... J>
  constexpr meta(best::tlist<best::ptr<dyn<J...>>>, const metadata& vt)
    requires ((best::same<I, J> || best::same<I, const J>) && ...)
    : vt_(vt) {}

  template <typename P>
  constexpr meta(best::tlist<P>, const typename P::metadata&)
    requires (P::is_thin() && (best::implements<typename P::pointee, I> && ...))
    : vt_(&best::itable<I...>::of(best::types<typename P::pointee>)) {}

  constexpr best::layout layout() const { return vt_->layout(); }
  constexpr auto deref(pointee* ptr) const { return accessor(ptr, vt_); }

  template <typename T>
  static constexpr meta meta_for(T&&) requires (best::implements<T, I> && ...)
  {
    return &best::itable<I...>::of(best::types<T>);
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

template <best::interface... I>
class dyn<I...>::accessor {
 private:
  template <typename J>
  static constexpr auto find =
    best::types<best::un_const<I>...>.template find<best::un_const<J>>();
  template <typename J>
  static constexpr bool is_const =
    best::is_const<typename best::tlist<I...>::template type<*find<J>>>;

 public:
  constexpr auto operator->() const requires (sizeof...(I) == 1)
  {
    return (*this)[best::types<I...>];
  }

  template <typename J>
  constexpr auto operator[](best::tlist<J>) const requires (find<J>.has_value())
  {
    return best::arrow<best::select<is_const<J>, const J, J>>(
      best::dyn_internal::access::wrap<J>(
        data_, best::addr((*vt_)[best::types<best::un_const<J>>])));
  }

 private:
  friend dyn::meta;
  constexpr explicit accessor(typename meta::pointee* data,
                              typename meta::metadata vt)
    : data_(data), vt_(vt) {}

  typename meta::pointee* data_;
  typename meta::metadata vt_;
};
}  // namespace best

#endif  // BEST_FUNC_DYN_H_
