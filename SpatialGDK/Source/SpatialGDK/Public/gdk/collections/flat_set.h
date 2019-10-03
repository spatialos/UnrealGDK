#ifndef GDK_FLAT_SET_H
#define GDK_FLAT_SET_H
#include <algorithm>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace gdk {

// todo could make allocator aware or allow the container type to be specified.
// todo work out which methods are needed and restore tests for them.
/**
 * A container which stores elements in contiguous memory in sorted order.
 * Supports heterogeneous lookup and modification where possible.
 * Ordering is defined by Compare.
 * The interface is similar to std::set.
 * Differences - 
 *  Complexity of operations.
 *  Capacity methods from std::vector.
 *  Mutable iterators.
 *  Lack of node methods.
 *  Not allocator aware.
 *  Lacking compare functions.
 * Mutating an element such that the order should change is undefined behaviour.
 */
template <typename Key, typename Compare = std::less<Key>>
class FlatSet {
public:
  using key_type = typename std::vector<Key>::value_type;
  using value_type = typename std::vector<Key>::value_type;
  using reference = typename std::vector<Key>::reference;
  using const_reference = typename std::vector<Key>::const_reference;
  using iterator = typename std::vector<Key>::iterator; // Unlike std::set this is mutable.
  using const_iterator = typename std::vector<Key>::const_iterator;
  using difference_type = typename std::vector<Key>::difference_type;
  using size_type = typename std::vector<Key>::size_type;
  using reverse_iterator = typename std::vector<Key>::reverse_iterator;
  using const_reverse_iterator = typename std::vector<Key>::const_reverse_iterator;

  FlatSet() = default;

  explicit FlatSet(const Compare& comp) : c(), comp(comp) {}

  explicit FlatSet(size_t initialCapacity) {
    c.reserve(initialCapacity);
  }

  FlatSet(std::initializer_list<value_type> init, const Compare& comp = Compare())
  : c(init), comp(comp) {
    std::sort(c.begin(), c.end(), comp);  
  }

  iterator begin() {
    return c.begin();
  }

  const_iterator begin() const {
    return c.begin();
  }

  const_iterator cbegin() const {
    return c.cbegin();
  }

  iterator rbegin() {
    return c.rbegin();
  }

  const_iterator rbegin() const {
    return c.rbegin();
  }

  const_iterator crbegin() const {
    return c.crbegin();
  }

  iterator end() {
    return c.end();
  }

  const_iterator end() const {
    return c.end();
  }

  const_iterator cend() const {
    return c.cend();
  }

  iterator rend() {
    return c.rend();
  }

  const_iterator rend() const {
    return c.rend();
  }

  const_iterator crend() const {
    return c.crend();
  }

  friend bool operator==(const FlatSet<Key, Compare>& lhs, const FlatSet<Key, Compare>& rhs) {
    return lhs.c == rhs.c;
  }

  friend bool operator!=(const FlatSet<Key, Compare>& lhs, const FlatSet<Key, Compare>& rhs) {
    return lhs.c != rhs.c;
  }

  void swap(FlatSet<Key, Compare>& other) noexcept {
    c.swap(other.c);
  }

  friend void swap(FlatSet<Key, Compare>& lhs, FlatSet<Key, Compare>& rhs) noexcept {
    lhs.swap(rhs);
  }

  void reserve(size_type capacity) {
    c.reserve(capacity);
  }

  size_type capacity() const {
    return c.capacity();
  }

  void shrink_to_fit() {
    c.shrink_to_fit();
  }

  size_type size() const {
    return c.size();
  }

  size_type max_size() const {
    return c.max_size();
  }

  bool empty() const {
    return c.empty();
  }

  void clear() {
    c.clear();
  }

  iterator erase(iterator pos) {
    return c.erase(pos);
  }

  iterator erase(const_iterator pos) {
    return c.erase(pos);
  }

  void erase(const_iterator first, const_iterator last) {
    c.erase(first, last);
  }

  void erase(iterator first, iterator last) {
    c.erase(first, last);
  }

  /**
   * Removes the equivalent element if it exists.
   * Returns 1 if such an element exists and 0 otherwise.
   */
  size_type erase(const Key& key) {
    auto lb = lower_bound(key);
    if (lb == end() || comp(key, *lb)) {
      return 0;
    }
    c.erase(lb);
    return 1;
  }

  /**
   * Removes the equivalent element if it exists.
   * Returns 1 if such an element exists and 0 otherwise.
   */
  template <typename T, typename Comp = Compare, typename = typename Comp::is_transparent>
  size_type erase(const T& id) {
    auto lb = lower_bound(id);
    if (lb == end() || comp(id, *lb)) {
      return 0;
    }
    c.erase(lb);
    return 1;
  }

  /**
   * Finds the first element with identifier not less than id.
   * Returns an iterator to the element provided one exists.
   * Returns end() if no such element exists.
   */
  template <typename T, typename Comp = Compare, typename = typename Comp::is_transparent>
  iterator lower_bound(const T& id) {
    return std::lower_bound(begin(), end(), id, comp);
  }

  /**
   * Finds the first element with identifier not less than id.
   * Returns an iterator to the element provided one exists.
   * Returns end() if no such element exists.
   */
  template <typename T, typename Comp = Compare, typename = typename Comp::is_transparent>
  const_iterator lower_bound(const T& id) const {
    return std::lower_bound(cbegin(), cend(), id, comp);
  }

