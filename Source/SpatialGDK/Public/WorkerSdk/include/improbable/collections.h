// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_COLLECTIONS_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_COLLECTIONS_H
#include <cstddef>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace worker {
namespace detail {
template <typename T, typename... Args>
struct Contains;
}  // ::detail

/**
 * Stores a contiguous array of values. In particular, this is used for "repeated" fields in
 * generated schema code. This class implements a subset of the std::vector interface, and all
 * operations have the corresponding semantics. However, unlike std::vector<bool>, List<bool>
 * is consistent with other instantiations, and has the same interface.
 */
template <typename T>
class List {
public:
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  List() = default;
  List(const List&) = default;
  List(List&&) = default;
  List& operator=(const List&) = default;
  List& operator=(List&&) = default;
  ~List() = default;

  /** Constructs from the given elements. */
  List(std::initializer_list<T> list);
  /** Constructs from the given range. */
  template <typename It>
  List(It first, It last);
  /** Assigns the given elements. */
  List& operator=(const std::initializer_list<T> list);

  /** Checks for equality. */
  bool operator==(const List& other) const;
  /** Checks for inequality. */
  bool operator!=(const List& other) const;

  /** Removes all elements. */
  void clear();
  /** Constructs an element in-place before pos. */
  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args);
  /** Constructs an element in-place at the back. */
  template <typename... Args>
  void emplace_back(Args&&... args);
  /** Inserts from the given range before the given position. */
  template <typename It>
  iterator insert(const_iterator pos, It first, It last);
  /** Inserts the given elements before the given position. */
  iterator insert(const_iterator pos, std::initializer_list<T> list);
  /** Inserts count copies of the given element. */
  iterator insert(const_iterator pos, size_type count, const T& value);
  /** Erases the element at the given position. */
  iterator erase(const_iterator pos);
  /** Erases the elements in the given range. */
  iterator erase(const_iterator first, const_iterator last);
  /** Erases the last element. */
  void pop_back();
  /** Swaps the contents with that of other. */
  void swap(List& other);

  /** True iff size() == 0. */
  bool empty() const;
  /** Number of elements. */
  size_type size() const;
  /** Access to underlying array. */
  pointer data();
  /** Access to underlying array. */
  const_pointer data() const;
  /** Iterator to the first element. */
  iterator begin();
  /** Iterator to the first element. */
  const_iterator begin() const;
  /** Iterator to one past the last element. */
  iterator end();
  /** Iterator to one past the last element. */
  const_iterator end() const;
  /** Reverse iterator to the first element. */
  reverse_iterator rbegin();
  /** Reverse iterator to the first element. */
  const_reverse_iterator rbegin() const;
  /** Reverse iterator to one past the last element. */
  reverse_iterator rend();
  /** Reverse iterator to one past the last element. */
  const_reverse_iterator rend() const;
  /** Access the first element. */
  reference front();
  /** Access the first element. */
  const_reference front() const;
  /** Access the last element. */
  reference back();
  /** Access the last element. */
  const_reference back() const;
  /** Access an arbitrary element. */
  reference operator[](size_type pos);
  /** Access an arbitrary element. */
  const_reference operator[](size_type pos) const;

private:
  static constexpr bool is_bool = std::is_same<value_type, bool>::value;
  using proxy_type = typename std::conditional<is_bool, unsigned char, value_type>::type;
  std::vector<proxy_type> impl;
};

/**
 * Represents an optional value. In particular, this is used for "option" fields in generated schema
 * code. Mostly compatible with the std::optional type introduced in C++17.
 */
template <typename T>
class Option {
public:
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  Option() = default;
  Option(const Option&);
  Option(Option&&) = default;
  Option& operator=(const Option&);
  Option& operator=(Option&&) = default;
  ~Option() = default;

  /** Constructs from the given element. */
  Option(const T& value);
  /** Constructs from the given element. */
  Option(T&& value);

  /** Checks for equality. */
  template <typename U>
  bool operator==(const Option<U>& other) const;
  /** Checks for inequality. */
  template <typename U>
  bool operator!=(const Option<U>& other) const;

  /** Removes the element. */
  void clear();
  /** Constructs the contained element in-place. */
  template <typename... Args>
  void emplace(Args&&... args);
  /** Swaps the contents with that of other. */
  void swap(Option& other);

