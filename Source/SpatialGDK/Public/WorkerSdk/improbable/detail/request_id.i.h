// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_REQUEST_ID_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_REQUEST_ID_I_H
#include <improbable/worker.h>
#include <cstdint>
#include <functional>

namespace std {
template <typename T>
struct hash<worker::RequestId<T>> {
  std::size_t operator()(const worker::RequestId<T>& key) const {
    return std::hash<std::uint32_t>{}(key.Id);
  }
};
}  // ::std

namespace worker {

template <typename T>
RequestId<T>::RequestId() : Id{0} {}

template <typename T>
RequestId<T>::RequestId(std::uint32_t id) : Id{id} {}

template <typename T>
bool RequestId<T>::operator==(const RequestId& other) const {
  return Id == other.Id;
}

template <typename T>
bool RequestId<T>::operator!=(const RequestId& other) const {
  return !operator==(other);
}

}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_REQUEST_ID_I_H
