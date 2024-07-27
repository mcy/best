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

#include "best/memory/ptr.h"

#include "best/meta/init.h"
#include "best/test/test.h"

namespace best::ptr_test {

static_assert(best::is_thin<int>);
static_assert(best::is_thin<const int>);
static_assert(best::is_thin<int&>);
static_assert(best::is_thin<const int&>);
static_assert(best::is_thin<void>);
static_assert(best::is_thin<const void>);
static_assert(best::is_thin<int[5]>);
static_assert(best::is_thin<const int[5]>);
static_assert(!best::is_thin<int[]>);
static_assert(!best::is_thin<const int[]>);

template <typename T>
concept is_const = best::ptr<T>::is_const();
static_assert(!is_const<int>);
static_assert(is_const<const int>);
static_assert(is_const<int&>);  // Intentional!
static_assert(is_const<const int&>);
static_assert(!is_const<void>);
static_assert(is_const<const void>);
static_assert(!is_const<int[5]>);
static_assert(is_const<const int[5]>);
static_assert(!is_const<int[]>);
static_assert(is_const<const int[]>);

best::test Null = [](auto& t) {
  best::ptr<int> x0;
  best::ptr<int> x1 = nullptr;

  t.expect_eq(x0, nullptr);
  t.expect_eq(x1, nullptr);

  best::ptr<int[]> x2;
  best::ptr<int[]> x3 = nullptr;

  t.expect_eq(x2, nullptr);
  t.expect_eq(x3, nullptr);
};

best::test FromRaw = [](auto& t) {
  int x = 5;
  best::ptr<int> x0 = &x;
  t.expect_eq(x0, &x);
  t.expect_eq(&*x0, &x);
  t.expect_eq(x0.get(), &x);
  t.expect_eq(*x0, 5);
  t.expect_eq(x0.deref(), 5);
  t.expect_eq(*x0 = 42, 42);
  t.expect_eq(x, 42);

  best::ptr<const int> x1 = x0.as_const();
  t.expect_eq(x1, &x);
  t.expect_eq(&*x1, &x);
  t.expect_eq(x1.get(), &x);
  t.expect_eq(*x1, 42);
  t.expect_eq(x1.deref(), 42);

  t.expect_eq(x0, x1);

  best::ptr<int&> x2 = &x0.raw();
  t.expect_eq(x2, &x0.raw());
  t.expect_eq(&*x0, &x);
  t.expect_eq(x0.get(), &x);
  t.expect_eq(*x0, 42);
  t.expect_eq(x0.deref(), 42);
  t.expect_eq(++*x0, 43);
  t.expect_eq(x, 43);

  best::ptr<void> x3 = &x2;
  t.expect_eq(x3, &x2);
  t.expect_eq(x3.get(), &x2);
};

template <typename From, typename To>
concept conv = best::convertible<best::ptr<To>, best::ptr<From>>;

static_assert(conv<int, int>);
static_assert(conv<int, const int>);
static_assert(conv<const int, const int>);
static_assert(conv<int, volatile int>);
static_assert(!conv<const int, int>);
static_assert(!conv<int, unsigned>);
static_assert(conv<int, void>);
static_assert(conv<int, const void>);
static_assert(!conv<const int, void>);
static_assert(conv<int&, const int&>);
static_assert(!conv<const int&, int&>);
static_assert(conv<int&, int* const>);
static_assert(conv<int*, int&>);
static_assert(conv<int&, int&&>);
static_assert(conv<int&&, int&>);
static_assert(conv<int(), int (*const)()>);
static_assert(conv<int (*const)(), int()>);

struct Base {};
struct Derived : Base {};

static_assert(conv<Derived, Base>);
static_assert(!conv<Base, Derived>);

static_assert(conv<int[4], const int[4]>);
static_assert(!conv<const int[4], int[4]>);
static_assert(conv<int[4], int[]>);
static_assert(conv<const int[4], const int[]>);
static_assert(!conv<const int[4], int[]>);
static_assert(conv<int, int[]>);
static_assert(conv<const int, const int[]>);
static_assert(!conv<const int, int[]>);

best::test Span = [](auto& t) {
  int xs[] = {1, 2, 3, 4, 5};
  best::ptr<int[5]> x0 = &xs;
  // t.expect_eq(x0->size(), 5);

  best::ptr<int[]> x1 = x0;
  t.expect_eq(x1->size(), 5);

  best::ptr<int[]> x2 = best::ptr(&(*x0)[1]);
  t.expect_eq(x2->size(), 1);
};

}  // namespace best::ptr_test