  /** True iff !empty(). */
  explicit operator bool() const;
  /** True iff size() == 0. */
  bool empty() const;
  /** Number of elements (either 0 or 1). */
  size_type size() const;
  /** Iterator to the first element. */
  iterator begin();
  /** Iterator to the first element. */
  const_iterator begin() const;
  /** Iterator to one past the last element. */
  iterator end();
  /** Iterator to one past the last element. */
  const_iterator end() const;
  /** Access the underlying pointer, or nullptr if empty. */
  pointer operator->();
  /** Access the underlying pointer, or nullptr if empty. */
  const_pointer operator->() const;
  /** Access the underlying data, or nullptr if empty. */
  pointer data();
  /** Access the underlying data, or nullptr if empty. */
  const_pointer data() const;
  /** Access the element. */
  reference operator*() &;
  /** Access the element. */
  const_reference operator*() const&;
  /** Move-from the element. */
  value_type&& operator*() &&;
  /** Move-from the element. */
  const value_type&& operator*() const&&;
  /** The contained value, or default_value if empty. */
  template <typename U>
  value_type value_or(U&& default_value) const&;
  /** The contained value, or default_value if empty. */
  template <typename U>
  value_type value_or(U&& default_value) &&;

private:
  std::unique_ptr<value_type> impl;
};

/** Specialization allowing an optional reference. */
template <typename T>
class Option<T&> {
public:
  using value_type = T;
  using reference = value_type&;
  using const_reference = reference;
  using pointer = value_type*;
  using const_pointer = pointer;
  using iterator = pointer;
  using const_iterator = iterator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  Option() = default;
  Option(const Option&) = default;
  Option(Option&&) = default;
  Option& operator=(const Option&) = default;
  Option& operator=(Option&&) = default;
  ~Option() = default;

  /** Constructs from the given element. */
  Option(T& ref);
  /** Avoid binding to temporaries. */
  Option(T&& ref) = delete;

  /** Checks for equality. */
  template <typename U>
  bool operator==(const Option<U>& other) const;
  /** Checks for inequality. */
  template <typename U>
  bool operator!=(const Option<U>& other) const;

  /** Removes the element. */
  void clear();
  /** Constructs the contained element in-place. */
  void emplace(T& ref);
  /** Swaps the contents with that of other. */
  void swap(Option& other);

  /** True iff !empty(). */
  explicit operator bool() const;
  /** True iff size() == 0. */
  bool empty() const;
  /** Number of elements (either 0 or 1). */
  size_type size() const;
  /** Iterator to the first element. */
  iterator begin() const;
  /** Iterator to one past the last element. */
  iterator end() const;
  /** Access the underlying pointer, or nullptr if empty. */
  pointer operator->() const;
  /** Access the underlying data, or nullptr if empty. */
  pointer data() const;
  /** Access the element. */
  reference operator*() const;
  /** The contained value, or default_value if empty. */
  template <typename U>
  value_type value_or(U&& default_value) const;

private:
  Option<std::reference_wrapper<T>> impl;
};

/** Disallow Option of rvalue reference. */
template <typename T>
class Option<T&&> {
  static_assert(!std::is_same<T, T>::value, "Option of rvalue reference is not allowed");
};

/**
 * Represents an unordered map. In particular, this is used for "map" fields in generated schema
 * code. This class implements a subset of the std::unordered_map interface, and all operations
 * have the corresponding semantics. The main differences are:
 * - iteration order is well-defined and matches *insertion* order;
 * - the value type V may be incomplete at the point of instantiation.
 */
template <typename K, typename V>
class Map {
public:
  using key_type = K;
  using mapped_type = V;
  using value_type = std::pair<const key_type, mapped_type>;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = typename std::list<value_type>::iterator;
  using const_iterator = typename std::list<value_type>::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  Map() = default;
  Map(const Map&);
  Map(Map&&);
  Map& operator=(const Map&);
  Map& operator=(Map&&);
  ~Map() = default;

  /** Constructs from the given elements. */
  Map(std::initializer_list<value_type> list);
  /** Constructs from the given range. */
  template <typename It>
  Map(It first, It last);
  /** Assigns the given elements. */
  Map& operator=(const std::initializer_list<value_type> list);

  /** Checks for equality. */
  bool operator==(const Map& other) const;
  /** Checks for inequality. */
  bool operator!=(const Map& other) const;

