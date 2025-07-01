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

#include "best/container/table.h"

#include "best/container/internal/table.h"
#include "best/test/fodder.h"
#include "best/test/test.h"

namespace best::table_test {
using ::best::table_internal::mirror;
using ::best_fodder::MoveOnly;

// A maximally swisstable-hostile hash implementation.
struct TerribleHash final {
  using output = uint64_t;
  constexpr void write(uint64_t x) { h += x; }
  constexpr void write(best::span<const char> bytes) {}
  constexpr output finish() const { return h << 7; }

  uint64_t h = 0;
};

best::test Mirror = [](auto& t) {
  t.expect_eq(mirror(0, 4, 4), 4);
  t.expect_eq(mirror(1, 4, 4), 5);
  t.expect_eq(mirror(2, 4, 4), 6);
  t.expect_eq(mirror(3, 4, 4), 7);

  t.expect_eq(mirror(0, 4, 8), 8);
  t.expect_eq(mirror(1, 4, 8), 9);
  t.expect_eq(mirror(2, 4, 8), 10);
  t.expect_eq(mirror(3, 4, 8), 11);

  t.expect_eq(mirror(4, 4, 8), 4);
  t.expect_eq(mirror(5, 4, 8), 5);
  t.expect_eq(mirror(6, 4, 8), 6);
  t.expect_eq(mirror(7, 4, 8), 7);
};

best::test SmallMap = [](auto& t) {
  best::table<int, int> m{
    {1, 2},
    {3, 4},
    {5, 6},
    {5, 7},
  };
  best::eprintln("{}", m.debug());

  t.expect_eq(m[1].pair(), best::row(1, 2));
  t.expect_eq(m[3].pair(), best::row(3, 4));
  t.expect_eq(m[5].pair(), best::row(5, 7));

  auto entries =
    m.iter().map([](auto e) { return e.pair()->copied(); }).to_vec();
  entries->sort([](const auto& e) { return e.first(); });

  t.expect_eq(entries, best::vec<best::row<int, int>>{{1, 2}, {3, 4}, {5, 7}});
};

best::test SmallSet = [](auto& t) {
  best::table<int> m{1, 2, 3, 4, 5, 6};
  best::eprintln("{}", m.debug());

  t.expect(m[1]);
  t.expect(m[2]);
  t.expect(m[3]);
  t.expect(m[4]);
  t.expect(m[5]);
  t.expect(m[6]);
  t.expect(!m[7]);

  auto entries = m.iter().map([](auto e) { return *e; }).to_vec();
  entries->sort();

  t.expect_eq(entries, best::vec{1, 2, 3, 4, 5, 6});
};

best::test HammerInts = [](auto& t) {
  best::table<int, int> m;

  for (int i : best::bounds{.count = 1000}) { m[i].insert(-i); }
  best::eprintln("{}", m.debug(false));
  for (int i : best::bounds{.count = 1000}) {
    t.expect_eq(m[i].pair(), best::row(i, -i));
  }
  for (auto i : best::bounds{.count = 1000 / 2}) {
    t.expect_eq(m[2 * i].remove(), best::row(2 * i, -2 * i));
  }
  best::eprintln("{}", m.debug(false));
  for (auto i : best::bounds{.count = 1000}) {
    switch (i % 2) {
      case 0: t.expect_eq(m[i].pair(), best::none); break;
      case 1: t.expect_eq(m[i].pair(), best::row(i, -i)); break;
    }
  }
  for (auto i : best::bounds{.count = 1000}) {
    int v = m[i].or_insert(1 - i);
    t.expect_eq(v, i % 2 == 0 ? 1 - i : -i);
  }
  best::eprintln("{}", m.debug(false));
  for (auto e : m) {
    auto k = *e.key();
    switch (k % 2) {
      case 0: t.expect_eq(*e, 1 - k); break;
      case 1:
        t.expect_eq(*e, -k);
        t.expect_eq(BEST_MOVE(e).remove(), best::row(k, -k));
        break;
    }
  }
  best::eprintln("{}", m.debug(false));
  for (auto e : m) {
    auto k = *e.key();
    t.expect_eq(k % 2, 0);
    t.expect_eq(*e, 1 - k);
  }
};

best::test Terrible = [](auto& t) {
  best::table<int>::with_hash<TerribleHash> m;

  for (int i : best::bounds{.count = 100}) { m[i * 2].insert(); }
  best::eprintln("{}", m.debug());

  for (int i : best::bounds{.count = 200}) {
    t.expect_eq(m[i].is_occupied(), i % 2 == 0);
  }

  for (int i : best::bounds{.count = 50}) { m[i * 4 + 2].remove(); }
  best::eprintln("{}", m.debug());

  for (int i : best::bounds{.count = 200}) {
    t.expect_eq(m[i].is_occupied(), i % 4 == 0);
  }
};

best::test Drain = [](auto& t) {
  best::table<int>::with_hash<TerribleHash> set(
    best::bounds{.count = 100}.iter());
  best::eprintln("{}", set.debug());
  for (auto e : set) { BEST_MOVE(e).remove(); }
  best::eprintln("{}", set.debug());

  for (int i : best::bounds{.start = 100, .count = 100}) { set[i].insert(); }
  best::eprintln("{}", set.debug());

  auto entries = set.iter().map([](auto e) { return *e; }).to_vec();
  entries->sort();
  t.expect_eq(entries,
              best::vec(best::bounds{.start = 100, .count = 100}.iter()));
};

best::test StringSet = [](auto& t) {
  best::table<best::strbuf> movies = {
    best::str("The Phonecian Scheme"),
    best::str("Beau Is Afraid"),
    best::str("Twelve Angry Men"),
    best::str("Final Destination"),
  };

  t.expect(movies["The Phonecian Scheme"]);
  t.expect(movies["Beau Is Afraid"]);
  t.expect(movies["Twelve Angry Men"]);
  t.expect(movies["Final Destination"]);

  t.expect(!movies["Spaceballs"]);
  movies[best::str("Spaceballs")].insert();
  t.expect(movies["Spaceballs"]);
};

best::test HammerStrings = [](auto& t) {
  best::table<best::strbuf> strs;

  best::strbuf k;
  for (auto _ : best::bounds{.count = 100}) {
    strs[k].insert();
    k.push('#');
  }
  best::eprintln("{}", strs.debug());
  for (auto i : best::bounds{.count = 100 / 2}) {
    strs[k[{.count = 2 * i}]].remove();
  }
  best::eprintln("{}", strs.debug());
  for (auto i : best::bounds{.count = 100}) {
    t.expect_eq(strs[k[{.count = i}]].is_occupied(), i % 2 != 0);
  }
};

best::test HammerIntVec = [](auto& t) {
  best::table<best::vec<int>> vecs;

  best::vec<int> k;
  for (auto i : best::bounds{.count = 100}) {
    vecs[k].insert();
    k.push(i);
  }
  best::eprintln("{}", vecs.debug());
  for (auto i : best::bounds{.count = 100 / 2}) {
    vecs[k[{.count = 2 * i}]].remove();
  }
  best::eprintln("{}", vecs.debug());
  for (auto i : best::bounds{.count = 100}) {
    t.expect_eq(vecs[k[{.count = i}]].is_occupied(), i % 2 != 0);
  }
};

best::test IntOption = [](auto& t) {
  best::table<best::option<int>, int> m = {

  };
};
}  // namespace best::table_test
