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

namespace best {
/// # `best::vtable_header`
///
/// The header for a `best::dyn` interface's vtable. It should be the first
/// field of the vtable struct.
struct vtable_header {
  best::layout layout;
  void (*dtor)(void*) = nullptr;
  void (*copy)(void* dst, void* src) = nullptr;

  template <typename T>
  static constexpr auto of() {
    vtable_header hdr = {
      .layout = best::layout::of<T>(),
      .dtor = +[](void* vp) { best::ptr<T>((T*)vp).destroy(); },
    };

    if constexpr (best::copyable<T>) {
      hdr.copy = +[](void* dst_, void* src_) {
        best::ptr<T> dst((T*)dst_), src((T*)src_);
        dst.copy(src);
      };
    }

    return hdr;
  }
};

/// # `best::interface<I>`
///
/// Whether `I` is an interface type usable with `best::dyn<I>`. Such a type
/// is a class with the following interface:
///
/// ```
/// class my_iface final {
///  public:
///   friend best::access;
///
///   struct BestVtable {
///     best::vtable_header header;
///     // ...
///   };
///
///   const BestVtable* vtable() { return vt_; }
///
///  private:
///   constexpr my_iface(void* data, const BestVtable* vt)
///     : data_(data), vt_(vt) {}
///
///   void* data_;
///   const BestVtable* vt_;
/// }
/// ```
///
/// In other words, it must have a member type named `BestVtable`, and it must
/// be privately constructible from a const pointer to it.
///
/// The class is expected to define member functions that call function pointers
/// stored in `my_iface.vt_`. TODO: Complete example.
template <typename I>
concept interface =
  requires(void* vp, const I& iface, const typename I::BestVtable* vt) {
    { iface.vtable() } -> best::same<const typename I::BestVtable*>;
    { vt->header } -> best::same<const best::vtable_header&>;
    requires best::dyn_internal::access::can_wrap<I>();
  };

/// # `best::vtable<I>`
///
/// The raw vtable type associated with some interface type.
template <best::interface I>
using vtable = const typename I::BestVtable*;

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
  { BestImplements(ptr, iptr) } -> best::same<best::vtable<I>>;
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
  constexpr ~dyn() = delete;

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
template <best::interface I>
class dyn<I>::meta {
 public:
  using type = I;
  using pointee = void;
  using metadata = best::vtable<I>;
  using as_const = const I;

  constexpr meta() = default;
  constexpr meta(metadata vt) : vt_(vt) {}
  constexpr const metadata& to_metadata() const { return vt_; }

  constexpr meta(best::tlist<best::ptr<I>>, const metadata& vt) : vt_(vt) {}

  template <typename P>
  constexpr meta(best::tlist<P>, const typename P::metadata&)
    requires (P::is_thin() && best::implements<typename P::pointee, I>)
    : vt_(BestImplements((typename P::pointee*)nullptr, (I*)nullptr)) {}

  constexpr best::layout layout() const { return vt_->header.layout; }
  constexpr auto deref(pointee* ptr) const {
    return best::dyn_internal::access::wrap<I>(ptr, vt_);
  }

  template <best::implements<I> T>
  static constexpr meta meta_for(T&&) {
    return BestImplements((T*)nullptr, (I*)nullptr);
  }
  template <typename T>
  static constexpr void construct(pointee* dst, bool assign, T&& arg) {
    new (dst) T(BEST_FWD(arg));
  }

  static constexpr bool is_statically_copyable() { return false; }
  constexpr bool is_dynamically_copyable() const {
    return vt_->header.copy != nullptr;
  }

  constexpr void copy(pointee* dst, pointee* src, bool assign) const {
    if (assign) { destroy(dst); }
    vt_->header.copy(dst, src);
  }

  constexpr void destroy(pointee* ptr) const { vt_->header.dtor(ptr); }

 private:
  metadata vt_ = nullptr;
};
}  // namespace best

#endif