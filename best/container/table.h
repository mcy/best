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

#ifndef BEST_CONTAINER_TABLE_H_
#define BEST_CONTAINER_TABLE_H_

#include "best/base/fwd.h"
#include "best/base/hint.h"
#include "best/base/tags.h"
#include "best/base/unsafe.h"
#include "best/container/internal/table.h"
#include "best/container/option.h"
#include "best/hash/hash.h"
#include "best/iter/bounds.h"
#include "best/log/location.h"
#include "best/log/wtf.h"
#include "best/memory/allocator.h"
#include "best/memory/layout.h"
#include "best/meta/init.h"
#include "best/meta/traits/empty.h"
#include "best/meta/traits/pairs.h"
#include "best/meta/traits/refs.h"
#include "best/meta/traits/types.h"
#include "best/text/format.h"

namespace best {
/// # `best::no_insert`
///
/// Type tag for marking a `best::table::entry` as non-inserting.
struct no_insert final {};

template <typename Table, typename Q = best::no_insert>
class table_entry;

/// # `best::table<K, V>`
///
/// `best::table` is a Swisstable implementation, a generic hash table type that
/// can be used as either a map or a set.
///
/// The API is radically different from that of `std::unordered_map`, which it
/// replaces. Instead of having a variety of lookup operations, `best::table`
/// has exactly one: `operator[]`. Depending on which overload is picked, this
/// will return an accessor for the corresponding entry (sort of like an
/// enhanced `best::option<best::row<K, V>>`); the mutable version is
/// `best::table::inserter`, which can be used to create an entry in the table.
///
/// For example, inserting a value into the table looks like this:
///
/// ```
/// best::table<int, best::strbuf> ints;
/// ints[42].insert("hello!");
/// ```
///
/// Removal is similar: `ints[42].remove()`, which move the associated entry
/// out of the map. Accessing members of a value is as simple as
/// `my_table[k]->foo()`, assuming `k` is present. Operating on possibly absent
/// members has a clean idiom:
///
/// ```
/// if (auto v = my_table[k]) {
///   v->do_something();
/// }
/// ```
///
/// When `V` is `void`, this type takes on a set-like interface. Almost
/// everything is the same, except that `operator*` and `operator->` on an entry
/// will operate on the key rather than the value.
template <typename K, typename V = void,
          best::abridged Policy = best::abridge<best::table_internal::policy<>>>
class BEST_RELOCATABLE table final {
 private:
  using policy = best::unabridge<Policy>;
  using group = best::table_internal::group;
  using ctrl = best::table_internal::ctrl;

 public:
  using key_type = K;
  using value_type = V;
  using identity = policy::identity;
  using hash_state = policy::hash_state;
  using allocator = policy::allocator;

  static_assert(best::hash_identity<identity, K>);

  using key_cref = best::as_ref<const key_type>;
  using key_ref = best::as_ref<key_type>;
  using key_crref = best::as_rref<const key_type>;
  using key_rref = best::as_rref<key_type>;
  using key_cptr = best::as_raw_ptr<const key_type>;
  using key_ptr = best::as_raw_ptr<key_type>;

  using value_cref = best::as_ref<const value_type>;
  using value_ref = best::as_ref<value_type>;
  using value_crref = best::as_rref<const value_type>;
  using value_rref = best::as_rref<value_type>;
  using value_cptr = best::as_raw_ptr<const value_type>;
  using value_ptr = best::as_raw_ptr<value_type>;

  // clang-format off
  /// # `table::with_identity<I>`, `table::with_hash<H>`, `table::with_allocator<A>`
  ///
  /// Aliases for creating a new table type with the given configuration
  /// changes.
  // clang-format on
  template <best::hash_identity<K> I>
  using with_identity = best::table<
    K, V,
    best::abridge<best::table_internal::policy<I, hash_state, allocator>>>;
  template <best::hash_state H>
  using with_hash = best::table<
    K, V, best::abridge<best::table_internal::policy<identity, H, allocator>>>;
  template <best::allocator A>
  using with_allocator = best::table<
    K, V, best::abridge<best::table_internal::policy<identity, hash_state, A>>>;

