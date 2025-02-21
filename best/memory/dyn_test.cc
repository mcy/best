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

#include "best/memory/dyn.h"

#include "best/memory/ptr.h"
#include "best/test/test.h"

namespace best::dyn_test {
class IntHolder {
 public:
  BEST_INTERFACE(IntHolder,              //
                 (int, get, (), const),  //
                 (void, set, (int x)));
};

constexpr const best::vtable<IntHolder> int_vtable(
  best::types<int>, {
                      .get = [](const int* thiz) { return -*thiz; },
                      .set = [](int* thiz, int x) { *thiz = x; },
                    });

constexpr const best::vtable<IntHolder>& BestImplements(int*, IntHolder*) {
  return int_vtable;
}

struct Struct {
  int value;

  int get() const { return value * 2; }
  int set(int x) { return value = x; }
};

static_assert(best::interface<IntHolder>);
static_assert(best::implements<int, IntHolder>);
static_assert(best::implements<Struct, IntHolder>);
static_assert(best::same<best::ptr<best::dyn<IntHolder>>::metadata,
                         const best::vtable<IntHolder>*>);

template <typename T>
constexpr bool can_set = requires(T& p) {
  { p->set(42) };
};

static_assert(can_set<best::dynptr<IntHolder>>);
static_assert(!can_set<best::dynptr<const IntHolder>>);
static_assert(can_set<best::dynbox<IntHolder>>);
static_assert(!can_set<best::dynbox<const IntHolder>>);

constexpr int ct_test() {
  int x;
  best::ptr<best::dyn<IntHolder>> p = &x;
  p->set(42);
  return p->get();
}

// Doesn't work in constexpr... yet!
// constexpr auto run_ct_test = ct_test();

best::test Ptr = [](best::test& t) {
  int x = 42;
  best::dynptr<IntHolder> p = &x;
  t.expect_eq(p->get(), -42);

  Struct y{42};
  p = &y;
  t.expect_eq(p->get(), 84);
};

best::test Box = [](best::test& t) {
  best::dynbox<IntHolder> p = best::box(42);
  best::dynptr<IntHolder> p_ = p;

  t.expect_eq(p->get(), -42);
  t.expect_eq(p_->get(), -42);

  p = best::box(Struct{42});
  t.expect_eq(p->get(), 84);

  auto p2 = p.try_copy();
  t.expect(p2.has_value());
  t.expect_eq((*p2)->get(), 84);

  (*p2)->set(45);
  t.expect_eq(p->get(), 84);
  t.expect_eq((*p2)->get(), 90);
};

best::test Of = [](best::test& t) {
  int x = 1;
  Struct y = {.value = 2};
  best::dynptr<IntHolder> p = &x;
  best::dynbox<IntHolder> q = best::box(y);

  t.expect_eq(best::dyn<IntHolder>::of(x)->get(), -1);
  t.expect_eq(best::dyn<IntHolder>::of(y)->get(), 4);
  t.expect_eq(best::dyn<IntHolder>::of(&x)->get(), -1);
  t.expect_eq(best::dyn<IntHolder>::of(&y)->get(), 4);
  t.expect_eq(best::dyn<IntHolder>::of(p)->get(), -1);
  t.expect_eq(best::dyn<IntHolder>::of(q)->get(), 4);
};
}  // namespace best::dyn_test
