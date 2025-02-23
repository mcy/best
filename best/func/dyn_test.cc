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

#include "best/func/dyn.h"

#include "best/memory/ptr.h"
#include "best/test/test.h"

namespace best::dyn_test {
class IntHolder : public best::interface_base<IntHolder> {
 public:
  BEST_INTERFACE(IntHolder,              //
                 (int, get, (), const),  //
                 (void, set, (int x)),   //
                 (void, reset, ()));

 private:
  void reset(best::defaulted) { set(0); }
};

class Reset : public best::interface_base<Reset> {
 public:
  BEST_INTERFACE(Reset, (void, reset, ()));
};

constexpr best::vtable<IntHolder> BestImplements(int*, IntHolder*) {
  return {
    .get = [](const int& self) { return -self; },
    .set = [](int& self, int x) { self = x; },
  };
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
                         const best::itable<IntHolder>*>);

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

class Templates {
 public:
  BEST_INTERFACE(Templates, ((best::result<int, int>), get, ()));
};

best::test Ptr = [](best::test& t) {
  int x = 42;
  best::dynptr<IntHolder> p = &x;
  t.expect_eq(p->get(), -42);

  Struct y{42};
  p = &y;
  t.expect_eq(p->get(), 84);
};

struct Struct2 {
  int value;

  int get() const { return value * 2; }
  int set(int x) { return value = x; }
  void reset() { value *= -1; }
};

best::test Default = [](best::test& t) {
  int x = 42;
  Struct y{42};
  Struct2 z(42);

  IntHolder::of(x)->reset();
  IntHolder::of(y)->reset();
  IntHolder::of(z)->reset();
  t.expect_eq(x, 0);
  t.expect_eq(y.value, 0);
  t.expect_eq(z.value, -42);
};

best::test Mixed = [](best::test& t) {
  Struct2 y(42);
  best::dynptr<IntHolder, Reset> p = &y;
  t.expect_eq(p[best::types<IntHolder>]->get(), 84);
  t.expect_eq(IntHolder::of(p)->get(), 84);

  Reset::of(p)->reset();
  t.expect_eq(IntHolder::of(p)->get(), -84);
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

  t.expect_eq(IntHolder::of(x)->get(), -1);
  t.expect_eq(IntHolder::of(y)->get(), 4);
  t.expect_eq(IntHolder::of(&x)->get(), -1);
  t.expect_eq(IntHolder::of(&y)->get(), 4);
  t.expect_eq(IntHolder::of(p)->get(), -1);
  t.expect_eq(IntHolder::of(q)->get(), 4);
};
}  // namespace best::dyn_test