  /// # `table::const_entry`, `table::entry`
  ///
  /// This tables's entry types.
  using const_entry = best::table_entry<const table>;
  using entry = best::table_entry<table>;

  /// # `table::inserter`
  ///
  /// This table's inserter type, used for performing mutation operations.
  template <typename Q>
  using inserter = best::table_entry<table, Q>;

  /// # `table::table()`
  ///
  /// Constructs a new empty table.
  table() = default;
  explicit table(identity identity, allocator allocator = {})
    : identity_(BEST_MOVE(identity)), alloc_(BEST_MOVE(allocator)) {}

  /// # `table::table { ... }`
  ///
  /// Constructs a new empty table with the given elements.
  table(std::initializer_list<best::row<K, V>> il) requires (!best::is_void<V>)
  {
    reserve(il.size());
    for (const auto& [k, v] : il) { query(k).insert(v); }
  }
  template <typename K2>
  table(std::initializer_list<K2> il) requires best::is_void<V>
  {
    reserve(il.size());
    for (const auto& k : il) { query(k).insert(); }
  }

  /// # `vec::vec(iterator)`
  ///
  /// Constructs a vector by emptying an iterator.
  template <is_iter Iter>
  explicit table(Iter&& iter)
    requires (!best::is_void<V>) &&
             best::constructible<K, best::first_in<best::iter_type<Iter>>> &&
             best::constructible<V, best::second_in<best::iter_type<Iter>>>
  {
    reserve(iter.size_hint().lower);
    for (auto&& [k, v] : iter) { query(BEST_FWD(k)).insert(BEST_FWD(v)); }
  }
  template <is_iter Iter>
  explicit table(Iter&& iter)
    requires best::is_void<V> && best::constructible<K, best::iter_type<Iter>>
  {
    reserve(iter.size_hint().lower);
    for (auto&& k : iter) { query(BEST_FWD(k)).insert(); }
  }

  /// # `table::table(table)`
  ///
  /// Copy and move constructors.
  table(const table&) requires best::copyable<K> && best::copyable<V>;
  table& operator=(const table&)  //
    requires best::copyable<K> && best::copyable<V>;
  table(table&&);
  table& operator=(table&&);

  /// # `table::~table()`
  ///
  /// Destroys this table.
  ~table() {
    raw_.destroy();
    free();
  }

  /// # `table::size()`
  ///
  /// Returns the number of entries in the table.
  size_t size() const { return size_; }

  /// # `table::capacity()`
  ///
  /// Returns the table's capacity, i.e. the total number of entries it can
  /// contain before a rehash happens.
  size_t capacity() const { return size_ + raw_.cap().soft; }

  /// # `table::operator[]`
  ///
  /// Extracts an entry from the table using the given query.
  ///
  /// This will not trigger an insertion if the key is not present; to do so,
  /// use the member functions on the returned `table::inserter`.
  template <typename Q>
  const_entry operator[](Q&& query) const
    requires best::hash_identity<identity, K, Q>;
  template <typename Q>
  inserter<Q> operator[](Q&& query) requires best::hash_identity<identity, K, Q>
  ;

  /// # `table::query()`
  ///
  /// Identical to `operator[]`; exists only to be used when accessing via `->`.
  /// Prefer `operator[]` in general.
  template <typename Q>
  const_entry query(Q&& query) const
    requires best::hash_identity<identity, K, Q>
  {
    return operator[](BEST_FWD(query));
  }
  template <typename Q>
  inserter<Q> query(Q&& query) requires best::hash_identity<identity, K, Q>
  {
    return operator[](BEST_FWD(query));
  }

  /// # `table::contains()`
  ///
  /// Returns whether an entry exists in this table matching the given query.
  template <typename Q>
  bool contains(Q&& query) const {
    return this->query(BEST_FWD(query)).is_occupied();
  }

