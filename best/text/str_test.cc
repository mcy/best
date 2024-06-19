#include "best/text/str.h"

#include "best/container/vec.h"
#include "best/test/test.h"

namespace best::str_test {

best::test Empty = [](auto& t) {
  best::str s1;
  t.expect_eq(s1, "");
  t.expect_eq(s1.size(), 0);
  t.expect(s1.is_empty());

  best::str s2 = "";
  t.expect_eq(s2, "");
  t.expect_eq(s2.size(), 0);
  t.expect(s2.is_empty());

  best::str s3 = *best::str::from_nul(nullptr);
  t.expect_eq(s3, "");
  t.expect_eq(s3.size(), 0);
  t.expect(s3.is_empty());

  best::str s4 = *best::str::from_nul("");
  t.expect_eq(s4, "");
  t.expect_eq(s4.size(), 0);
  t.expect(s4.is_empty());
};

best::test Size = [](auto& t) {
  best::str s = "foo";
  t.expect_eq(s.size(), 3);
  t.expect(!s.is_empty());

  best::str s2 = "foo\0foo";
  t.expect_eq(s2.size(), 7);
};

best::test Eq = [](auto& t) {
  best::str test = "solomon🧶🐈‍⬛黒猫";
  t.expect_eq(test, test);
  t.expect_eq(test, "solomon🧶🐈‍⬛黒猫");
  t.expect_eq(test, (const char*)"solomon🧶🐈‍⬛黒猫");
  t.expect_eq(test, std::string_view("solomon🧶🐈‍⬛黒猫"));

  t.expect_ne(test, best::str("solomon"));
  t.expect_ne(test, "solomon");
  t.expect_ne(test, (const char*)"solomon");
  t.expect_ne(test, std::string_view("solomon"));
};

best::test Cmp = [](auto& t) {
  best::str x0 = "";
  best::str x1 = "xyx";
  best::str x2 = "xyz";
  best::str x3 = "xyz2";
  best::str16 x4 = u"";
  best::str16 x5 = u"xyx";
  best::str16 x6 = u"xyz";
  best::str16 x7 = u"xyz2";

  // Same-encoding comparisons.
  t.expect_lt(x0, x1);
  t.expect_lt(x0, x2);
  t.expect_lt(x1, x2);
  t.expect_lt(x0, x3);
  t.expect_lt(x1, x3);
  t.expect_lt(x2, x3);

  t.expect_lt(x4, x5);
  t.expect_lt(x4, x6);
  t.expect_lt(x5, x6);
  t.expect_lt(x4, x7);
  t.expect_lt(x5, x7);
  t.expect_lt(x6, x7);

  // Cross-encoding comparisons.
  t.expect_lt(x4, x1);
  t.expect_lt(x4, x2);
  t.expect_lt(x5, x2);
  t.expect_lt(x4, x3);
  t.expect_lt(x5, x3);
  t.expect_lt(x6, x3);

  t.expect_lt(x0, x5);
  t.expect_lt(x0, x6);
  t.expect_lt(x1, x6);
  t.expect_lt(x0, x7);
  t.expect_lt(x1, x7);
  t.expect_lt(x2, x7);
};

best::test Utf8Decode = [](auto& t) {
  best::str test = "solomon🧶🐈‍⬛黒猫";
  t.expect_eq(test.size(), 27);

  best::vec<rune> runes;
  for (rune r : test.runes()) {
    runes.push(r);
  }

  t.expect_eq(runes,
              best::span<const rune>{'s', 'o', 'l', 'o', 'm', 'o', 'n', U'🧶',
                                     U'🐈', 0x200d, U'⬛', U'黒', U'猫'});
};

best::test Utf16Decode = [](auto& t) {
  best::str16 test = u"solomon🧶🐈‍⬛黒猫";
  t.expect_eq(test.size(), 15);
  best::vec<rune> runes;
  for (rune r : test.runes()) {
    runes.push(r);
  }

  t.expect_eq(runes,
              best::span<const rune>{'s', 'o', 'l', 'o', 'm', 'o', 'n', U'🧶',
                                     U'🐈', 0x200d, U'⬛', U'黒', U'猫'});
};

best::test Affix = [](auto& t) {
  best::str haystack = "a complicated string. see solomon: 🐈‍⬛";

  t.expect(haystack.starts_with("a complicated string"));
  t.expect(!haystack.starts_with("complicated string"));
  t.expect(haystack.starts_with(u"a complicated string"));
  t.expect(!haystack.starts_with(u"complicated string"));
  t.expect(haystack.starts_with(str("a complicated string")));
  t.expect(!haystack.starts_with(str("complicated string")));
  t.expect(haystack.starts_with(str16(u"a complicated string")));
  t.expect(!haystack.starts_with(str16(u"complicated string")));

  t.expect(haystack.starts_with('a'));
  t.expect(!haystack.starts_with('z'));
  t.expect(!haystack.starts_with(U'🧶'));
};

best::test Contains = [](auto& t) {
  best::str haystack = "a complicated string. see solomon: 🐈‍⬛";

  t.expect(haystack.contains("solomon"));
  t.expect(!haystack.contains("daisy"));
  t.expect(haystack.contains(u"solomon"));
  t.expect(!haystack.contains(u"daisy"));

  t.expect(haystack.contains(U'🐈'));
  t.expect(!haystack.contains('z'));
  t.expect(!haystack.contains(U'🍣'));
  t.expect(haystack.contains(U"🐈‍⬛"));
};

best::test Find = [](auto& t) {
  best::str haystack = "a complicated string. see solomon: 🐈‍⬛";

  t.expect_eq(haystack.find("solomon"), 26);
  t.expect_eq(haystack.find("daisy"), best::none);
  t.expect_eq(haystack.find(u"solomon"), 26);
  t.expect_eq(haystack.find(u"daisy"), best::none);

  t.expect_eq(haystack.find(U'🐈'), 35);
  t.expect_eq(haystack.find('z'), best::none);
  t.expect_eq(haystack.find(U'🍣'), best::none);
  t.expect_eq(haystack.find(U"🐈‍⬛"), 35);

  t.expect_eq(haystack.find(&rune::is_ascii_punct), 20);
};

best::test Find16 = [](auto& t) {
  best::str16 haystack = u"a complicated string. see solomon: 🐈‍⬛";

  t.expect_eq(haystack.find("solomon"), 26);
  t.expect_eq(haystack.find("daisy"), best::none);
  t.expect_eq(haystack.find(u"solomon"), 26);
  t.expect_eq(haystack.find(u"daisy"), best::none);

  t.expect_eq(haystack.find(U'🐈'), 35);
  t.expect_eq(haystack.find('z'), best::none);
  t.expect_eq(haystack.find(U'🍣'), best::none);
  t.expect_eq(haystack.find(U"🐈‍⬛"), 35);

  t.expect_eq(haystack.find(&rune::is_ascii_punct), 20);
};

best::test SplitAt = [](auto& t) {
  best::str test = "黒猫";

  t.expect_eq(test.split_at(0), best::row{"", "黒猫"});
  t.expect_eq(test.split_at(1), best::none);
  t.expect_eq(test.split_at(2), best::none);
  t.expect_eq(test.split_at(3), best::row{"黒", "猫"});
  t.expect_eq(test.split_at(4), best::none);
  t.expect_eq(test.split_at(5), best::none);
  t.expect_eq(test.split_at(6), best::row{"黒猫", ""});

  test = "🐈‍⬛";

  t.expect_eq(test.split_at(0), best::row{"", "🐈‍⬛"});
  t.expect_eq(test.split_at(1), best::none);
  t.expect_eq(test.split_at(2), best::none);
  t.expect_eq(test.split_at(3), best::none);
  t.expect_eq(test.split_at(4), best::row{"🐈", "\u200d⬛"});
  t.expect_eq(test.split_at(5), best::none);
  t.expect_eq(test.split_at(6), best::none);
  t.expect_eq(test.split_at(7), best::row{"🐈\u200d", "⬛"});
  t.expect_eq(test.split_at(8), best::none);
  t.expect_eq(test.split_at(9), best::none);
  t.expect_eq(test.split_at(10), best::row{"🐈‍⬛", ""});
};

best::test SplitAt16 = [](auto& t) {
  best::str16 test = u"黒猫";

  t.expect_eq(test.split_at(0), best::row{u"", u"黒猫"});
  t.expect_eq(test.split_at(1), best::row{u"黒", u"猫"});
  t.expect_eq(test.split_at(2), best::row{u"黒猫", u""});

  test = u"🐈‍⬛";

  t.expect_eq(test.split_at(0), best::row{u"", u"🐈‍⬛"});
  t.expect_eq(test.split_at(1), best::none);
  t.expect_eq(test.split_at(2), best::row{u"🐈", u"\u200d⬛"});
  t.expect_eq(test.split_at(3), best::row{u"🐈\u200d", u"⬛"});
  t.expect_eq(test.split_at(4), best::row{u"🐈‍⬛", u""});
};

best::test SplitOn = [](auto& t) {
  best::str haystack = "a complicated string. see solomon: 🐈‍⬛";

  t.expect_eq(haystack.split_on("solomon"),
              best::row{"a complicated string. see ", ": 🐈‍⬛"});
  t.expect_eq(haystack.split_on("daisy"), best::none);
  t.expect_eq(haystack.split_on(u"solomon"),
              best::row{"a complicated string. see ", ": 🐈‍⬛"});
  t.expect_eq(haystack.split_on(u"daisy"), best::none);

  t.expect_eq(haystack.split_on(U'🐈'),
              best::row{"a complicated string. see solomon: ", "\u200d⬛"});
  t.expect_eq(haystack.split_on('z'), best::none);
  t.expect_eq(haystack.split_on(U'🍣'), best::none);
  t.expect_eq(haystack.split_on(U"🐈‍⬛"),
              best::row{"a complicated string. see solomon: ", ""});

  t.expect_eq(haystack.split_on(&rune::is_ascii_punct),
              best::row{"a complicated string", " see solomon: 🐈‍⬛"});
};
}  // namespace best::str_test