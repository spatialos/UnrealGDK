// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_COLLECTIONS_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_COLLECTIONS_I_H
// Not strictly necessary, since this file shouldn't be included on its own, but may as well make it
// standalone for the benefit of automated tooling.
#include <improbable/collections.h>

namespace worker {
namespace detail {

template <typename, std::size_t, typename...>
struct TypeIndexImpl;
template <typename T, std::size_t N, typename... Args>
struct TypeIndexImpl<T, N, T, Args...> {
  static constexpr const std::size_t value = N;
};
template <typename T, std::size_t N, typename A, typename... Args>
struct TypeIndexImpl<T, N, A, Args...> {
  static constexpr const std::size_t value = TypeIndexImpl<T, 1 + N, Args...>::value;
};
template <typename T, typename... Args>
using TypeIndex = TypeIndexImpl<T, 0, Args...>;

template <typename T>
struct Contains<T> {
  static constexpr const bool value = false;
};
template <typename T, typename... Args>
struct Contains<T, T, Args...> {
  static constexpr const bool value = true;
};
template <typename T, typename A, typename... Args>
struct Contains<T, A, Args...> {
  static constexpr const bool value = Contains<T, Args...>::value;
};

template <typename... Args>
struct VariantEq;
template <>
struct VariantEq<> {
  template <typename... Brgs>
  static bool eq(const Variant<Brgs...>&, const Variant<Brgs...>&) {
    return false;
  }
};
template <typename A, typename... Args>
struct VariantEq<A, Args...> {
  template <typename... Brgs>
  static bool eq(const Variant<Brgs...>& a, const Variant<Brgs...>& b) {
    auto a_data = a.template data<A>();
    auto b_data = b.template data<A>();
    return (a_data && b_data && *a_data == *b_data) || VariantEq<Args...>::eq(a, b);
  }
};

template <typename T>
struct ListIter {
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  static T* To(std::vector<T>& v, iterator it) {
    return (it - v.begin()) + v.data();
  }
  static const T* To(const std::vector<T>& v, const_iterator it) {
    return (it - v.begin()) + v.data();
  }
  static iterator From(std::vector<T>& v, T* pointer) {
    return v.begin() + (pointer - v.data());
  }
  static const_iterator From(const std::vector<T>& v, const T* pointer) {
    return v.begin() + (pointer - v.data());
  }
};

template <>
struct ListIter<bool> {
  using iterator = std::vector<unsigned char>::iterator;
  using const_iterator = std::vector<unsigned char>::const_iterator;