  /// # `table::const_iterator`, `table::iterator`
  ///
  /// This tables's iterator types.
  using const_iterator = best::iter<table_internal::iter_impl<const table>>;
  using iterator = best::iter<table_internal::iter_impl<table>>;

  /// # `table::iter()`, `table::begin()`, `table::end()`.
  ///
  /// Tables are iterable; the yielded values are entries, which support
  /// structured bindings, but, in the case of the non-const iterator, can also
  /// be used for removal.
  const_iterator iter() const {
    return const_iterator(table_internal::iter_impl<const table>(this));
  }
  iterator iter() { return iterator(table_internal::iter_impl<table>(this)); }
  auto begin() const { return iter().into_range(); }
  auto begin() { return iter().into_range(); }
  auto end() const { return best::iter_range_end{}; }

  /// # `table::clear()`
  ///
  /// Destroys all entries in the table without freeing capacity. This will
  /// always be faster than calling `e.remove()` in a loop.
  void clear() {
    raw_.destroy();
    raw_.reset_ctrl();
    raw_.cap().reset_soft();
    size_ = 0;
  }

  /// # `table::reserve()`
  ///
  /// Ensures that inserting another `capacity` elements will not trigger a
  /// rehash of the table.
  void reserve(size_t capacity);

  /// # `table::rehash()`
  ///
  /// Shuffles around the internal hash table data structure to improve future
  /// lookups. May cause reallocation.
  void rehash();

  /// # `table::hash_value`
  ///
  /// A hash calculated for some query. See `table::hash()`.
  class hash_value final {
   public:
    constexpr hash_value() = default;

   private:
    friend table;
    template <typename, typename>
    friend class best::table_entry;

    table_internal::hash h_;
  };

  /// # `table::hash()`
  ///
  /// Calculates the hash for some query.
  /// The returned hash becomes invalid if the table is resized or rehashed.
  template <typename Q>
  requires best::hash_identity<identity, K, Q>
  hash_value hash(const Q& query) const {
    return hash(query, raw_.ptr());
  }

  /// # `table::debug()`
  ///
  /// Dumps debugging information about this table.
  best::strbuf debug(bool include_entries = true) const;

 private:
  template <typename, typename>
  friend class best::table_entry;
  template <typename>
  friend class best::table_internal::iter_impl;

  template <typename Q>
  requires best::hash_identity<identity, K, Q>
  hash_value hash(const Q& query, const void* ctrl) const {
    hash_value h;
    h.h_ = best::table_internal::hash::compute<K, hash_state>(query, identity_,
                                                              ctrl);
    return h;
  }

  best::option<size_t> search(hash_value hash, const auto& query) const;
  best::row<size_t, bool> maybe_preinsert(hash_value hash, const auto& query);
  size_t preinsert(hash_value hash, const auto& query);

  template <bool relo>
  table_internal::array<K, V> rehash_to(table_internal::capacity new_cap,
                                        const table& target) const;

  void free() {
    if (raw_.ptr() != nullptr) {
      alloc_.dealloc(best::ptr(raw_.ptr()), raw_.cap().template layout<K, V>());
    }
  }

  table_internal::array<K, V> raw_{};
  size_t size_{};
  [[no_unique_address]] identity identity_;
  [[no_unique_address]] allocator alloc_;
};

/// # `best::table_entry<Table, Query>`
///
/// An entry in a table (a bucket). All operations on the table use entries
/// in some way. Entries are ephemeral; whenever a rehash occurs, all entries
/// returned by a table are invalidated.
///
/// To obtain a value of this type, use `best::table::operator[]`.
template <typename Table, typename Query>
class table_entry final {
 private:
  using group = best::table_internal::group;
  using ctrl = best::table_internal::ctrl;

 public:
  using table = Table;
  static constexpr bool is_const = best::is_const<table>;

