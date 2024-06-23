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

//#include "best/meta/reflect.h"

#include "best/meta/internal/reflect.h"
#include "best/test/test.h"

namespace best::reflect_test {
struct Foo final {
  int foo, bar, baz;
};

best::test t = [](auto& t) {
  constexpr auto i = reflect_internal::infer_struct<Foo>().index<&Foo::bar>();
  t.expect_eq(i, 1);
};

/*struct Tag {};

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

  constexpr friend auto BestReflect(auto& m, MyType*) {
    return m("MyType",                                                 //
             m.field("x", &MyType::x),                                 //
             m.field("y", &MyType::y),                                 //
             m.field("z", &MyType::z, MyCallback([] { return 42; })),  //
             m.field("s1", &MyType::s1),                               //
             m.field("s2", &MyType::s2));
  }
};

static_assert(best::is_reflected_struct<MyType>);

enum class MyEnum { A, B, C };
constexpr auto BestReflect(auto& m, MyEnum*) {
  return m("MyEnum",                 //
           m.value("A", MyEnum::A),  //
           m.value("B", MyEnum::B),  //
           m.value("C", MyEnum::C));
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
  int found = best::reflect<MyType>.find(
      &MyType::z,
      [](auto field) {
        auto tags = field.tags(best::types<Tag>);
        if constexpr (!tags.is_empty()) {
          return tags[best::index<0>].callback();
        }
        return 0;
      },
      [] { return 0; });
  t.expect_eq(found, 42);
};*/
}  // namespace best::reflect_test
