#include "best/strings/utf.h"

#include "best/strings/rune.h"
#include "best/test/test.h"

namespace best::utf_test {
best::test Utf8Encode = [](auto& t) {
  using S = best::span<const uint8_t>;
  char buf[4];

  t.expect_eq(rune('\0').encode(buf), S{0});
  t.expect_eq(rune('a').encode(buf), S{'a'});
  t.expect_eq(rune(0x7f).encode(buf), S{0x7f});
  t.expect_eq(rune(u'µ').encode(buf), S{0b110'00010, 0b10'110101});
  t.expect_eq(rune(u'猫').encode(buf),
              S{0b1110'0111, 0b10'001100, 0b10'101011});
  t.expect_eq(rune(U'🧶').encode(buf),
              S{0b11110'000, 0b10'011111, 0b10'100111, 0b10'110110});
};

best::test Utf8Decode = [](auto& t) {
  using S = best::span<const char>;

  t.expect_eq(rune::decode(S{0}), '\0');
  t.expect_eq(rune::decode(S{'a'}), 'a');
  t.expect_eq(rune::decode(S{0x7f}), 0x7f);
  t.expect_eq(rune::decode(S{0b110'00010, 0b10'110101}), u'µ');
  t.expect_eq(rune::decode(S{0b1110'0111, 0b10'001100, 0b10'101011}), u'猫');
  t.expect_eq(
      rune::decode(S{0b11110'000, 0b10'011111, 0b10'100111, 0b10'110110},
                   utf8{}),
      U'🧶');

  // Over-long encodings are forbidden.
  t.expect_eq(rune::decode(S{0b1100'0000, 0b1000'0000}), best::none);

  // Encoding unpaired surrogates is forbidden.
  t.expect_eq(rune::decode(S{0b1110'1101, 0b1010'0001, 0b1011'0111}),
              best::none);
  // But wtf8 is ok with that.
  t.expect_eq(rune::decode(S{0b1110'1101, 0b1010'0001, 0b1011'0111}, wtf8{}),
              0xd877);

  // This is the largest value accepted by utf8 and wtf8.
  t.expect_eq(
      rune::decode(S{0b1111'0100, 0b1000'1111, 0b1011'1111, 0b1011'1111},
                   utf8{}),
      0x10ffff);
  t.expect_eq(
      rune::decode(S{0b1111'0100, 0b1000'1111, 0b1011'1111, 0b1011'1111},
                   wtf8{}),
      0x10ffff);

  t.expect_eq(
      rune::decode(S{0b1111'0100, 0b1001'0000, 0b1000'0000, 0b1000'0000},
                   utf8{}),
      best::none);
  t.expect_eq(
      rune::decode(S{0b1111'0100, 0b1001'0000, 0b1000'0000, 0b1000'0000},
                   wtf8{}),
      best::none);
};

best::test Utf16Encode = [](auto& t) {
  using S = best::span<const uint16_t>;
  char16_t buf[2];

  t.expect_eq(rune('\0').encode<utf16>(buf), S{0});
  t.expect_eq(rune('a').encode<utf16>(buf), S{'a'});
  t.expect_eq(rune(0x7f).encode<utf16>(buf), S{0x7f});
  t.expect_eq(rune(u'µ').encode<utf16>(buf), S{u'µ'});
  t.expect_eq(rune(u'猫').encode<utf16>(buf), S{u'猫'});
  t.expect_eq(rune(U'🧶').encode<utf16>(buf),
              S{0b1101100000111110, 0b1101110111110110});
};

best::test Utf16Decode = [](auto& t) {
  using S = best::span<const char16_t>;

  t.expect_eq(rune::decode<utf16>(S{0}), '\0');
  t.expect_eq(rune::decode<utf16>(S{'a'}), 'a');
  t.expect_eq(rune::decode<utf16>(S{0x7f}), 0x7f);
  t.expect_eq(rune::decode<utf16>(S{u'µ'}), u'µ');
  t.expect_eq(rune::decode<utf16>(S{u'猫'}), u'猫');
  t.expect_eq(rune::decode<utf16>(S{0b1101100000111110, 0b1101110111110110}),
              U'🧶');
};

best::test Utf32Encode = [](auto& t) {
  using S = best::span<const uint32_t>;
  char32_t buf[1];

  t.expect_eq(rune('\0').encode<utf32>(buf), S{0});
  t.expect_eq(rune('a').encode<utf32>(buf), S{'a'});
  t.expect_eq(rune(0x7f).encode<utf32>(buf), S{0x7f});
  t.expect_eq(rune(u'µ').encode<utf32>(buf), S{u'µ'});
  t.expect_eq(rune(u'猫').encode<utf32>(buf), S{u'猫'});
  t.expect_eq(rune(U'🧶').encode<utf32>(buf), S{U'🧶'});
};

best::test Utf32Decode = [](auto& t) {
  using S = best::span<const char32_t>;

  t.expect_eq(rune::decode<utf32>(S{0}), '\0');
  t.expect_eq(rune::decode<utf32>(S{'a'}), 'a');
  t.expect_eq(rune::decode<utf32>(S{0x7f}), 0x7f);
  t.expect_eq(rune::decode<utf32>(S{u'µ'}), u'µ');
  t.expect_eq(rune::decode<utf32>(S{u'猫'}), u'猫');
  t.expect_eq(rune::decode<utf32>(S{U'🧶'}), U'🧶');
};
}  // namespace best::utf_test