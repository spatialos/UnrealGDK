#ifndef GDK_SORTED_ARRAY_H
#define GDK_SORTED_ARRAY_H
#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

namespace gdk {

// Exemplar of the SortedArrayComparator concept
struct ExemplarSortedArrayComparator {
  // The value type, which must be the same as that of the the SortedArray container using it.
  struct ValueType;
  // The identifier type to which the value type can be converted.
  struct IdentifierType;

  // Function that takes a value type and returns a corresponding instance of the identifier type.
  // Arguments do not need to be passed by const& but must not be modified.
  static IdentifierType ToIdentifier(const ValueType& value);
  // A function that returns true when lhs should be ordered before rhs.
  // Arguments do not need to be passed by const& but must not be modified.
  static bool IsBefore(const IdentifierType& lhs, const IdentifierType& rhs);
  // Returns true if lhs is not ordered before rhs and rhs is not ordered before lhs.
  // Arguments do not need to be passed by const& but must not be modified.
  static bool IsEquivalent(const IdentifierType& lhs, const IdentifierType& rhs);
};

/**
 * A comparator for value types that can be converted to some comparable identifier type.
 * Value must be explicitly convertible to Identifier.
 * Identifier must be equality and less-than comparable.
 */
template <typename Value, typename Identifier = Value>
struct ConvertingSortedArrayComparator {
  using ValueType = Value;
  using IdentifierType = Identifier;

  static constexpr IdentifierType ToIdentifier(const ValueType& value) {
    return static_cast<Identifier>(value);
  }

  static constexpr bool IsBefore(const IdentifierType& lhs, const IdentifierType& rhs) {
    return lhs < rhs;
  }

  static constexpr bool IsEquivalent(const IdentifierType& lhs, const IdentifierType& rhs) {
    return lhs == rhs;
  }
};

/**
 * A container which stores elements in contiguous memory in sorted order.
 * Ordering is defined by Comparator.
 * Comparator must match the SortedArrayComparator concept.
 * Comparator::ValueType must be the same as Value.
 */
template <typename Value, typename Comparator = ConvertingSortedArrayComparator<Value>>
class SortedArray {
  static_assert(std::is_same<Value, typename Comparator::ValueType>::value,
                "Container value type must be the same as the comparator value type.");

public:
  using identifier_type = typename Comparator::IdentifierType;
  using value_type = typename std::vector<Value>::value_type;
  using reference = typename std::vector<Value>::reference;
  using const_reference = typename std::vector<Value>::const_reference;
  using iterator = typename std::vector<Value>::iterator;
  using const_iterator = typename std::vector<Value>::const_iterator;
  using difference_type = typename std::vector<Value>::difference_type;
  using size_type = typename std::vector<Value>::size_type;
  using reverse_iterator = typename std::vector<Value>::reverse_iterator;
  using const_reverse_iterator = typename std::vector<Value>::const_reverse_iterator;

  SortedArray() = default;

  // Creates a new SortedArray with a buffer large enough to hold initialCapacity elements.
  explicit SortedArray(size_t initialCapacity) : vec() {
    vec.reserve(initialCapacity);
  }

  iterator begin() {
    return vec.begin();
  }

  const_iterator begin() const {
    return vec.begin();
  }

  const_iterator cbegin() const {
    return vec.cbegin();
  }

  iterator rbegin() {
    return vec.rbegin();
  }

  const_iterator rbegin() const {
    return vec.rbegin();
  }

  const_iterator crbegin() const {
    return vec.crbegin();
  }

  iterator end() {
    return vec.end();
  }

  const_iterator end() const {
    return vec.end();
  }

  const_iterator cend() const {
    return vec.cend();
  }

  iterator rend() {
    return vec.rend();
  }

  const_iterator rend() const {
    return vec.rend();
  }

  const_iterator crend() const {
    return vec.crend();
  }

  friend bool operator==(const SortedArray<Value, Comparator>& lhs,
                         const SortedArray<Value, Comparator>& rhs) {
    return lhs.vec == rhs.vec;
  }

  friend bool operator!=(const SortedArray<Value, Comparator>& lhs,
                         const SortedArray<Value, Comparator>& rhs) {
    return lhs.vec != rhs.vec;
  }

  void swap(SortedArray<Value, Comparator>& other) noexcept {
    vec.swap(other.vec);
  }

  friend void swap(SortedArray<Value, Comparator>& lhs,
                   SortedArray<Value, Comparator>& rhs) noexcept {
    lhs.swap(rhs);
  }

  size_type size() const {
    return vec.size();
  }

  size_type max_size() const {
    return vec.max_size();
  }

  bool empty() const {
    return vec.empty();
  }

  reference operator[](size_t index) {
    return vec[index];
  }

  const_reference operator[](size_t index) const {
    return vec[index];
  }

  void erase(const_iterator pos) {
    vec.erase(pos);
  }

  void erase(const_iterator first, const_iterator last) {
    vec.erase(first, last);
  }

  /**
   *  Removes the all elements with equivalent identifier to id.
   *  Returns the number of elements removed.
   */
  size_type erase_all(const identifier_type& id) {
    auto lb = lower_bound(id);
    if (lb == vec.end() || !IsEqual(*lb, id)) {
      return 0;
    }
    auto ub = std::upper_bound(lb, end(), id, &CompareUpper);
    size_type n = std::distance(lb, ub);
    erase(lb, ub);
    return n;
  }

  void clear() {
    vec.clear();
  }

  /**
   *  Removes the first elements with equivalent identifier to id.
   *  Returns true if such an element exists and false otherwise.
   */
  bool erase_first(const identifier_type& id) {
    auto lb = lower_bound(id);
    if (lb == vec.end() || !IsEqual(*lb, id)) {
      return false;
    }
    vec.erase(lb);
    return true;
  }