  /** Removes all elements. */
  void clear();
  /** Constructs an element in-place (if the key does not exist). */
  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args);
  /** Inserts from the given range. */
  template <typename It>
  void insert(It first, It last);
  /** Inserts the given elements. */
  void insert(std::initializer_list<value_type> list);
  /** Erases the element at the given position. */
  iterator erase(const_iterator pos);
  /** Erases the elements in the given range. */
  iterator erase(const_iterator first, const_iterator last);
  /** Erases the element with the given key. */
  bool erase(const key_type& key);
  /** Swaps the contents with that of other. */
  void swap(Map& other);

  /** True iff size() == 0. */
  bool empty() const;
  /** Number of elements. */
  size_type size() const;
  /** Whether the key exists. */
  bool count(const key_type& key) const;
  /** Find the given key. */
  iterator find(const key_type& key);
  /** Find the given key. */
  const_iterator find(const key_type& key) const;
  /** Iterator to the first element. */
  iterator begin();
  /** Iterator to the first element. */
  const_iterator begin() const;
  /** Iterator to one past the last element. */
  iterator end();
  /** Iterator to one past the last element. */
  const_iterator end() const;
  /** Reverse iterator to the first element. */
  reverse_iterator rbegin();
  /** Reverse iterator to the first element. */
  const_reverse_iterator rbegin() const;
  /** Reverse iterator to one past the last element. */
  reverse_iterator rend();
  /** Reverse iterator to one past the last element. */
  const_reverse_iterator rend() const;
  /** Access an element by key. */
  mapped_type& operator[](const key_type& key);
  /** Access an element by key. */
  mapped_type& operator[](key_type&& key);

private:
  std::list<value_type> values;
  std::unordered_map<key_type, iterator> lut;
};

/**
 * Algebraic data type. Stores exactly one value, whose type might be any of the variadic template
 * arguments. This is a fairly minimal implementation.
 */
template <typename... Args>
class Variant {
public:
  Variant(const Variant&);
  Variant(Variant&&) = default;
  Variant& operator=(const Variant&);
  Variant& operator=(Variant&&) = default;
  ~Variant() = default;

  /** Forwarding constructor for any contained type. */
  template <typename T, typename = typename std::enable_if<
                            detail::Contains<typename std::decay<T>::type, Args...>::value>::type>
  Variant(T&&);

  /** Forwarding assignment operator for any contained type. */
  template <typename T, typename = typename std::enable_if<
                            detail::Contains<typename std::decay<T>::type, Args...>::value>::type>
  Variant& operator=(T&&);

  /** Swaps the contents with that of other. */
  void swap(Variant& other);

  /** Checks for equality. */
  bool operator==(const Variant& other) const;
  /** Checks for inequality. */
  bool operator!=(const Variant& other) const;

  /**
   * Accessor for the value of any contained type. Returns nullptr if the variant does not contain
   * the given type.
   */
  template <typename T,
            typename = typename std::enable_if<detail::Contains<T, Args...>::value>::type>
  T* data();
  /**
   * Accessor for the value of any contained type. Returns nullptr if the variant does not contain
   * the given type.
   */
  template <typename T,
            typename = typename std::enable_if<detail::Contains<T, Args...>::value>::type>
  const T* data() const;

private:
  struct StorageBase {
    virtual ~StorageBase() = default;
    virtual std::unique_ptr<StorageBase> copy() const = 0;
  };
  template <typename T>
  struct Storage : StorageBase {
    Storage(const T&);
    Storage(T&&);
    ~Storage() override = default;
    std::unique_ptr<StorageBase> copy() const override;
    T value;
  };
  std::size_t type_index;  // Index of type within variadic parameter list.
  std::unique_ptr<StorageBase> storage;
};

/** Free-standing swap function for List. */
template <typename T>
void swap(List<T>& lhs, List<T>& rhs);

/** Free-standing swap function for Option. */
template <typename T>
void swap(Option<T>& lhs, Option<T>& rhs);

/** Free-standing swap function for Map. */
template <typename K, typename V>
void swap(Map<K, V>& lhs, Map<K, V>& rhs);

/** Free-standing swap function for Variant. */
template <typename... Args>
void swap(Variant<Args...>& lhs, Variant<Args...>& rhs);

}  // ::worker

namespace std {
template <typename T>
struct hash<worker::Option<T>> {
  std::size_t operator()(const worker::Option<T>& value) const;
};
template <typename T>
struct hash<worker::List<T>> {
  std::size_t operator()(const worker::List<T>& value) const;
};
template <typename K, typename V>
struct hash<worker::Map<K, V>> {
  std::size_t operator()(const worker::Map<K, V>& value) const;
};
}  // ::std

#include <improbable/detail/collections.i.h>
#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_COLLECTIONS_H
