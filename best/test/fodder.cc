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

#include "best/test/fodder.h"

#include "best/log/location.h"

namespace best_fodder {
namespace {
thread_local LeakTest* current_ = nullptr;
LeakTest& current(best::location loc = best::here) {
  if (current_ == nullptr) {
    best::crash_internal::crash(
        {"operation requires an ambient LeakTest", loc});
  }
  return *current_;
}
}  // namespace

LeakTest::Bubble::Bubble() : token_(current().next_++) {
  current().created_[token_]++;
}

LeakTest::Bubble::Bubble(const Bubble& that) : token_(that.token_) {
  if (token_ >= 0) {
    current().created_[token_]++;
  }
}

LeakTest::Bubble& LeakTest::Bubble::operator=(const Bubble& that) {
  std::destroy_at(this);
  return *std::construct_at(this);
}

LeakTest::Bubble::Bubble(Bubble&& that)
    : token_(std::exchange(that.token_, -1)) {}

LeakTest::Bubble& LeakTest::Bubble::operator=(Bubble&& that) {
  std::destroy_at(this);
  return *std::construct_at(this, std::move(that));
}

LeakTest::Bubble::~Bubble() {
  if (token_ >= 0) {
    current().destroyed_[token_]++;
  }
}

LeakTest::LeakTest(best::test& t) : t_(&t) {
  if (current_ != nullptr) {
    best::crash_internal::crash("operation requires no active LeakTest");
  }
  current_ = this;
}

LeakTest::~LeakTest() {
  for (auto [token, created] : created_) {
    int diff = destroyed_[token] - created;
    destroyed_.erase(token);

    if (diff > 0) {
      t_->fail("unexpected extra {} free(s) of #{}", diff, token);
    } else if (diff < 0) {
      t_->fail("unexpected missing {} free(s) of #{}", -diff, token);
    }
  }

  for (auto [token, destroyed] : destroyed_) {
    t_->fail("unexpected {} free(s) of uncreated #{}", destroyed, token);
  }
}
}  // namespace best_fodder
