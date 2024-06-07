#ifndef BEST_TEST_FODDER_H_
#define BEST_TEST_FODDER_H_

#include <map>
#include <ostream>
#include <type_traits>

#include "best/base/port.h"
#include "best/test/test.h"

//! Testing fodder.
//!
//! Types that implement some kind of common property that are useful for
//! testing generic code.

namespace best_fodder {
/// A non-trivial type whose copy constructor is observable.
class NonTrivialPod final {
 public:
  constexpr NonTrivialPod(int x, int y) : x_(x), y_(y) {}
  constexpr NonTrivialPod(const NonTrivialPod& that)
      : x_(that.x_), y_(that.y_) {}
  constexpr NonTrivialPod& operator=(const NonTrivialPod& that) {
    x_ = that.x_;
    y_ = that.y_;
    return *this;
  }

  constexpr int x() const { return x_; }
  constexpr int y() const { return y_; }

 private:
  int x_, y_;
};
static_assert(!std::is_trivially_copyable_v<NonTrivialPod>);

/// A type with an observable non-trivial destructor that is otherwise trivial.
class NonTrivialDtor final {
 public:
  constexpr NonTrivialDtor() = default;
  constexpr NonTrivialDtor(const NonTrivialDtor&) = default;
  constexpr NonTrivialDtor& operator=(const NonTrivialDtor&) = default;
  constexpr NonTrivialDtor(NonTrivialDtor&&) = default;
  constexpr NonTrivialDtor& operator=(NonTrivialDtor&&) = default;

  constexpr NonTrivialDtor(int* target, int value)
      : target_(target), value_(value) {}

  constexpr ~NonTrivialDtor() {
    if (target_ != nullptr) {
      *target_ = value_;
    }
  }

 private:
  int* target_ = nullptr;
  int value_ = 0;
};

/// A trivially relocatable type.
class BEST_RELOCATABLE Relocatable final {
 public:
  constexpr Relocatable() = default;
  constexpr Relocatable(const Relocatable&) {}
  constexpr Relocatable& operator=(const Relocatable&) { return *this; }
  constexpr ~Relocatable(){};
};

/// A non-trivial, trivially copyable relocatable type.
class TrivialCopy final {
 public:
  constexpr TrivialCopy() {}
  constexpr TrivialCopy(const TrivialCopy&) = default;
  constexpr TrivialCopy& operator=(const TrivialCopy&) = default;
};

class Stuck final {
 public:
  constexpr Stuck() {}
  constexpr Stuck(const Stuck&) = delete;
  constexpr Stuck& operator=(const Stuck&) = delete;
};

/// A helper for verifying that a type does not leak or double-free values.
///
/// Within the scope of a LeakTest, LeakTest::Bubbles may be created. When it
/// is destroyed, LeakTest will generate assertions to verify that every
/// Bubble created was correctly destroyed exactly one.
class LeakTest final {
 public:
  /// A token for detecting whether constructors and destructors are run as
  /// appropriate.
  class BEST_RELOCATABLE Bubble final {
   public:
    Bubble();
    Bubble(const Bubble&);
    Bubble& operator=(const Bubble&);
    Bubble(Bubble&&);
    Bubble& operator=(Bubble&&);
    ~Bubble();

    friend std::ostream& operator<<(std::ostream& os, const Bubble& b) {
      if (b.token_ < 0) {
        return os << "<moved>";
      }
      return os << "#" << b.token_;
    }

   private:
    int token_ = -1;
  };

  /// Creates a new LeakTest in the context of some unit test.
  LeakTest(best::test&);
  ~LeakTest();

  LeakTest(const LeakTest&) = delete;
  LeakTest& operator=(const LeakTest&) = delete;

 private:
  friend Bubble;

  best::test* t_;
  int next_ = 0;
  std::map<int, int> created_, destroyed_;
};

}  // namespace best_fodder

#endif  // BEST_TEST_FODDER_H_