  using key_type = table::key_type;
  using value_type = table::value_type;
  using query_type = Query;

  using kref = table::key_cref;
  using vref = best::select<is_const, typename table::value_cref,
                            typename table::value_ref>;

  static constexpr bool CanInsert = !best::same<Query, best::no_insert>;

  /// # `table_entry::table_entry()`
  ///
  /// Creates a new vacant entry which cannot be inserted with. Attempting to
  /// insert
  constexpr table_entry() requires best::constructible<query_type>
    : idx_(-1) {}

  /// # `table_entry::is_vacant()`, `table_entry::is_occupied()`
  ///
  /// Whether this entry is currently occupied by a pair or not.
  bool is_vacant() const { return best::to_signed(idx_) < 0; }
  bool is_occupied() const { return !is_vacant(); }

  /// # `table_entry::pair()`, `table_entry::key()`, `table_entry::value()`
  ///
  /// If the entry is occupied, returns the key/value for the entry.
  best::option<kref> key() const;
  best::option<vref> value() const;
  best::option<best::row<kref, vref>> pair() const;

  /// # `table_entry::value_or()`
  ///
  /// Forwards to `value().value_or()`, which comes off as very silly otherwise.
  template <typename... Args>
  decltype(auto) value_or(Args&&... args) const {
    return value().value_or(BEST_FWD(args)...);
  }

  /// # `table_entry::operator*`, `table_entry::operator->`
  ///
  /// Allow access to the value. Panic if this entry is vacant.
  decltype(auto) operator*() const {
    check_occupied();
    if constexpr (best::is_void<value_type>) {
      return *key();
    } else {
      return *value();
    }
  }
  auto* operator->() const {
    check_occupied();
    if constexpr (best::is_void<value_type>) {
      return key().operator->();
    } else {
      return value().operator->();
    }
  }
  explicit operator bool() const { return is_occupied(); }

  /// # `table_entry::insert()`, `table_entry::or_insert()`
  ///
  /// Inserts a new value, constructing it in-place using the given arguments.
  /// `insert()` does so unconditionally, destroying an existing value in the
  /// process. `or_insert()` only does so if this entry is vacant.
  ///
  /// Returns a reference to the inserted value.
  template <typename... Args>
  vref insert(Args&&... args) requires CanInsert;
  template <typename... Args>
  vref or_insert(Args&&... args) requires CanInsert;

  /// # `option::or_insert([] { ... })`
  ///
  /// Like `best::option::value_or([] { ... })`.
  template <typename... Args>
  vref or_insert(best::callable<value_type()> auto&& or_else) requires CanInsert
  ;

  operator best::table_entry<table>() const {
    return best::table_entry<table>(table_, idx_);
  }

  /// # `table_entry::remove()`, `table_entry::erase()`
  ///
  /// Removes this entry, if it was occupied. `erase` is like `remove`, but it
  /// does not return a value.
  best::option<best::row<key_type, value_type>> remove() &&;
  void erase() &&;

 private:
  friend table;
  template <typename, typename>
  friend class best::table_entry;
  template <typename>
  friend class best::table_internal::iter_impl;

  explicit table_entry(table* table, size_t idx, table::hash_value hash,
                       best::object<Query> query) requires CanInsert
    : table_(table), idx_(idx), insert_info_{hash, BEST_MOVE(query)} {}

  explicit table_entry(table* table, size_t idx) requires (!CanInsert)
    : table_(table), idx_(idx) {}

  template <size_t i>
  requires (i < 2)
  friend decltype(auto) get(const table_entry& entry,
                            best::location loc = best::here) {
    entry.check_occupied(loc);
    if constexpr (i == 0) {
      return entry.key().value(best::unsafe{"checked above"});
    } else {
      return entry.value().value(best::unsafe{"checked above"});
    }
  }

