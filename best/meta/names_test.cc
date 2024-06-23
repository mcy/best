
#include "best/meta/names.h"
#include "best/test/test.h"

namespace best::names_test {
struct Something {};

template <typename T>
struct WithParams {};

best::test Types = [](auto& t) {
  t.expect_eq(best::type_names::of<Something>.name(), "Something");
  t.expect_eq(best::type_names::of<Something>.path(),
              "best::names_test::Something");
  t.expect_eq(best::type_names::of<Something>.name_space(),
              "best::names_test");
  t.expect_eq(best::type_names::of<Something>.params(), "");
  t.expect_eq(best::type_names::of<Something>.name_with_params(), "Something");
  t.expect_eq(best::type_names::of<Something>.path_with_params(),
              "best::names_test::Something");

  t.expect_eq(best::type_names::of<WithParams<int>>.name(), "WithParams");
  t.expect_eq(best::type_names::of<WithParams<int>>.path(),
              "best::names_test::WithParams");
  t.expect_eq(best::type_names::of<WithParams<int>>.name_space(),
              "best::names_test");
  t.expect_eq(best::type_names::of<WithParams<int>>.params(), "<int>");
  t.expect_eq(best::type_names::of<WithParams<int>>.name_with_params(),
              "WithParams<int>");
  t.expect_eq(best::type_names::of<WithParams<int>>.path_with_params(),
              "best::names_test::WithParams<int>");
};

struct Struct {
  int foo;
  int bar();
};

best::test Members = [](auto& t) {
  t.expect_eq(best::field_name<&Struct::foo>, "foo");
  t.expect_eq(best::field_name<&Struct::bar>, "bar");
};

enum Foo { A };
enum class Bar { B };
best::test Enums = [](auto& t) {
  t.expect_eq(best::value_name<A>, "A");
  t.expect_eq(best::value_name<Bar::B>, "B");
};
}  // namespace best::names_test