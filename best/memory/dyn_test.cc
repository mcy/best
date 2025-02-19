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

#include "best/base/access.h"
#include "best/test/test.h"
#include "best/text/format.h"

namespace best::dyn_test {
class Iface {
 public:
  friend best::access;

  struct BestVtable {
    best::vtable_header header;
    int (*get)(void*);
  };

  int get() const { return vt_->get(data_); }

  const BestVtable* vtable() const { return vt_; }

 private:
  constexpr Iface(void* data, const BestVtable* vt) : data_(data), vt_(vt) {}

  void* data_;
  const BestVtable* vt_;
};

inline const Iface::BestVtable int_vtable = {
  .header = best::vtable_header::of<int>(),
  .get = +[](void* thiz) { return -*(int*)thiz; }};

constexpr best::vtable<Iface> BestImplements(int*, Iface*) {
  return &int_vtable;
}

struct Struct {
  int value;
};

inline const Iface::BestVtable struct_vtable = {
  .header = best::vtable_header::of<Struct>(),
  .get = +[](void* thiz) { return ((Struct*)thiz)->value * 2; }};

constexpr best::vtable<Iface> BestImplements(Struct*, Iface*) {
  return &struct_vtable;
}

static_assert(best::interface<Iface>);
static_assert(best::implements<int, Iface>);
static_assert(best::implements<Struct, Iface>);
static_assert(
  best::same<best::ptr<best::dyn<Iface>>::metadata, best::vtable<Iface>>);

best::test Ptr = [](best::test& t) {
  int x = 42;
  best::ptr<best::dyn<Iface>> p = &x;
  t.expect_eq(p->get(), -42);

  Struct y{42};
  p = &y;
  t.expect_eq(p->get(), 84);
};

best::test Box = [](best::test& t) {
  best::box<best::dyn<Iface>> p = best::box(42);
  t.expect_eq(p->get(), -42);

  p = best::box(Struct{42});
  t.expect_eq(p->get(), 84);

  auto p2 = p.try_copy();
  best::println("{} {} {}", p->vtable(), p->vtable()->header.dtor, p->vtable()->get);
  best::println("{} {} {}", (*p2)->vtable(), (*p2)->vtable()->header.dtor, (*p2)->vtable()->get);
  t.expect(p2.has_value());
  t.expect_eq((*p2)->get(), 84);
};
}  // namespace best::dyn_test