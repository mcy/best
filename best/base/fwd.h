#ifndef BEST_BASE_FWD_H_
#define BEST_BASE_FWD_H_

#include <stddef.h>

#include <cstdint>

//! Forward declarations of all types in best that can be forward-declared.
//!
//! This header is used both for breaking dependency cycles within best, and
//! is intended to contain the canonical forward declarations for all types in
//! best. Users SHOULD NOT forward-declared best types.

namespace best {
// best/container/bounds.h
struct bounds;

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
struct row_forward;

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

// best/meta/ebo.h
struct empty;

// best/meta/reflect.h
class mirror;
template <auto&>
class reflected_field;
template <auto&>
class reflected_value;
template <auto&>
class reflected_type;

// best/meta/tlist.h
template <typename...>
class tlist;

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

// best/text/utf.h
struct utf8;
struct wtf8;
struct utf16;
struct utf32;

// best/test/test.h
class test;

}  // namespace best

#endif  // BEST_BASE_FWD_H_