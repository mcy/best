#include "best/meta/init.h"

#include <memory>

#include "best/container/vec.h"
#include "best/test/fodder.h"

namespace best::init_test {
using ::best_fodder::NonTrivialPod;
using ::best_fodder::Stuck;
using ::best_fodder::TrivialCopy;

static_assert(best::constructible<int>);
static_assert(best::constructible<int, int>);
static_assert(best::constructible<int, long>);
static_assert(best::constructible<int, void>);

static_assert(best::constructible<int, trivially>);
static_assert(best::constructible<int, trivially, int>);
static_assert(best::constructible<int, trivially, long>);
static_assert(best::constructible<int, trivially, void>);

static_assert(!best::constructible<NonTrivialPod, void>);
static_assert(best::constructible<NonTrivialPod, int, int>);
static_assert(best::constructible<NonTrivialPod, const int&, int>);
static_assert(best::constructible<NonTrivialPod, NonTrivialPod>);
static_assert(best::constructible<NonTrivialPod, const NonTrivialPod>);

static_assert(!best::constructible<NonTrivialPod, trivially, void>);
static_assert(!best::constructible<NonTrivialPod, trivially, int, int>);
static_assert(!best::constructible<NonTrivialPod, trivially, const int&, int>);
static_assert(!best::constructible<NonTrivialPod, trivially, NonTrivialPod>);
static_assert(
    !best::constructible<NonTrivialPod, trivially, const NonTrivialPod>);

static_assert(best::constructible<TrivialCopy, void>);
static_assert(best::constructible<TrivialCopy, const TrivialCopy&>);

static_assert(!best::constructible<TrivialCopy, trivially, void>);
static_assert(best::constructible<TrivialCopy, trivially, const TrivialCopy&>);

static_assert(best::constructible<const int&, int&>);
static_assert(best::constructible<const int&, int&&>);
static_assert(!best::constructible<const int&, int>);
static_assert(!best::constructible<int&, const int&>);
static_assert(!best::constructible<int&&, int&>);

static_assert(best::constructible<const int&, trivially, int&>);
static_assert(best::constructible<const int&, trivially, int&&>);
static_assert(!best::constructible<const int&, trivially, int>);
static_assert(!best::constructible<int&, trivially, const int&>);
static_assert(!best::constructible<int&&, trivially, int&>);

static_assert(best::constructible<int(int), int (&)(int)>);
static_assert(best::constructible<void(void), void (&)()>);
static_assert(!best::constructible<int(int), int (*)(int)>);
static_assert(!best::constructible<void(void), void (*)()>);

static_assert(best::constructible<int(int), trivially, int (&)(int)>);
static_assert(best::constructible<void(void), trivially, void (&)()>);
static_assert(!best::constructible<int(int), trivially, int (*)(int)>);
static_assert(!best::constructible<void(void), trivially, void (*)()>);

static_assert(best::constructible<void>);
static_assert(best::constructible<void, void>);
static_assert(best::constructible<void, int>);
static_assert(!best::constructible<void, void, void>);

static_assert(best::constructible<void, trivially>);
static_assert(best::constructible<void, trivially, void>);
static_assert(best::constructible<void, trivially, int>);
static_assert(!best::constructible<void, trivially, void, void>);

static_assert(!best::convertible<int>);
static_assert(best::convertible<int, int>);
static_assert(best::convertible<int, long>);
static_assert(best::convertible<int&, int&>);

static_assert(best::convertible<NonTrivialPod, const NonTrivialPod>);
static_assert(best::convertible<NonTrivialPod, const NonTrivialPod&>);
static_assert(!best::convertible<NonTrivialPod, int, int>);
static_assert(
    !best::convertible<NonTrivialPod, trivially, const NonTrivialPod>);
static_assert(
    !best::convertible<NonTrivialPod, trivially, const NonTrivialPod&>);

static_assert(best::convertible<TrivialCopy, const TrivialCopy&>);
static_assert(best::convertible<TrivialCopy, trivially, const TrivialCopy&>);

static_assert(best::convertible<const int&, int&>);
static_assert(!best::convertible<int&, const int&>);

static_assert(best::convertible<void>);
static_assert(best::convertible<void, void>);
static_assert(best::convertible<void, int>);
static_assert(!best::convertible<void, void, void>);

static_assert(best::assignable<int, int>);
static_assert(best::assignable<int, long>);
static_assert(!best::assignable<int&, int>);
static_assert(!best::assignable<int>);
static_assert(!best::assignable<int, int, int>);

static_assert(best::assignable<int, trivially, int>);
static_assert(best::assignable<int, trivially, long>);

static_assert(!best::assignable<NonTrivialPod, int, int>);
static_assert(best::assignable<NonTrivialPod, NonTrivialPod>);
static_assert(best::assignable<NonTrivialPod, const NonTrivialPod>);
static_assert(!best::assignable<NonTrivialPod, trivially, NonTrivialPod>);
static_assert(!best::assignable<NonTrivialPod, trivially, const NonTrivialPod>);

static_assert(!best::assignable<TrivialCopy>);
static_assert(best::assignable<TrivialCopy, const TrivialCopy&>);
static_assert(best::assignable<TrivialCopy, trivially, const TrivialCopy&>);

static_assert(best::assignable<const int&, int&>);
static_assert(best::assignable<const int&, int&&>);
static_assert(!best::assignable<const int&, int>);
static_assert(!best::assignable<int&, const int&>);
static_assert(!best::assignable<int&&, int&>);

static_assert(best::assignable<const int&, trivially, int&>);
static_assert(best::assignable<const int&, trivially, int&&>);
static_assert(!best::assignable<const int&, trivially, int>);
static_assert(!best::assignable<int&, trivially, const int&>);
static_assert(!best::assignable<int&&, trivially, int&>);

static_assert(best::assignable<void>);
static_assert(best::assignable<void, void>);
static_assert(best::assignable<void, int>);
static_assert(!best::assignable<void, void, void>);

static_assert(best::assignable<void, trivially>);
static_assert(best::assignable<void, trivially, void>);
static_assert(best::assignable<void, trivially, int>);
static_assert(!best::assignable<void, trivially, void, void>);

static_assert(best::copy_constructible<int>);
static_assert(best::copy_constructible<int&>);
static_assert(best::copy_constructible<int()>);
static_assert(best::copy_constructible<void>);
static_assert(best::copy_constructible<NonTrivialPod>);
static_assert(best::copy_constructible<TrivialCopy>);
static_assert(best::copy_constructible<best::vec<int>>);
static_assert(!best::copy_constructible<std::unique_ptr<int>>);
static_assert(!best::copy_constructible<Stuck>);

static_assert(best::copy_constructible<int, trivially>);
static_assert(best::copy_constructible<int&, trivially>);
static_assert(best::copy_constructible<int(), trivially>);
static_assert(best::copy_constructible<void, trivially>);
static_assert(!best::copy_constructible<NonTrivialPod, trivially>);
static_assert(best::copy_constructible<TrivialCopy, trivially>);
static_assert(!best::copy_constructible<best::vec<int>, trivially>);
static_assert(!best::copy_constructible<std::unique_ptr<int>, trivially>);
static_assert(!best::copy_constructible<Stuck, trivially>);

static_assert(best::move_constructible<int>);
static_assert(best::move_constructible<int&>);
static_assert(best::move_constructible<int()>);
static_assert(best::move_constructible<void>);
static_assert(best::move_constructible<NonTrivialPod>);
static_assert(best::move_constructible<TrivialCopy>);
static_assert(best::move_constructible<best::vec<int>>);
static_assert(best::move_constructible<std::unique_ptr<int>>);
static_assert(!best::move_constructible<Stuck>);

static_assert(best::move_constructible<int, trivially>);
static_assert(best::move_constructible<int&, trivially>);
static_assert(best::move_constructible<int(), trivially>);
static_assert(best::move_constructible<void, trivially>);
static_assert(!best::move_constructible<NonTrivialPod, trivially>);
static_assert(best::move_constructible<TrivialCopy, trivially>);
static_assert(!best::move_constructible<best::vec<int>, trivially>);
static_assert(!best::move_constructible<std::unique_ptr<int>, trivially>);
static_assert(!best::move_constructible<Stuck, trivially>);

static_assert(best::copy_assignable<int>);
static_assert(best::copy_assignable<int&>);
static_assert(best::copy_assignable<int()>);
static_assert(best::copy_assignable<void>);
static_assert(best::copy_assignable<NonTrivialPod>);
static_assert(best::copy_assignable<TrivialCopy>);
static_assert(best::copy_assignable<best::vec<int>>);
static_assert(!best::copy_assignable<std::unique_ptr<int>>);
static_assert(!best::copy_assignable<Stuck>);

static_assert(best::copy_assignable<int, trivially>);
static_assert(best::copy_assignable<int&, trivially>);
static_assert(best::copy_assignable<int(), trivially>);
static_assert(best::copy_assignable<void, trivially>);
static_assert(!best::copy_assignable<NonTrivialPod, trivially>);
static_assert(best::copy_assignable<TrivialCopy, trivially>);
static_assert(!best::copy_assignable<best::vec<int>, trivially>);
static_assert(!best::copy_assignable<std::unique_ptr<int>, trivially>);
static_assert(!best::copy_assignable<Stuck, trivially>);

static_assert(best::move_assignable<int>);
static_assert(best::move_assignable<int&>);
static_assert(best::move_assignable<int()>);
static_assert(best::move_assignable<void>);
static_assert(best::move_assignable<NonTrivialPod>);
static_assert(best::move_assignable<TrivialCopy>);
static_assert(best::move_assignable<best::vec<int>>);
static_assert(best::move_assignable<std::unique_ptr<int>>);
static_assert(!best::move_assignable<Stuck>);

static_assert(best::move_assignable<int, trivially>);
static_assert(best::move_assignable<int&, trivially>);
static_assert(best::move_assignable<int(), trivially>);
static_assert(best::move_assignable<void, trivially>);
static_assert(!best::move_assignable<NonTrivialPod, trivially>);
static_assert(best::move_assignable<TrivialCopy, trivially>);
static_assert(!best::move_assignable<best::vec<int>, trivially>);
static_assert(!best::move_assignable<std::unique_ptr<int>, trivially>);
static_assert(!best::move_assignable<Stuck, trivially>);

static_assert(best::copyable<int>);
static_assert(best::copyable<int&>);
static_assert(best::copyable<int()>);
static_assert(best::copyable<void>);
static_assert(best::copyable<NonTrivialPod>);
static_assert(best::copyable<TrivialCopy>);
static_assert(best::copyable<best::vec<int>>);
static_assert(!best::copyable<std::unique_ptr<int>>);
static_assert(!best::copyable<Stuck>);

static_assert(best::copyable<int, trivially>);
static_assert(best::copyable<int&, trivially>);
static_assert(best::copyable<int(), trivially>);
static_assert(best::copyable<void, trivially>);
static_assert(!best::copyable<NonTrivialPod, trivially>);
static_assert(best::copyable<TrivialCopy, trivially>);
static_assert(!best::copyable<best::vec<int>, trivially>);
static_assert(!best::copyable<std::unique_ptr<int>, trivially>);
static_assert(!best::copyable<Stuck, trivially>);

static_assert(best::moveable<int>);
static_assert(best::moveable<int&>);
static_assert(best::moveable<int()>);
static_assert(best::moveable<void>);
static_assert(best::moveable<NonTrivialPod>);
static_assert(best::moveable<TrivialCopy>);
static_assert(best::moveable<best::vec<int>>);
static_assert(best::moveable<std::unique_ptr<int>>);
static_assert(!best::moveable<Stuck>);

static_assert(best::moveable<int, trivially>);
static_assert(best::moveable<int&, trivially>);
static_assert(best::moveable<int(), trivially>);
static_assert(best::moveable<void, trivially>);
static_assert(!best::moveable<NonTrivialPod, trivially>);
static_assert(best::moveable<TrivialCopy, trivially>);
static_assert(!best::moveable<best::vec<int>, trivially>);
static_assert(!best::moveable<std::unique_ptr<int>, trivially>);
static_assert(!best::moveable<Stuck, trivially>);

static_assert(best::relocatable<best_fodder::Relocatable, trivially>);
}  // namespace best::init_test

int main() {}