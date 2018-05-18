// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_CLIENT_HANDLE_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_CLIENT_HANDLE_I_H
#include <memory>
#include <utility>

namespace worker {
namespace detail {

// A smart pointer used to implement the client vtable functions while avoiding unnecessary
// allocation. It allows a raw pointer to client-owned memory to be promoted to a shared_ptr on
// demand when the vtable Copy function is called. This shared_ptr is created by copy- or
// move-constructing from the source depending on whether ownership has been assumed or not.
class ClientHandleBase {
public:
  virtual ~ClientHandleBase(){};
};

template <typename T>
class ClientHandle : public ClientHandleBase {
public:
  ~ClientHandle() override = default;
  ClientHandle(ClientHandle&&) = default;
  ClientHandle(const ClientHandle& ref) = delete;
  ClientHandle& operator=(ClientHandle&&) = default;
  ClientHandle& operator=(const ClientHandle& ref) = delete;

  template <typename... Args>
  static ClientHandle* allocate(Args&&... args) {
    auto* result = new ClientHandle{nullptr, nullptr};
    result->shared = std::shared_ptr<T>{new T{std::forward<Args>(args)...}};
    return result;
  }

  static ClientHandle copyable(const T* raw) {
    return ClientHandle{raw, nullptr};
  }

  static ClientHandle movable(T* raw) {
    return ClientHandle{nullptr, raw};
  }

  static void free(ClientHandleBase* handle) {
    delete static_cast<ClientHandle*>(handle);
  }

  static ClientHandle* new_copy(ClientHandleBase* handle) {
    auto& ref = *static_cast<ClientHandle*>(handle);
    ref.PromoteToShared();
    auto* result = new ClientHandle{nullptr, nullptr};
    result->shared = ref.shared;
    return result;
  }

  static const T& get(const ClientHandleBase* handle) {
    return static_cast<const ClientHandle*>(handle)->get();
  }

  static T& get_mutable(ClientHandleBase* handle) {
    return static_cast<ClientHandle*>(handle)->get_mutable();
  }

  const T& get() const {
    return copyable_raw ? *copyable_raw : movable_raw ? *movable_raw : *shared;
  }

  T& get_mutable() {
    if (copyable_raw) {
      PromoteToShared();
    }
    return movable_raw ? *movable_raw : *shared;
  }

private:
  explicit ClientHandle(const T* copyable, T* movable)
  : copyable_raw{copyable}, movable_raw{movable} {}

  void PromoteToShared() {
    if (copyable_raw) {
      shared = std::make_shared<T>(*copyable_raw);
      copyable_raw = nullptr;
    }
    if (movable_raw) {
      shared = std::make_shared<T>(std::move(*movable_raw));
      movable_raw = nullptr;
    }
  }

  const T* copyable_raw;
  T* movable_raw;
  std::shared_ptr<T> shared;
};

}  // ::detail
}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_CLIENT_HANDLE_I_H
