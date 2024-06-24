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

#include "best/meta/reflect.h"

#include "best/test/test.h"

namespace best::reflect_test {

static_assert(best::is_struct<std::array<int, 4>>);
static_assert(best::is_reflected_struct<std::array<int, 4>>);
static_assert(!best::is_reflected_struct<std::array<int, 65>>);

struct Tag {};

template <typename F>
struct MyCallback {
  using BestRowKey = Tag;
  F callback;
};

template <typename F>
MyCallback(F) -> MyCallback<F>;

struct MyType final {
  int x, y, z;
  best::str s1, s2;
  int transient = 0;

  constexpr friend auto BestReflect(auto& m, MyType*) {
    return m.infer()
               ->*m.field(best::vals<&MyType::y>, MyCallback([] { return 42; }))
               ->*m.hide(best::vals<&MyType::transient>);
  }

  constexpr bool operator==(const MyType&) const = default;
};
static_assert(best::is_reflected_struct<MyType>);

enum class MyEnum { A, B, C };
constexpr auto BestReflect(auto& m, MyEnum*) {
  return m.infer()->*m.value(best::vals<MyEnum::B>,
                             MyCallback([] { return 57; }));
}

static_assert(best::is_reflected_enum<MyEnum>);

best::test ToString = [](auto& t) {
  t.expect_eq(best::format("{:?}", MyType{1, 2, 3, "foo", "bar"}),
              R"(MyType {x: 1, y: 2, z: 3, s1: "foo", s2: "bar"})");

  t.expect_eq(best::format("{:?}, {:?}", MyEnum::B, MyEnum(42)),
              "MyEnum::B, MyEnum(42)");
};

best::test Fields = [](auto& t) {
  MyType x0{1, 2, 3, "foo", "bar"};
  t.expect_eq(best::fields(x0), best::row(1, 2, 3, "foo", "bar"));
};

best::test FindTag = [](auto& t) {
  int found = -1;
  best::reflect<MyType>.each([&](auto field) {
    auto tags = field.tags(best::types<Tag>);
    if constexpr (!tags.is_empty()) {
      return found = tags.first().callback();
    }
  });
  t.expect_eq(found, 42);

  best::reflect<MyEnum>.each([&](auto field) {
    auto tags = field.tags(best::types<Tag>);
    if constexpr (!tags.is_empty()) {
      return found = tags.first().callback();
    }
  });
  t.expect_eq(found, 57);
};

best::test FindField = [](auto& t) {
  MyType x0{1, 2, 3, "foo", "bar"};
  best::reflect<MyType>.match(
      "x", [&] {},
      [&](auto f) {
        if constexpr (best::integer<typename decltype(f)::type>) {
          x0->*f = 42;
        }
      });
  t.expect_eq(x0, MyType{42, 2, 3, "foo", "bar"});

  auto x1 = best::reflect<MyEnum>.match(
      "C", [&](auto f) { return f.value; }, [&] { return MyEnum(0); });
  t.expect_eq(x1, MyEnum::C);
};
}  // namespace best::reflect_test
