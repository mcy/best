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

#include "best/text/strbuf.h"

#include <string_view>

#include "best/test/test.h"
#include "best/text/ascii.h"

namespace best::strbuf_test {

best::test Empty = [](auto& t) {
  best::strbuf s1;
  t.expect_eq(s1, "");
  t.expect_eq(s1.size(), 0);
  t.expect(s1.is_empty());

  best::strbuf s2 = "";
  t.expect_eq(s2, "");
  t.expect_eq(s2.size(), 0);
  t.expect(s2.is_empty());

  best::strbuf s3 = *best::strbuf::from_nul(nullptr);
  t.expect_eq(s3, "");
  t.expect_eq(s3.size(), 0);
  t.expect(s3.is_empty());

  best::strbuf s4 = *best::strbuf::from_nul("");
  t.expect_eq(s4, "");
  t.expect_eq(s4.size(), 0);
  t.expect(s4.is_empty());
};

best::test Size = [](auto& t) {
  best::strbuf s = "foo";
  t.expect_eq(s.size(), 3);
  t.expect(!s.is_empty());

  best::strbuf s2 = "foo\0foo";
  t.expect_eq(s2.size(), 7);
};

best::test Eq = [](auto& t) {
  best::strbuf test = "solomon🧶🐈‍⬛黒猫";
  t.expect_eq(test, test);
  t.expect_eq(test, "solomon🧶🐈‍⬛黒猫");
  t.expect_eq(test, (const char*)"solomon🧶🐈‍⬛黒猫");
  t.expect_eq(test, std::string_view("solomon🧶🐈‍⬛黒猫"));

  t.expect_ne(test, best::strbuf("solomon"));
  t.expect_ne(test, "solomon");
  t.expect_ne(test, (const char*)"solomon");
  t.expect_ne(test, std::string_view("solomon"));
};

best::test Push = [](auto& t) {
  best::strbuf buf;

  buf.push("solomon");
  buf.push(U'🧶');
  buf.push('z');
  buf.push(u'猫');
  t.expect_eq(buf, "solomon🧶z猫");

  buf.clear();
  buf.push(u"... solomon");
  buf.push(u"🧶🐈‍⬛黒猫");
  t.expect_eq(buf, "... solomon🧶🐈‍⬛黒猫");
};

best::test PushLossy = [](auto& t) {
  best::textbuf<best::ascii> buf;

  buf.push_lossy("solomon");
  buf.push_lossy(U'🧶');
  buf.push_lossy('z');
  buf.push_lossy(u'猫');
  t.expect_eq(buf, "solomon?z?");

  buf.clear();
  buf.push_lossy(u"... solomon");
  buf.push_lossy(u"🧶🐈‍⬛黒猫");
  t.expect_eq(buf, "... solomon??????");
};

best::test Affix = [](auto& t) {
  best::strbuf haystack = "a complicated string. see solomon: 🐈‍⬛";

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
  best::strbuf haystack = "a complicated string. see solomon: 🐈‍⬛";

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
  best::strbuf haystack = "a complicated string. see solomon: 🐈‍⬛";

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
  best::strbuf16 haystack = u"a complicated string. see solomon: 🐈‍⬛";

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
  best::strbuf test = "黒猫";

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
  best::strbuf16 test = u"黒猫";

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
  best::strbuf haystack = "a complicated string. see solomon: 🐈‍⬛";

  t.expect_eq(haystack.split_once("solomon"),
              best::row{"a complicated string. see ", ": 🐈‍⬛"});
  t.expect_eq(haystack.split_once("daisy"), best::none);
  t.expect_eq(haystack.split_once(u"solomon"),
              best::row{"a complicated string. see ", ": 🐈‍⬛"});
  t.expect_eq(haystack.split_once(u"daisy"), best::none);

  t.expect_eq(haystack.split_once(U'🐈'),
              best::row{"a complicated string. see solomon: ", "\u200d⬛"});
  t.expect_eq(haystack.split_once('z'), best::none);
  t.expect_eq(haystack.split_once(U'🍣'), best::none);
  t.expect_eq(haystack.split_once(U"🐈‍⬛"),
              best::row{"a complicated string. see solomon: ", ""});

  t.expect_eq(haystack.split_once(&rune::is_ascii_punct),
              best::row{"a complicated string", " see solomon: 🐈‍⬛"});
};
}  // namespace best::strbuf_test