  void check_occupied(best::location loc = best::here) const {
    if (best::unlikely(is_vacant())) {
      if constexpr (CanInsert) {
        best::wtf({"accessed a vacant table::entry with query ({:?})", loc},
                  best::make_formattable(insert_info_.query.or_empty()));
      }
      best::wtf({"accessed a vacant table::entry", loc});
    }
  }

  best::row<key_type, value_type> move_pair();
  void destroy_pair();

  static constexpr size_t Full = ~(best::max_of<size_t> >> 1);

  table* table_;
  size_t idx_;  // Negative if not present in the table. If an insertion
                // position is known, it is ~ that index. If not, because the
                // table is at-capacity, this will be Full.

  // Fields necessary to execute an insertion.
  struct insert_info {
    table::hash_value hash;
    best::object<Query> query;
  };
  [[no_unique_address]] best::select<CanInsert, insert_info, best::empty>
    insert_info_;
};
}  // namespace best

/* ////////////////////////////////////////////////////////////////////////// *\
 * ////////////////// !!! IMPLEMENTATION DETAILS BELOW !!! ////////////////// *
\* ////////////////////////////////////////////////////////////////////////// */

// Enable structured bindings.
namespace std {
template <typename T, typename Q>
struct tuple_size<::best::table_entry<T, Q>> {
  static constexpr size_t value = 2;
};
template <typename T, typename Q>
struct tuple_element<0, ::best::table_entry<T, Q>> {
  using type = ::best::table_entry<T, Q>::kref;
};
template <typename T, typename Q>
struct tuple_element<1, ::best::table_entry<T, Q>> {
  using type = ::best::table_entry<T, Q>::vref;
};
}  // namespace std

