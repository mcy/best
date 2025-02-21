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

#ifndef BEST_BASE_FWD_H_
#define BEST_BASE_FWD_H_

#include <stddef.h>

#include <cstdint>

#include "best/base/tags.h"
#include "best/meta/traits/objects.h"
#include "best/meta/traits/refs.h"
#include "best/meta/traits/types.h"

//! Forward declarations of all types in best that can be forward-declared.
//!
//! This header is used both for breaking dependency cycles within best, and
//! is intended to contain the canonical forward declarations for all types in
//! best. Users SHOULD NOT forward-declared best types.

namespace best {
// best/base/access.h
class access;

// best/cli/cli.h
struct argv_query;
class cli;

// best/container/box.h
template <typename, typename>
class box;
template <typename, typename>
class vbox;

// best/container/choice.h
template <typename...>
class choice;

// best/container/option.h
template <typename>
class option;
struct none_t;

// best/container/result.h
template <typename, typename>
class result;
template <typename...>
struct ok;
template <typename...>
struct err;

// best/container/row.h
template <typename...>
class row;
template <typename...>
struct args;

// best/container/pun.h
template <typename...>
class pun;

// best/func/arrow.h
template <typename>
class arrow;

// best/func/fnref.h
template <typename>
class fnref;

// best/func/tap.h
template <typename, typename>
class tap;
template <typename Cb>
tap(Cb&&) -> tap<tags_internal_do_not_use::ctad_guard, best::as_auto<Cb>>;
template <typename Cb>
tap(best::bind_t, Cb&&) -> tap<tags_internal_do_not_use::ctad_guard, Cb&&>;

// best/func/tap.h
template <typename>
class iter;
template <typename Impl>
iter(Impl) -> iter<Impl>;
template <typename>
class iter_range;
template <typename Impl>
iter_range(Impl) -> iter_range<Impl>;
struct iter_range_end final {};

// best/iter/bounds.h
template <typename>
struct int_range;

// best/io/ioerr.h
class ioerr;

// best/io/path.h
class path;
class pathbuf;

// best/log/location.h
template <typename>
class track_location;

// best/memory/dyn.h
template <typename>
class interface_base;
template <typename...>
class itable;

// best/memory/ptr.h
template <typename>
class ptr;

// best/memory/span.h
// To actually forward-declare this, we can't utter best::option<size_t> here
// because it's not defined yet. However, if it's a dependent type, we can delay
// it to the second phase of two-phase lookup.
template <best::is_object T, best::option<best::dependent<size_t, T>> =
                               best::option<best::dependent<size_t, T>>{}>
class span;

// best/meta/names.h
class type_names;

// best/meta/reflect.h
template <typename, typename>
class mirror;
template <auto&>
class reflected_field;
template <auto&>
class reflected_value;
template <auto&>
class reflected_type;

// best/meta/tlist.h
template <auto>
struct val;
template <typename...>
class tlist;

// best/meta/traits/arrays.h
template <typename, size_t>
class shape;

// best/meta/traits/empty.h
struct empty;

// best/text/encoding.h
struct encoding_about;
enum class encoding_error : uint8_t;

// best/text/rune.h
class rune;

// best/text/format.h
struct format_spec;
class formatter;

// best/text/str.h
template <typename>
class text;
template <typename>
class pretext;

// best/text/utf8.h
struct utf8;
struct wtf8;
// best/text/utf16.h
struct utf16;
// best/text/utf32.h
struct utf32;

// best/test/test.h
class test;

// Some internal types that need forward decls.
namespace ptr_internal {
struct access;
}
namespace dyn_internal {
struct access;
}
}  // namespace best

#endif  // BEST_BASE_FWD_H_
