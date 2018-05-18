// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_CALLBACKS_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_CALLBACKS_I_H
#include <improbable/detail/worker_detail.h>

namespace worker {
namespace detail {

// Registers that a call is entered and exited via constructor and destructor, respectively. The
// state of the given callback map is updated accordingly.
template <typename T>
class UpdateGuard {
public:
  UpdateGuard(CallbackMap<T>& callback_map) : callback_map{callback_map} {
    ++callback_map.call_depth;
  }

  ~UpdateGuard() {
    if (--callback_map.call_depth == 0) {
      callback_map.UpdateCallbacks();
    }
  }

  // Noncopyable.
  UpdateGuard(const UpdateGuard&) = delete;
  UpdateGuard& operator=(const UpdateGuard&) = delete;

private:
  CallbackMap<T>& callback_map;
};

template <typename T>
CallbackMap<T>::CallbackMap() : call_depth{0} {}

template <typename T>
void CallbackMap<T>::Add(CallbackKey key, const Callback<T>& callback) {
  UpdateGuard<T> update_guard{*this};
  to_add[key] = callback;
  to_remove.erase(key);
}

template <typename T>
bool CallbackMap<T>::Remove(CallbackKey key) {
  UpdateGuard<T> update_guard{*this};
  bool is_in_map = map.find(key) != map.end();
  bool had_pending_add = to_add.erase(key);

  if (is_in_map) {
    bool created_pending_remove = to_remove.emplace(key, /* ignored */ 0).second;
    return created_pending_remove;
  } else {
    return had_pending_add;
  }
}

template <typename T>
void CallbackMap<T>::InvokeAll(const T& op) const {
  UpdateGuard<T> update_guard{*const_cast<CallbackMap<T>*>(this)};
  for (const auto& pair : map) {
    pair.second(op);
  }
}

template <typename T>
void CallbackMap<T>::ReverseInvokeAll(const T& op) const {
  UpdateGuard<T> update_guard{*const_cast<CallbackMap<T>*>(this)};
  for (auto it = map.rbegin(), end = map.rend(); it != end; ++it) {
    it->second(op);
  }
}

template <typename T>
void CallbackMap<T>::UpdateCallbacks() {
  map.insert(to_add.begin(), to_add.end());
  to_add.clear();
  for (const auto& pair : to_remove) {
    map.erase(pair.first);
  }
  to_remove.clear();
}

template <typename T>
void ComponentCallbackMap<T>::Add(ComponentId component_id, CallbackKey key,
                                  const Callback<T>& callback) {
  map[component_id].Add(key, callback);
}

template <typename T>
bool ComponentCallbackMap<T>::Remove(CallbackKey key) {
  for (auto& pair : map) {
    if (pair.second.Remove(key)) {
      return true;
    }
  }
  return false;
}

template <typename T>
void ComponentCallbackMap<T>::InvokeAll(ComponentId component_id, const T& op) const {
  auto it = map.find(component_id);
  if (it != map.end()) {
    it->second.InvokeAll(op);
  }
}

template <typename T>
void ComponentCallbackMap<T>::ReverseInvokeAll(ComponentId component_id, const T& op) const {
  auto it = map.find(component_id);
  if (it != map.end()) {
    it->second.ReverseInvokeAll(op);
  }
}

}  // ::detail
}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_WORKER_I_H
