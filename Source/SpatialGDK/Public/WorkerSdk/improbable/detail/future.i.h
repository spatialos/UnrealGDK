// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_FUTURE_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_FUTURE_I_H
#include <improbable/collections.h>
#include <improbable/worker.h>
#include <cstdint>
#include <exception>
#include <functional>

namespace worker {
namespace detail {

template <typename T>
struct FutureImpl {
  ~FutureImpl() {
    destroy();
  }

  std::function<Option<T>(const Option<std::uint32_t>&)> get;
  std::function<void()> destroy;
  bool moved_from;
  Option<T> result;
};

}  // ::detail

template <typename T>
bool Future<T>::Wait(const Option<std::uint32_t>& timeout_millis) {
  if (!impl->result) {
    impl->result = impl->get(timeout_millis);
  }
  return static_cast<bool>(impl->result);
}

template <typename T>
T Future<T>::Get() {
  if (impl->moved_from) {
    std::terminate();
  }
  if (!impl->result) {
    Wait({});
  }
  impl->moved_from = true;
  return std::move(*impl->result);
}

template <typename T>
Future<T>::Future(const std::function<Option<T>(const Option<std::uint32_t>&)> get,
                  const std::function<void()>& destroy)
: impl{new detail::FutureImpl<T>{get, destroy, false, {}}} {}

}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_FUTURE_I_H
