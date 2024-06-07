#include "best/strings/str.h"

#include "best/test/test.h"

namespace best::str_test {

best::test Empty = [](auto& t) {
  best::str s1;
  t.expect_eq(s1, "");
  t.expect_eq(s1, nullptr);
  t.expect_eq(s1.size(), 0);
  t.expect(s1.is_empty());

  best::str s2 = "";
  t.expect_eq(s2, "");
  t.expect_eq(s2, nullptr);
  t.expect_eq(s2.size(), 0);
  t.expect(s2.is_empty());

  best::str s3 = nullptr;
  t.expect_eq(s3, "");
  t.expect_eq(s3, nullptr);
  t.expect_eq(s3.size(), 0);
  t.expect(s3.is_empty());

  best::str s4 = best::str(static_cast<const char*>(""));
  t.expect_eq(s4, "");
  t.expect_eq(s4, nullptr);
  t.expect_eq(s4.size(), 0);
  t.expect(s4.is_empty());

  best::str s5 = best::str(nullptr);
  t.expect_eq(s5, "");
  t.expect_eq(s5, nullptr);
  t.expect_eq(s5.size(), 0);
  t.expect(s5.is_empty());
};

best::test Size = [](auto& t) {
  best::str s = "foo";
  t.expect_eq(s.size(), 3);
  t.expect(!s.is_empty());

  best::str s2 = "foo\0foo";
  t.expect_eq(s2.size(), 7);
};

best::test Utf8Decode = [](auto& t) {
  best::str test = "solomon🧶🐈‍⬛黒猫";
  std::vector<rune> runes;
  for (rune r : test.runes()) {
    runes.push_back(r);
  }

  t.expect_eq(best::span(runes),
              best::span<const rune>{'s', 'o', 'l', 'o', 'm', 'o', 'n', U'🧶',
                                     U'🐈', 0x200d, U'⬛', U'黒', U'猫'});
};

best::test Utf16Decode = [](auto& t) {
  best::str16 test = u"solomon🧶🐈‍⬛黒猫";
  std::vector<rune> runes;
  for (rune r : test.runes()) {
    runes.push_back(r);
  }

  t.expect_eq(best::span(runes),
              best::span<const rune>{'s', 'o', 'l', 'o', 'm', 'o', 'n', U'🧶',
                                     U'🐈', 0x200d, U'⬛', U'黒', U'猫'});
};
}  // namespace best::str_test