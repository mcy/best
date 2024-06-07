#ifndef BEST_STRINGS_ENCODING_H_
#define BEST_STRINGS_ENCODING_H_

#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>

#include "best/container/option.h"
#include "best/container/span.h"
#include "best/meta/init.h"

//! Text encodings.
//!
//! best::str and friends are encoding agnostic: they are always sequences of
//! Unicode characters, but that sequence may be encoded in more than one way.
//! This header provides concepts and types for working with encodings.
//!
//! See utf.h for examples of encodings.

namespace best {
/// A text encoding.
///
/// A text encoding is any type that fulfills the "Lucky 7" encoding API
/// from ztd.text.
/// <https://ztdtext.readthedocs.io/en/latest/design/lucky%207.html>
template <typename E_, typename E = std::remove_cvref_t<E_>>
concept encoding = requires(const E& e) {
  /// Required type aliases.
  typename E::code;
  typename E::state;

  /// Required constants.
  { E::MaxCodesPerRune } -> std::convertible_to<size_t>;

  /// The state type must be constructible from const E&, as well
  /// as copyable.
  requires best::constructible<typename E::state> ||
               best::constructible<typename E::state, const E&>;
  requires best::copyable<typename E::state>;

  /// The encoding type itself must be equality comparable, as must its state.
  requires std::equality_comparable<E>;
  requires std::equality_comparable<typename E::state>;

  requires requires(typename E::state& state,
                    best::span<typename E::code>& output, rune r) {
    { e.write_rune(state, output, r) } -> std::convertible_to<bool>;
  };

  requires requires(typename E::state& state,
                    best::span<const typename E::code>& input) {
    { e.read_rune(state, input) } -> std::convertible_to<best::option<rune>>;
  };
};

/// A self-synchronizing encoding, i.e., an encoding which can continue
/// encoding/decoding despite errors.
///
/// An encoding can advertise it is self-syncing by defining a type alias
/// `self_syncing`.
template <typename E>
concept self_syncing_encoding =
    encoding<E> && requires { typename E::self_syncing; };

/// A stateless encoding, which allows performing decoding operations are
/// arbitrary positions within a stream.
///
/// This is determined by the encoding and its state being empty classes.
template <typename E>
concept stateless_encoding = self_syncing_encoding<E> && std::is_empty_v<E> &&
                             std::is_empty_v<typename E::state>;

/// A stateful wrapper over some best::encoding for encoding/decoding from one
/// stream to another.
template <encoding E>
class encoder {
 private:
  /// Make `rune` a dependent type to make it useable in fwd-declared position
  /// on constexpr functions.
  template <typename = E>
  using rune = std::conditional_t<sizeof(E) == 0, void, best::rune>;

 public:
  /// The code unit for this encoding. This is the element type of an encoded
  /// stream.
  using code = E::code;
  /// Any state necessary to save between indivisible decoding steps.
  using state = E::state;

  /// The maximum number of code units E::write_rune() will write.
  static constexpr size_t MaxCodesPerRune = E::MaxCodesPerRune;

  /// Whether this encoding is self-synchronizing.
  static constexpr bool is_self_syncing() { return self_syncing_encoding<E>; }

  /// Whether this encoding is stateless, i.e., whether its state type
  /// is empty.
  static constexpr bool is_stateless() { return stateless_encoding<E>; }

  /// Constructs the singleton encoder if this encoding is totally
  /// stateless.
  constexpr encoder()
    requires(is_stateless());

  constexpr encoder(const encoder&) = default;
  constexpr encoder& operator=(const encoder&) = default;
  constexpr encoder(encoder&&) = default;
  constexpr encoder& operator=(encoder&&) = default;

  /// Constructs a new encoder for the given encoding.
  constexpr explicit encoder(const E& encoding)
    requires best::constructible<state> &&
                 (!best::constructible<state, const E&>)
      : encoding_(std::addressof(encoding)), state_() {}

  constexpr explicit encoder(const E& encoding)
    requires best::constructible<state, const E&>
      : encoding_(std::addressof(encoding)), state_(encoding) {}

  /// Validates whether a span of code units is correctly encoded.
  static constexpr bool validate(const E& e, best::span<const code> input) {
    encoder enc(e);

    while (!input.is_empty()) {
      if (enc.read_rune(&input).is_empty()) {
        return false;
      }
    }

    return true;
  }

  /// Computes the would-be-encoded size from calling write_rune().
  constexpr best::option<size_t> size(rune<> rune) const {
    code buf[MaxCodesPerRune];

    auto copy = *this;
    if (auto encoded = copy.write_rune(buf, rune)) {
      return encoded->size();
    }
    return best::none;
  }

  /// Performs a single indivisible encoding operation.
  ///
  /// Advances both input and output past the read/written-to portions, and
  /// returns the part of output written to.
  ///
  /// Returns best::none on failure; in this case, output is not changed.
  constexpr best::option<best::span<code>> write_rune(best::span<code>* output,
                                                      rune<> rune) {
    auto out0 = *output;

    if (encoding_->write_rune(state_, *output, rune)) {
      size_t written = out0.size() - output->size();
      return out0[{.count = written}];
    }

    *output = out0;
    return best::none;
  }

  /// Identical write_rune(), but does not advance `output`.
  constexpr best::option<best::span<code>> write_rune(best::span<code> output,
                                                      rune<> rune) {
    return write_rune(&output, rune);
  }

  /// Performs a single indivisible decoding operation.
  ///
  /// Advances both input and output past the read/written-to portions, and
  /// returns the part of output written to.
  ///
  /// Returns best::none on failure; in this case, input is not changed.
  constexpr best::option<rune<>> read_rune(best::span<const code>* input) {
    auto in0 = *input;

    if (auto encoded = encoding_->read_rune(state_, *input)) {
      return encoded;
    }

    *input = in0;
    return best::none;
  }

  /// Identical encode(), but does not advance `output`.
  constexpr best::option<rune<>> read_rune(best::span<const code> input) {
    return read_rune(&input);
  }

  constexpr bool operator==(const encoder& it) const = default;

 private:
  const E* encoding_;
  [[no_unique_address]] state state_;
};

/// A string type: a contiguous range that defines the BestEncoding FTADLE
/// and whose data pointer matches that encoding.
template <typename T>
concept string_type = best::contiguous<T> && requires(const T& value) {
  { BestEncoding(best::types<const T&>, value) } -> best::encoding;
  {
    std::data(value)
  } -> best::same<const typename std::remove_cvref_t<decltype(BestEncoding(
      best::types<const T&>, value))>::code*>;
};

/// Extracts the encoding out of a string type.
constexpr const auto& get_encoding(const string_type auto& string) {
  return BestEncoding(best::types<decltype(string)>, string);
}

template <encoding E>
encoder(const E&) -> encoder<E>;

namespace encoding_internal {
template <encoding E>
inline constexpr E singleton;
}  // namespace encoding_internal

template <encoding E>
constexpr encoder<E>::encoder()
  requires(is_stateless())
    : encoder(encoding_internal::singleton<E>) {}

}  // namespace best

#endif  // BEST_STRINGS_ENCODING_H_