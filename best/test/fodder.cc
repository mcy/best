#include "best/test/fodder.h"

#include <format>

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
      t_->fail(std::format("unexpected extra {} free(s) of #{}", diff, token));
    } else if (diff < 0) {
      t_->fail(
          std::format("unexpected missing {} free(s) of #{}", -diff, token));
    }
  }

  for (auto [token, destroyed] : destroyed_) {
    t_->fail(std::format("unexpected {} free(s) of uncreated #{}", destroyed,
                         token));
  }
}
}  // namespace best_fodder