  iterator lower_bound(const Key& key) {
    return std::lower_bound(begin(), end(), key, comp);
  }

  const_iterator lower_bound(const Key& key) const {
    return std::lower_bound(cbegin(), cend(), key, comp);
  }

  /** Returns an iterator to the first element greater than key or end() if none exists. */
  template <typename T, typename Comp = Compare, typename = typename Comp::is_transparent>
  iterator upper_bound(const T& id) {
    return std::upper_bound(begin(), end(), id, comp);
  }

  /** Returns an iterator to the first element greater than key or end() if none exists. */
  template <typename T, typename Comp = Compare, typename = typename Comp::is_transparent>
  const_iterator upper_bound(const T& id) const {
    return std::upper_bound(cbegin(), cend(), id, comp);
  }

  /** Returns an iterator to the first element greater than key or end() if none exists. */
  iterator upper_bound(const Key& key) {
    return std::upper_bound(begin(), end(), key, comp);
  }

  /** Returns an iterator to the first element greater than key or end() if none exists. */
  const_iterator upper_bound(const Key& key) const {
    return std::upper_bound(cbegin(), cend(), key, comp);
  }

  /**
   * Inserts value if there is no existing, equivalent element.
   * On success returns an iterator to the new element and true.
   * On failure returns an iterator to an equivalent element and false.
   */
  std::pair<iterator, bool> insert(const value_type& value) {
    auto lb = lower_bound(value);
    if (lb != end() && !comp(value, *lb)) {
      return std::make_pair(lb, false);
    }
    return std::make_pair(c.insert(lb, value), true);
  }

  /**
   * Inserts value if there is no existing, equivalent element.
   * On success returns an iterator to the new element and true.
   * On failure returns an iterator to an equivalent element and false.
   */
  std::pair<iterator, bool> insert(value_type&& value) {
    auto lb = lower_bound(value);
    if (lb != end() && !comp(value, *lb)) {
      return std::make_pair(lb, false);
    }
    return std::make_pair(c.insert(lb, std::move(value)), true);
  }

  iterator insert(const_iterator hint, const value_type& value) {
    bool hintGreaterThanValue = hint == end() || comp(value, *hint);
    bool predecessorLessThanValue = hint == begin() || comp(*(hint - 1), value);

    // If the hint is correct then insert value just before hint.
    if (hintGreaterThanValue && predecessorLessThanValue) {
      return c.insert(hint, value);
    }

    // If the hint was wrong then insert normally.
    return insert(value).first;
  }

  iterator insert(const_iterator hint, value_type&& value) {
    bool hintGreaterThanValue = hint == end() || comp(value, *hint);
    bool predecessorLessThanValue = hint == begin() || comp(*(hint - 1), value);

    // If the hint is correct then insert value just before hint.
    if (hintGreaterThanValue && predecessorLessThanValue) {
      return c.insert(hint, std::move(value));
    }

    // If the hint was wrong then insert normally.
    return insert(std::move(value)).first;
  }

  /**
   * Constructs an element from given arguments and inserts it if there is no equivalent element.
   * On success returns an iterator to the new element and true.
   * On failure returns an iterator to an existing, equivalent element and false.
   * The element will be constructed in both cases.
   */
  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    return insert(Key(std::forward<Args>(args)...));
  }

  /**
   * Constructs an element from given arguments and inserts it if there is no equivalent element.
   * On success returns an iterator to the new element and true.
   * On failure returns an iterator to an existing, equivalent element and false.
   * The element will be constructed in both cases.
   */
  template <typename... Args>
  std::pair<iterator, bool> emplace_hint(const_iterator hint, Args&&... args) {
    return insert(hint, Key(std::forward<Args>(args)...));
  }

  /** Returns an iterator to an equivalent element to key or end() if no such element exists. */
  iterator find(const Key& key) {
    auto it = lower_bound(key);
    if (it == end() || comp(key, *it)) {
      return end();
    }
    return it;
  }

  /** Returns an iterator to an equivalent element to key or end() if no such element exists. */
  const_iterator find(const Key& key) const {
    auto it = lower_bound(key);
    if (it == cend() || comp(key, *it)) {
      return cend();
    }
    return it;
  }

  /** Returns an iterator to an equivalent element to id or end() if no such element exists. */
  template <typename T, typename Comp = Compare, typename = typename Comp::is_transparent>
  iterator find(const T& id) {
    auto it = lower_bound(id);
    if (it == end() || comp(id, *it)) {
      return end();
    }
    return it;
  }

  /** Returns an iterator to an equivalent element to id or end() if no such element exists. */
  template <typename T, typename Comp = Compare, typename = typename Comp::is_transparent>
  const_iterator find(const T& id) const {
    auto it = lower_bound(id);
    if (it == cend() || comp(id, *it)) {
      return cend();
    }
    return it;
  }

  // todo consider if this is needed and what type it should return.
  std::vector<Key>& GetUnderlying() {
    return c;
  }

  const std::vector<Key>& GetUnderlying() const {
    return c;
  }

private:
  std::vector<Key> c;
  Compare comp;
};

}  // namespace gdk
#endif  // GDK_FLAT_SET_H