namespace best {
template <typename K, typename V, best::abridged P>
template <typename Q>
auto table<K, V, P>::operator[](Q&& query) const -> const_entry
  requires best::hash_identity<identity, K, Q>
{
  auto hash = this->hash(query);
  return const_entry(this, search(hash, query).value_or(-1));
}

template <typename K, typename V, best::abridged P>
template <typename Q>
auto table<K, V, P>::operator[](Q&& query) -> inserter<Q>
  requires best::hash_identity<identity, K, Q>
{
  auto hash = this->hash(query);
  auto found = search(hash, query);
  if (!found) {
    found = raw_.vacant(hash.h_).map([&](auto i) { return ~i; });
  }

  return inserter<Q>(this, found.value_or(inserter<Q>::Full), hash,
                     best::object<Q>(best::in_place, BEST_FWD(query)));
}

template <typename T, typename Q>
auto table_entry<T, Q>::key() const -> best::option<kref> {
  if (is_vacant()) { return best::none; }

  return table_->raw_.key(idx_).deref();
}

template <typename T, typename Q>
auto table_entry<T, Q>::value() const -> best::option<vref> {
  if (is_vacant()) { return best::none; }

  if constexpr (best::is_void<value_type>) {
    return best::option<vref>(best::in_place);
  } else {
    return table_->raw_.value(idx_).deref();
  }
}

template <typename T, typename Q>
auto table_entry<T, Q>::pair() const -> best::option<best::row<kref, vref>> {
  if (is_vacant()) { return best::none; }

  using row = best::row<kref, vref>;
  if constexpr (best::is_void<value_type>) {
    return best::option(row{table_->raw_.key(idx_).deref(), {}});
  } else {
    return best::option(row{
      table_->raw_.key(idx_).deref(),
      table_->raw_.value(idx_).deref(),
    });
  }
}

template <typename T, typename Q>
auto table_entry<T, Q>::move_pair() -> best::row<key_type, value_type> {
  using row = best::row<key_type, value_type>;
  if constexpr (best::is_void<value_type>) {
    return row{BEST_MOVE(table_->raw_.key(idx_).deref()), {}};
  } else {
    return row{
      BEST_MOVE(table_->raw_.key(idx_).deref()),
      BEST_MOVE(table_->raw_.value(idx_).deref()),
    };
  }
}

template <typename T, typename Q>
void table_entry<T, Q>::destroy_pair() {
  table_->raw_.key(idx_).destroy();
  if constexpr (!best::is_void<value_type>) {
    table_->raw_.value(idx_).destroy();
  }
}

template <typename T, typename Q>
template <typename... Args>
auto table_entry<T, Q>::insert(Args&&... args) -> vref requires CanInsert
{
  best::debug_must(table_ != nullptr,
                   "called insert() on default-constructed best::table_entry");

  if (is_vacant()) { return BEST_MOVE(*this).or_insert(BEST_FWD(args)...); }

  table_->raw_.key(idx_).assign(BEST_MOVE(*insert_info_.query));

  auto value = table_->raw_.value(idx_);
  value.assign(BEST_FWD(args)...);
  return *value;
}

template <typename T, typename Q>
template <typename... Args>
auto table_entry<T, Q>::or_insert(Args&&... args) -> vref requires CanInsert
{
  best::debug_must(table_ != nullptr,
                   "called insert() on default-constructed best::table_entry");

  bool vacant = is_vacant();
  if (vacant) {
    if (idx_ == Full) {
      table_->rehash();
      insert_info_.hash = table_->hash(*insert_info_.query);
      idx_ = table_->raw_.vacant_unchecked(insert_info_.hash.h_);
    } else {
      idx_ = ~idx_;
    }

    table_->raw_.set_ctrl(idx_, insert_info_.hash.h_.h2);
    table_->raw_.key(idx_).construct(*BEST_MOVE(insert_info_.query));
    ++table_->size_;
  }

  auto ptr = table_->raw_.value(idx_);
  if (vacant) { ptr.construct(BEST_FWD(args)...); }

  return *ptr;
}

template <typename T, typename Q>
auto table_entry<T, Q>::remove() && -> best::option<
                                      best::row<key_type, value_type>> {
  best::debug_must(table_ != nullptr,
                   "called insert() on default-constructed best::table_entry");
  if (is_vacant()) { return best::none; }

  auto removed = move_pair();
  BEST_MOVE(*this).erase();
  return removed;
}

template <typename T, typename Q>
void table_entry<T, Q>::erase() && {
  if (is_vacant()) { return; };
  destroy_pair();

  table_->size_--;
  auto next = idx_;
  auto prev = (idx_ - size_of<group>)&(table_->raw_.cap().hard - 1);

  auto next_empty = group::load(table_->raw_.ptr(), next).match_empty();
  auto prev_empty = group::load(table_->raw_.ptr(), prev).match_empty();

  bool was_never_full =
    !next_empty.empty() && !prev_empty.empty() &&
    next_empty.trailing_zeros() + prev_empty.leading_zeros() < sizeof(group);

  table_->raw_.set_ctrl(idx_, was_never_full ? ctrl::Empty : ctrl::Tombstone);
  table_->raw_.cap().soft += was_never_full;

  table_ = nullptr;
  idx_ = -1;
}

namespace table_internal {
template <typename Table>
class iter_impl final {
  using table = Table;

 public:
  best::option<best::table_entry<table>> next() {
    while (cur_ != end_) {
      auto cur = *cur_++;
      if (cur.is_occupied()) {
        --count_;
        return best::table_entry<table>(table_, cur_ - start_ - 1);
      }
    }

    return best::none;
  }

  best::size_hint size_hint() const {
    return {.lower = count_, .upper = count_};
  }
  size_t count() && { return count_; }

 private:
  friend table;
  template <typename, typename>
  friend class table_entry;

  explicit iter_impl(table* table) : table_(table), count_(table->size_) {
    start_ = table_->raw_.ptr();
    cur_ = start_;
    end_ = start_ + table_->raw_.cap().hard;
  }

