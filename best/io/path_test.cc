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

#include "best/io/path.h"

#include "best/test/test.h"

namespace best::path_test {
best::test Components = [](best::test& t) {
  auto c = [](best::path p) {
    return p.components().collect<best::vec<best::path>>();
  };
  auto r = [](best::path p) {
    return p.components().rev().collect<best::vec<best::path>>();
  };

  t.expect_eq(c(""), {""});
  t.expect_eq(c("/"), {"/"});
  t.expect_eq(c("."), {"."});
  t.expect_eq(c("/a"), {"/", "a"});
  t.expect_eq(c("/a/b"), {"/", "a", "b"});
  t.expect_eq(c("//a/b"), {"/", "a", "b"});
  t.expect_eq(c("/a//b"), {"/", "a", "b"});
  t.expect_eq(c("/a/b/"), {"/", "a", "b"});
  t.expect_eq(c("/a/b/."), {"/", "a", "b"});
  t.expect_eq(c("./a/b"), {".", "a", "b"});
  t.expect_eq(c("a/b"), {"a", "b"});
  t.expect_eq(c("a//b"), {"a", "b"});
  t.expect_eq(c("a/b/c"), {"a", "b", "c"});
  t.expect_eq(c("a/../c"), {"a", "..", "c"});

  t.expect_eq(r(""), {""});
  t.expect_eq(r("/"), {"/"});
  t.expect_eq(r("."), {"."});
  t.expect_eq(r("/a"), {"a", "/"});
  t.expect_eq(r("/a/b"), {"b", "a", "/"});
  t.expect_eq(r("//a/b"), {"b", "a", "/"});
  t.expect_eq(r("/a//b"), {"b", "a", "/"});
  t.expect_eq(r("/a/b/"), {"b", "a", "/"});
  t.expect_eq(r("/a/b/."), {"b", "a", "/"});
  t.expect_eq(r("./a/b"), {"b", "a", "."});
  t.expect_eq(r("a/b"), {"b", "a"});
  t.expect_eq(r("a//b"), {"b", "a"});
  t.expect_eq(r("a/b/c"), {"c", "b", "a"});
  t.expect_eq(r("a/../c"), {"c", "..", "a"});
};
}