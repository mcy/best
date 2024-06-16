#ifndef BEST_BASE_FWD_H_
#define BEST_BASE_FWD_H_

#include <stddef.h>

//! Forward declarations of all types in best that can be forward-declared.
//!
//! This header is used both for breaking dependency cycles within best, and
//! is intended to contain the canonical forward declarations for all types in
//! best. Users SHOULD NOT forward-declared best types.

namespace best {
// best/container/bag.h
template <typename...>
class bag;

// best/container/bounds.h
struct bounds;

// best/container/choice.h
template <typename...>
class choice;

// best/container/option.h
template <typename>
class option;
struct none_t;

// best/container/pun.h
template <typename...>
class pun;

// best::span cannot be forward-declared because that depends on
// best::option<size_t> being defined.
//
// best/container/span.h
// template <typename, best::option<size_t>>
// class span;

// best/log/location.h
template <typename>
class track_location;

// best/meta/tlist.h
template <typename...>
class tlist;

// best/text/rune.h
class rune;

// best::text cannot be forward-declared because that depends on
// best::encoding being defined.
//
// best/text/str.h
// template <best::encoding>
// class text;

// best/text/utf.h
struct utf8;
struct wtf8;
struct utf16;
struct utf32;

// best/test/test.h
class test;

}  // namespace best

#endif  // BEST_BASE_FWD_H_