  static bool* To(std::vector<unsigned char>& v, iterator it) {
    return reinterpret_cast<bool*>((it - v.begin()) + v.data());
  }
  static const bool* To(const std::vector<unsigned char>& v, const_iterator it) {
    return reinterpret_cast<const bool*>((it - v.begin()) + v.data());
  }
  static iterator From(std::vector<unsigned char>& v, bool* pointer) {
    return v.begin() + (reinterpret_cast<unsigned char*>(pointer) - v.data());
  }
  static const_iterator From(const std::vector<unsigned char>& v, const bool* pointer) {
    return v.begin() + (reinterpret_cast<const unsigned char*>(pointer) - v.data());
  }
};

}  // namespace detail

template <typename T>
List<T>::List(std::initializer_list<T> list) : impl{list.begin(), list.end()} {}

template <typename T>
template <typename It>
List<T>::List(It first, It last) {
  insert(begin(), first, last);
}

template <typename T>
auto List<T>::operator=(const std::initializer_list<T> list) -> List& {
  impl = {list.begin(), list.end()};
  return *this;
}

template <typename T>
bool List<T>::operator==(const List& other) const {
  return impl == other.impl;
}

template <typename T>
bool List<T>::operator!=(const List& other) const {
  return impl != other.impl;
}

template <typename T>
void List<T>::clear() {
  impl.clear();
}

template <typename T>
template <typename... Args>
auto List<T>::emplace(const_iterator pos, Args&&... args) -> iterator {
  return detail::ListIter<T>::To(
      impl, impl.emplace(detail::ListIter<T>::From(impl, pos), std::forward<Args>(args)...));
}

template <typename T>
template <typename... Args>
void List<T>::emplace_back(Args&&... args) {
  impl.emplace_back(std::forward<Args>(args)...);
}

template <typename T>
template <typename It>
auto List<T>::insert(const_iterator pos, It first, It last) -> iterator {
  return detail::ListIter<T>::To(impl,
                                 impl.insert(detail::ListIter<T>::From(impl, pos), first, last));
}

template <typename T>
auto List<T>::insert(const_iterator pos, std::initializer_list<T> list) -> iterator {
  return detail::ListIter<T>::To(
      impl, impl.insert(detail::ListIter<T>::From(impl, pos), list.begin(), list.end()));
}

template <typename T>
auto List<T>::insert(const_iterator pos, size_type count, const T& value) -> iterator {
  return detail::ListIter<T>::To(impl,
                                 impl.insert(detail::ListIter<T>::From(impl, pos), count, value));
}

template <typename T>
auto List<T>::erase(const_iterator pos) -> iterator {
  return detail::ListIter<T>::To(impl, impl.erase(detail::ListIter<T>::From(impl, pos)));
}

template <typename T>
auto List<T>::erase(const_iterator first, const_iterator last) -> iterator {
  return detail::ListIter<T>::To(impl, impl.erase(detail::ListIter<T>::From(impl, first),
                                                  detail::ListIter<T>::From(impl, last)));
}

template <typename T>
void List<T>::pop_back() {
  return impl.pop_back();
}

template <typename T>
void List<T>::swap(List& other) {
  impl.swap(other.impl);
}

template <typename T>
bool List<T>::empty() const {
  return impl.empty();
}

template <typename T>
auto List<T>::size() const -> size_type {
  return impl.size();
}

template <typename T>
auto List<T>::data() -> pointer {
  return begin();
}

template <typename T>
auto List<T>::data() const -> const_pointer {
  return begin();
}

template <typename T>
auto List<T>::begin() -> iterator {
  return detail::ListIter<T>::To(impl, impl.begin());
}

template <typename T>
auto List<T>::begin() const -> const_iterator {
  return detail::ListIter<T>::To(impl, impl.begin());
}

template <typename T>
auto List<T>::end() -> iterator {
  return detail::ListIter<T>::To(impl, impl.end());
}

template <typename T>
auto List<T>::end() const -> const_iterator {
  return detail::ListIter<T>::To(impl, impl.end());
}

template <typename T>
auto List<T>::rbegin() -> reverse_iterator {
  return reverse_iterator{end()};
}

template <typename T>
auto List<T>::rbegin() const -> const_reverse_iterator {
  return const_reverse_iterator{end()};
}

template <typename T>
auto List<T>::rend() -> reverse_iterator {
  return reverse_iterator{begin()};
}

template <typename T>
auto List<T>::rend() const -> const_reverse_iterator {
  return const_reverse_iterator{begin()};
}

template <typename T>
auto List<T>::front() -> reference {
  return *begin();
}

template <typename T>
auto List<T>::front() const -> const_reference {
  return *begin();
}

template <typename T>
auto List<T>::back() -> reference {
  return *rbegin();
}

template <typename T>
auto List<T>::back() const -> const_reference {
  return *rbegin();
}

template <typename T>
auto List<T>::operator[](size_type pos) -> reference {
  return begin()[pos];
}

template <typename T>
auto List<T>::operator[](size_type pos) const -> const_reference {
  return begin()[pos];
}

template <typename T>
Option<T>::Option(const Option& other) {
  *this = other;
}

template <typename T>
auto Option<T>::operator=(const Option& other) -> Option& {
  if (this != &other) {
    if (other) {
      impl.reset(new T{*other});
    } else {
      impl.reset();
    }
  }
  return *this;
}

template <typename T>
Option<T>::Option(const T& value) {
  impl.reset(new T{value});
}

template <typename T>
Option<T>::Option(T&& value) {
  impl.reset(new T{std::move(value)});
}

template <typename T>
template <typename U>
bool Option<T>::operator==(const Option<U>& other) const {
  return (empty() && other.empty()) || (!empty() && !other.empty() && *impl == *other);
}

template <typename T>
template <typename U>
bool Option<T>::operator!=(const Option<U>& other) const {
  return !operator==(other);
}

template <typename T>
void Option<T>::clear() {
  impl.reset();
}

template <typename T>
template <typename... Args>
void Option<T>::emplace(Args&&... args) {
  impl.reset(new T(std::forward<Args>(args)...));
}

template <typename T>
void Option<T>::swap(Option& other) {
  impl.swap(other.impl);
}

template <typename T>
Option<T>::operator bool() const {
  return static_cast<bool>(impl);
}

template <typename T>
bool Option<T>::empty() const {
  return !operator bool();
}

template <typename T>
auto Option<T>::size() const -> size_type {
  return empty() ? 0 : 1;
}

template <typename T>
auto Option<T>::begin() -> iterator {
  return impl ? impl.get() : nullptr;
}

template <typename T>
auto Option<T>::begin() const -> const_iterator {
  return impl ? impl.get() : nullptr;
}

template <typename T>
auto Option<T>::end() -> iterator {
  return impl ? 1 + impl.get() : nullptr;
}

template <typename T>
auto Option<T>::end() const -> const_iterator {
  return impl ? 1 + impl.get() : nullptr;
}

template <typename T>
auto Option<T>::operator-> () -> pointer {
  return impl ? impl.get() : nullptr;
}

template <typename T>
auto Option<T>::operator-> () const -> const_pointer {
  return impl ? impl.get() : nullptr;
}

template <typename T>
auto Option<T>::data() -> pointer {
  return impl ? impl.get() : nullptr;
}

template <typename T>
auto Option<T>::data() const -> const_pointer {
  return impl ? impl.get() : nullptr;
}

template <typename T>
auto Option<T>::operator*() & -> reference {
  return *impl;
}

template <typename T>
auto Option<T>::operator*() const& -> const_reference {
  return *impl;
}

template <typename T>
auto Option<T>::operator*() && -> value_type&& {
  return std::move(*impl);
}

template <typename T>
auto Option<T>::operator*() const&& -> const value_type&& {
  return std::move(*impl);
}

template <typename T>
template <typename U>
auto Option<T>::value_or(U&& default_value) const& -> value_type {
  return impl ? *impl : static_cast<T>(std::forward<U>(default_value));
}

template <typename T>
template <typename U>
auto Option<T>::value_or(U&& default_value)&& -> value_type {
  return impl ? std::move(*impl) : static_cast<T>(std::forward<U>(default_value));
}

template <typename T>
Option<T&>::Option(T& ref) : impl{ref} {}

template <typename T>
template <typename U>
bool Option<T&>::operator==(const Option<U>& other) const {
  return (empty() && other.empty()) || (!empty() && !other.empty() && impl->get() == *other);
}

template <typename T>
template <typename U>
bool Option<T&>::operator!=(const Option<U>& other) const {
  return !operator==(other);
}

template <typename T>
void Option<T&>::clear() {
  impl.clear();
}

template <typename T>
void Option<T&>::emplace(T& ref) {
  impl.emplace(ref);
}

template <typename T>
void Option<T&>::swap(Option& other) {
  impl.swap(other.impl);
}

template <typename T>
Option<T&>::operator bool() const {
  return static_cast<bool>(impl);
}

template <typename T>
bool Option<T&>::empty() const {
  return impl.empty();
}

template <typename T>
auto Option<T&>::size() const -> size_type {
  return impl.size();
}

template <typename T>
auto Option<T&>::begin() const -> iterator {
  return impl ? &impl->get() : nullptr;
}

template <typename T>
auto Option<T&>::end() const -> iterator {
  return impl ? 1 + &impl->get() : nullptr;
}

template <typename T>
auto Option<T&>::operator-> () const -> pointer {
  return impl ? &impl->get() : nullptr;
}

template <typename T>
auto Option<T&>::data() const -> pointer {
  return impl ? &impl->get() : nullptr;
}

template <typename T>
auto Option<T&>::operator*() const -> reference {
  return *impl;
}

template <typename T>
template <typename U>
auto Option<T&>::value_or(U&& default_value) const -> value_type {
  return impl ? *impl : static_cast<T>(std::forward<U>(default_value));
}

template <typename K, typename V>
Map<K, V>::Map(const Map& other) {
  insert(other.begin(), other.end());
}

template <typename K, typename V>
Map<K, V>::Map(Map&& other) {
  // Preserves iterators.
  swap(other);
}

template <typename K, typename V>
auto Map<K, V>::operator=(const Map& other) -> Map& {
  if (this == &other) {
    return *this;
  }
  clear();
  insert(other.begin(), other.end());
  return *this;
}

template <typename K, typename V>
auto Map<K, V>::operator=(Map&& other) -> Map& {
  // Preserves iterators.
  swap(other);
  return *this;
}

template <typename K, typename V>
Map<K, V>::Map(std::initializer_list<value_type> list) {
  insert(list);
}

template <typename K, typename V>
template <typename It>
Map<K, V>::Map(It first, It last) {
  insert(first, last);
}

template <typename K, typename V>
auto Map<K, V>::operator=(const std::initializer_list<value_type> list) -> Map& {
  clear();
  insert(list);
  return *this;
}

template <typename K, typename V>
bool Map<K, V>::operator==(const Map& other) const {
  auto subset = [](const Map& a, const Map& b) {
    for (const auto& pair : a) {
      auto it = b.find(pair.first);
      if (it == b.end() || it->second != pair.second) {
        return false;
      }
    }
    return true;
  };
  return size() == other.size() && subset(*this, other);
}

template <typename K, typename V>
bool Map<K, V>::operator!=(const Map& other) const {
  return !operator==(other);
}

template <typename K, typename V>
void Map<K, V>::clear() {
  values.clear();
  lut.clear();
}

template <typename K, typename V>
template <typename... Args>
auto Map<K, V>::emplace(Args&&... args) -> std::pair<iterator, bool> {
  value_type pair{std::forward<Args>(args)...};
  auto it = lut.find(pair.first);
  if (it != lut.end()) {
    return {it->second, false};
  }
  auto key_copy = pair.first;
  values.emplace_back(std::move(pair));
  auto value_it = --values.end();
  lut.emplace(key_copy, value_it);
  return {value_it, true};
}

template <typename K, typename V>
template <typename It>
void Map<K, V>::insert(It first, It last) {
  for (auto it = first; it != last; ++it) {
    emplace(*it);
  }
}

template <typename K, typename V>
void Map<K, V>::insert(std::initializer_list<value_type> list) {
  insert(list.begin(), list.end());
}

template <typename K, typename V>
auto Map<K, V>::erase(const_iterator pos) -> iterator {
  lut.erase(pos->first);
  return values.erase(pos);
}

template <typename K, typename V>
auto Map<K, V>::erase(const_iterator first, const_iterator last) -> iterator {
  for (auto it = first; it != last; ++it) {
    lut.erase(it->first);
  }
  return values.erase(first, last);
}

template <typename K, typename V>
bool Map<K, V>::erase(const key_type& key) {
  auto it = lut.find(key);
  if (it == lut.end()) {
    return false;
  }
  values.erase(it->second);
  lut.erase(it);
  return true;
}

template <typename K, typename V>
void Map<K, V>::swap(Map& other) {
  values.swap(other.values);
  lut.swap(other.lut);
}

template <typename K, typename V>
bool Map<K, V>::empty() const {
  return values.empty();
}

template <typename K, typename V>
auto Map<K, V>::size() const -> size_type {
  return values.size();
}

template <typename K, typename V>
bool Map<K, V>::count(const key_type& key) const {
  return lut.count(key) != 0;
}

template <typename K, typename V>
auto Map<K, V>::find(const key_type& key) -> iterator {
  auto it = lut.find(key);
  return it == lut.end() ? end() : it->second;
}

template <typename K, typename V>
auto Map<K, V>::find(const key_type& key) const -> const_iterator {
  auto it = lut.find(key);
  return it == lut.end() ? end() : static_cast<const_iterator>(it->second);
}

template <typename K, typename V>
auto Map<K, V>::begin() -> iterator {
  return values.begin();
}

template <typename K, typename V>
auto Map<K, V>::begin() const -> const_iterator {
  return values.begin();
}

template <typename K, typename V>
auto Map<K, V>::end() -> iterator {
  return values.end();
}

template <typename K, typename V>
auto Map<K, V>::end() const -> const_iterator {
  return values.end();
}

template <typename K, typename V>
auto Map<K, V>::rbegin() -> reverse_iterator {
  return reverse_iterator{end()};
}

template <typename K, typename V>
auto Map<K, V>::rbegin() const -> const_reverse_iterator {
  return const_reverse_iterator{end()};
}

template <typename K, typename V>
auto Map<K, V>::rend() -> reverse_iterator {
  return reverse_iterator{begin()};
}

template <typename K, typename V>
auto Map<K, V>::rend() const -> const_reverse_iterator {
  return const_reverse_iterator{begin()};
}

template <typename K, typename V>
auto Map<K, V>::operator[](const key_type& key) -> mapped_type& {
  auto it = lut.find(key);
  return it == lut.end() ? emplace(key, mapped_type{}).first->second : it->second->second;
}

template <typename K, typename V>
auto Map<K, V>::operator[](key_type&& key) -> mapped_type& {
  auto it = lut.find(key);
  return it == lut.end() ? emplace(std::move(key), mapped_type{}).first->second
                         : it->second->second;
}

template <typename... Args>
Variant<Args...>::Variant(const Variant& other)
: type_index{other.type_index}, storage{other.storage->copy()} {}

template <typename... Args>
auto Variant<Args...>::operator=(const Variant& other) -> Variant& {
  if (this == &other) {
    return *this;
  }
  storage = other.storage->copy();
  type_index = other.type_index;
  return *this;
}

template <typename... Args>
template <typename T, typename>
Variant<Args...>::Variant(T&& new_value)
: type_index{detail::TypeIndex<typename std::decay<T>::type, Args...>::value}
, storage{new Storage<typename std::decay<T>::type>(std::forward<T>(new_value))} {}

template <typename... Args>
template <typename T, typename>
auto Variant<Args...>::operator=(T&& new_value) -> Variant& {
  if (type_index == detail::TypeIndex<typename std::decay<T>::type, Args...>::value) {
    static_cast<Storage<typename std::decay<T>::type>*>(storage.get())->value =
        std::forward<T>(new_value);
  } else {
    std::unique_ptr<StorageBase> new_storage{
        new Storage<typename std::decay<T>::type>(std::forward<T>(new_value))};
    type_index = detail::TypeIndex<typename std::decay<T>::type, Args...>::value;
    storage.swap(new_storage);
  }
  return *this;
}

template <typename... Args>
void Variant<Args...>::swap(Variant& other) {
  storage.swap(other.storage);
  std::swap(type_index, other.type_index);
}

template <typename... Args>
bool Variant<Args...>::operator==(const Variant& other) const {
  return detail::VariantEq<Args...>::eq(*this, other);
}

template <typename... Args>
bool Variant<Args...>::operator!=(const Variant& other) const {
  return !operator==(other);
}

template <typename... Args>
template <typename T, typename>
T* Variant<Args...>::data() {
  return type_index == detail::TypeIndex<T, Args...>::value
      ? &static_cast<Storage<T>*>(storage.get())->value
      : nullptr;
}

template <typename... Args>
template <typename T, typename>
const T* Variant<Args...>::data() const {
  return type_index == detail::TypeIndex<T, Args...>::value
      ? &static_cast<Storage<T>*>(storage.get())->value
      : nullptr;
}

template <typename... Args>
template <typename T>
Variant<Args...>::Storage<T>::Storage(const T& new_value) : value(new_value) {}

template <typename... Args>
template <typename T>
Variant<Args...>::Storage<T>::Storage(T&& new_value) : value(new_value) {}

template <typename... Args>
template <typename T>
auto Variant<Args...>::Storage<T>::copy() const -> std::unique_ptr<StorageBase> {
  return std::unique_ptr<StorageBase>{new Storage<T>{value}};
}

template <typename T>
void swap(List<T>& lhs, List<T>& rhs) {
  lhs.swap(rhs);
}

template <typename T>
void swap(Option<T>& lhs, Option<T>& rhs) {
  lhs.swap(rhs);
}

template <typename K, typename V>
void swap(Map<K, V>& lhs, Map<K, V>& rhs) {
  lhs.swap(rhs);
}

template <typename... Args>
void swap(Variant<Args...>& lhs, Variant<Args...>& rhs) {
  lhs.swap(rhs);
}

}  // ::worker

template <typename T>
std::size_t std::hash<worker::Option<T>>::operator()(const worker::Option<T>& value) const {
  return value.empty() ? 977 : 1327 * (std::hash<T>{}(*value) + 977);
}

template <typename T>
std::size_t std::hash<worker::List<T>>::operator()(const worker::List<T>& value) const {
  std::size_t result = 1327;
  for (const auto& item : value) {
    result = (result * 977) + std::hash<T>{}(item);
  }
  return result;
}

template <typename K, typename V>
std::size_t std::hash<worker::Map<K, V>>::operator()(const worker::Map<K, V>& value) const {
  std::size_t result = 0;
  // The hash, to match the equality operator, does not encode order.
  for (const auto& pair : value) {
    result += 1327 * (std::hash<K>{}(pair.first) + 977 * std::hash<V>{}(pair.second));
  }
  return result;
}

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_COLLECTIONS_I_H