  table* table_;
  const ctrl *start_, *cur_, *end_;
  size_t count_;
};
}  // namespace table_internal

template <typename K, typename V, best::abridged P>
table<K, V, P>::table(const table& that)
  requires best::copyable<K> && best::copyable<V>
  : table(0, that.identity_, that.alloc_) {
  *this = that;
}

template <typename K, typename V, best::abridged P>
auto table<K, V, P>::operator=(const table& that)
  -> table& requires best::copyable<K> && best::copyable<V>
{
  raw_.destroy();
  raw_ = that.rehash_to<false>(that.cap_, *this);
  size_ = that.size_;

  return *this;
}

template <typename K, typename V, best::abridged P>
table<K, V, P>::table(table&& that)
  : raw_(that.raw_),
    size_(that.size_),
    identity_(BEST_MOVE(that.identity_)),
    alloc_(BEST_MOVE(that.alloc_)) {
  that.raw_ = {};
  that.size_ = 0;
}

template <typename K, typename V, best::abridged P>
auto table<K, V, P>::operator=(table&& that) -> table& {
  raw_.destroy();

  size_ = that.size_;
  raw_ = that.raw_;
  identity_ = BEST_MOVE(that.identity_);
  alloc_ = BEST_MOVE(that.alloc_);

  that.raw_ = {};
  that.size_ = 0;
  return *this;
}

template <typename K, typename V, best::abridged P>
void table<K, V, P>::reserve(size_t n) {
  auto new_size = size() + n;
  if (new_size <= capacity()) { return; }

  table_internal::capacity new_cap(new_size);
  auto new_array = rehash_to<true>(new_cap, *this);

  free();
  raw_ = new_array;
}

template <typename K, typename V, best::abridged P>
void table<K, V, P>::rehash() {
  if (
    raw_.cap().hard < sizeof(group) ||
    // See
    // https://github.com/google/cwisstable/blob/main/cwisstable/internal/raw_table.h#L445.
    size_ * uint64_t(32) > raw_.cap().hard * uint64_t(25)) {
    reserve(1);
    return;
  }

  for (auto i : best::bounds{.count = raw_.cap().group_count()}) {
    i *= sizeof(group);
    group::load(raw_.ptr(), i).prepare_for_rehash().store(raw_.ptr(), i);
  }

  for (size_t i = 0; i < raw_.cap().hard; ++i) {
    if (!raw_.ctrl(i).is_tombstone()) { continue; }

    auto old_key = raw_.key(i);
    auto old_value = raw_.value(i);
    auto h = hash(*old_key);

    size_t new_i = raw_.vacant_unchecked(h.h_);
    auto new_key = raw_.key(i);
    auto new_value = raw_.value(i);

    // Verify if the old and new i fall within the same group wrt the hash.
    // If they do, we don't need to move the object as it falls already in the
    // best probe we can.
    auto mask = raw_.cap().hard - 1;
    size_t probe_offset = h.h_.h1 & mask;
    auto probe_index = [probe_offset, mask](size_t i) {
      return ((i - probe_offset) & mask) / sizeof(group);
    };

    if (best::likely(probe_index(i) == probe_index(new_i))) {
      // Don't move.
      raw_.set_ctrl(i, h.h_.h2);
      continue;
    }

    if (raw_.ctrl(new_i).is_empty()) {
      // Relocate to new slot.
      raw_.set_ctrl(new_i, h.h_.h2);
      new_key.relo(old_key);
      if constexpr (!best::is_void<V>) { new_value.relo(old_value); }
      raw_.set_ctrl(i, ctrl::Empty);

      continue;
    }

    best::debug_must(raw_.ctrl(new_i).is_tombstone(),
                     "corrupt control byte: {:?}", raw_.ctrl(new_i));
    raw_.set_ctrl(new_i, h.h_.h2);

    // TODO: Swap.
    best::object<K> tmp{best::in_place, BEST_MOVE(*old_key)};
    old_key.move_assign(new_key);
    new_key.move_assign(tmp.as_ptr());
    if constexpr (!best::is_void<V>) {
      best::object<V> tmp{best::in_place, BEST_MOVE(*old_value)};
      old_value.move_assign(new_value);
      new_value.move_assign(tmp.as_ptr());
    }

    --i;  // Try again.
  }

  raw_.cap().reset_soft();
  raw_.cap().soft -= size_;
}

template <typename K, typename V, best::abridged P>
template <bool relo>
table_internal::array<K, V> table<K, V, P>::rehash_to(
  table_internal::capacity new_cap, const table& target) const {
  table_internal::array<K, V> new_raw{
    target.alloc_.alloc(new_cap.layout<K, V>()),
    new_cap,
  };

  for (auto entry : iter()) {
    size_t idx_rhs = entry.idx_;
    auto key_rhs = raw_.key(idx_rhs);
    auto value_rhs = raw_.value(idx_rhs);

    auto hash = target.hash(*key_rhs, new_raw.ptr());
    size_t idx_lhs = new_raw.vacant_unchecked(hash.h_);

    auto key_lhs = new_raw.key(idx_lhs);
    auto value_lhs = new_raw.value(idx_lhs);

    if constexpr (relo) {
      key_lhs.relo(key_rhs);
    } else {
      key_lhs.copy(key_rhs);
    }
    if constexpr (!best::is_void<V>) {
      if constexpr (relo) {
        value_lhs.relo(value_rhs);
      } else {
        value_lhs.copy(value_rhs);
      }
    }

    new_raw.set_ctrl(idx_lhs, hash.h_.h2);
  }
  return new_raw;
}

template <typename K, typename V, best::abridged P>
BEST_INLINE_ALWAYS best::option<size_t> table<K, V, P>::search(
  hash_value hash, const auto& query) const {
  if (raw_.ptr() == nullptr) { return best::none; }

  return best::table_internal::probe(  //
    *this, raw_.ptr(), hash.h_, raw_.cap().hard,
    [&](size_t base, group g) -> best::option<best::option<size_t>> {
      auto match = g.match(hash.h_.h2);
      while (auto next = match.next()) {
        size_t idx = (base + *next) & (raw_.cap().hard - 1);
        if (best::likely(identity_.equal(*raw_.key(idx), query))) {
          return {best::in_place, best::option(idx)};
        }
      }

      if (best::likely(!g.match_empty().empty())) {
        return {best::in_place, best::none};
      }

      return best::none;
    });
}

template <typename K, typename V, best::abridged P>
best::strbuf table<K, V, P>::debug(bool include_entries) const {
  best::strbuf out =
    best::format("type: {}\narray: {:p}, {}/{}/{}\n",
                 best::type_names::of<table>.path_with_params(), raw_.ptr(),
                 size_, raw_.cap().soft, raw_.cap().hard);

  if (raw_.ptr() == nullptr) { return out; }

  out.push("ctrl:");
  for (auto i : best::bounds{.count = raw_.cap().group_count()}) {
    i *= sizeof(group);
    best::format(out, "\n  {:p}: {:?}", raw_.ptr() + i,
                 group::load(raw_.ptr(), i));
  }
  out.push(" (mirrored)\n");

  if (!include_entries) { return out; }

  out.push("keys:\n");
  for (auto i : best::bounds{.count = raw_.cap().hard}) {
    auto key = raw_.key(i);
    if (raw_.ctrl(i).is_vacant()) {
      best::format(out, "  {:p}: ---\n", key);
      continue;
    }

    best::format(out, "  {:p}: {:?}\n", key, best::make_formattable(*key));
  }

  if constexpr (!best::is_empty<V>) {
    out.push("values:\n");
    for (auto i : best::bounds{.count = raw_.cap().hard}) {
      auto value = raw_.value(i);
      if (raw_.ctrl(i).is_vacant()) {
        best::format(out, "  {:p}: ---\n", value);
        continue;
      }

      best::format(out, "  {:p}: {:?}\n", value,
                   best::make_formattable(*value));
    }
  }

  return out;
}
}  // namespace best

#endif  // BEST_CONTAINER_TABLE_H_