  /**
   *  Finds the first element with identifier not less than id.
   *  Returns an iterator to the element provided one exists.
   *  Returns end() if no such element exists.
   */
  iterator lower_bound(const identifier_type& id) {
    return std::lower_bound(vec.begin(), vec.end(), id, &CompareLower);
  }

  /**
   *  Finds the first element with identifier not less than id.
   *  Returns an iterator to the element provided one exists.
   *  Returns end() if no such element exists.
   */
  const_iterator lower_bound(const identifier_type& id) const {
    return std::lower_bound(vec.cbegin(), vec.cend(), id, &CompareLower);
  }

  /**
   *  Finds the first element with identifier greater than id.
   *  Returns an iterator to the element provided one exists.
   *  Returns end() if no such element exists.
   */
  iterator upper_bound(const identifier_type& id) {
    return std::upper_bound(vec.cbegin(), vec.cend(), id, &CompareUpper);
  }

  /**
   *  Finds the first element with identifier greater than id.
   *  Returns an iterator to the element provided one exists.
   *  Returns end() if no such element exists.
   */
  const_iterator upper_bound(const identifier_type& id) const {
    return std::upper_bound(vec.cbegin(), vec.cend(), id, &CompareUpper);
  }

  /** Insert a copy of value in before any other elements with equivalent identifier. */
  iterator insert(const value_type& value) {
    identifier_type id = Comparator::ToIdentifier(value);
    return vec.insert(lower_bound(id), value);
  }

  /** Move-insert value before any other elements with equivalent or greater identifier. */
  iterator insert(value_type&& value) {
    identifier_type id = Comparator::ToIdentifier(value);
    return vec.insert(lower_bound(id), std::move(value));
  }

  /**
   *  Construct an element in place before any existing element with equivalent id.
   *  Returns an iterator to the new element.
   *  The created element must have equivalent identifier to id.
   */
  template <typename... Args>
  iterator emplace(const identifier_type& id, Args&&... args) {
    auto it = lower_bound(id);
    return vec.emplace(it, std::forward<Args>(args)...);
  }

  /**
   *  Inserts value if there is no existing element with equivalent id.
   *  On success returns an iterator to the new element and true.
   *  On failure returns an iterator to an existing element with equivalent id and false.
   */
  std::pair<iterator, bool> try_insert(const value_type& value) {
    auto id = Comparator::ToIdentifier(value);
    auto it = lower_bound(id);
    if (it != end() && IsEqual(*it, id)) {
      return std::make_pair(it, false);
    }
    auto insertedIt = vec.insert(it, value);
    return std::make_pair(insertedIt, true);
  }

  /**
   *  Inserts value if there is no existing element with equivalent id.
   *  If such an element exists then value is not moved from.
   *  On success returns an iterator to the new element and true.
   *  On failure returns an iterator to an existing element with equivalent id and false.
   */
  std::pair<iterator, bool> try_insert(value_type&& value) {
    auto id = Comparator::ToIdentifier(value);
    auto it = lower_bound(id);
    if (it != end() && IsEqual(*it, id)) {
      return std::make_pair(it, false);
    }
    auto insertedIt = vec.insert(it, std::move(value));
    return std::make_pair(insertedIt, true);
  }

  /**
   *  Construct an element in place if there is no existing element with equivalent identifier.
   *  If such an element exists then the new element is not constructed.
   *  In this case rvalue args are not moved from.
   *  On success returns an iterator to the new element and true.
   *  On failure returns an iterator to an existing element with equivalent id and false.
   *  The created element must have equivalent identifier to id.
   */
  template <typename... Args>
  std::pair<iterator, bool> try_emplace(const identifier_type& id, Args&&... args) {
    auto it = lower_bound(id);
    if (it != end() && IsEqual(*it, id)) {
      return std::make_pair(it, false);
    }
    auto emplacedIt = vec.emplace(it, std::forward<Args>(args)...);
    return std::make_pair(emplacedIt, true);
  }

  /**
   *  Find an element with equivalent identifier to id.
   *  Returns an iterator to the element provided one exists.
   *  Returns end() if no such element exists.
   */
  iterator find(const identifier_type& id) {
    auto it = lower_bound(id);
    if (it != vec.end() && IsEqual(*it, id)) {
      return it;
    }
    return end();
  }

  /**
   *  Find an element with equivalent identifier to id.
   *  Returns an iterator to the element provided one exists.
   *  Returns end() if no such element exists.
   */
  const_iterator find(const identifier_type& id) const {
    auto it = lower_bound(id);
    if (it != vec.cend() && IsEqual(*it, id)) {
      return it;
    }
    return cend();
  }

  // todo consider if this is needed and what type it should return.
  std::vector<value_type>& GetUnderlying() {
    return vec;
  }

  const std::vector<value_type>& GetUnderlying() const {
    return vec;
  }

private:
  static bool CompareLower(const value_type& value, const identifier_type& identifier) {
    return Comparator::IsBefore(Comparator::ToIdentifier(value), identifier);
  }

  static bool CompareUpper(const identifier_type& identifier, const value_type& value) {
    return Comparator::IsBefore(identifier, Comparator::ToIdentifier(value));
  }

  static bool IsEqual(const value_type& value, const identifier_type& identifier) {
    return Comparator::IsEquivalent(Comparator::ToIdentifier(value), identifier);
  }

  std::vector<value_type> vec;
};

}  // namespace gdk
#endif  // GDK_SORTED_ARRAY_H
