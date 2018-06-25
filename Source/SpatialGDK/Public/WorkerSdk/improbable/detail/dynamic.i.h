// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_DYNAMIC_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_DYNAMIC_I_H
#include <improbable/worker.h>
#include <unordered_map>
#include <utility>

namespace worker {
namespace detail {

template <typename H>
struct DynamicDispatch {
  virtual void InvokeHandler(H& handler) const = 0;
};

template <typename H, typename T>
struct DispatchImpl : DynamicDispatch<H> {
  void InvokeHandler(H& handler) const override {
    handler.template Accept<T>();
  }
};

template <typename H>
using DynamicDispatchMap = std::unordered_map<ComponentId, std::unique_ptr<DynamicDispatch<H>>>;

template <typename H>
struct RegisterHandler {
  DynamicDispatchMap<H>& dispatch_map;

  template <typename T>
  void Accept() const {
    dispatch_map.emplace(T::ComponentId,
                         std::unique_ptr<DynamicDispatch<H>>{new DispatchImpl<H, T>});
  }
};

template <typename... T>
struct MultiDispatch {
  // MultiDispatch<T0, T1, ...>::InvokeForAll(handler) calls each of
  // handler.Accept<T0>(), handler.Accept<T1>() and so on in order.
  template <typename H>
  static void InvokeForAll(H&& handler) {
    using evaluate = int[];
    (void)handler;  // MSVC incorrectly thinks this is an unused variable.
    (void)evaluate{0, ((void)DispatchImpl<H, T>{}.InvokeHandler(handler), 0)...};
  }

  template <typename H>
  static DynamicDispatchMap<H> CreateDynamicDispatchMap() {
    DynamicDispatchMap<H> result;
    InvokeForAll(RegisterHandler<H>{result});
    return result;
  }
};

}  // ::detail

template <typename... T, typename Handler>
void ForComponent(const Components<T...>&, ComponentId component_id, Handler&& handler) {
  static const auto dynamic_dispatch_map =
      detail::MultiDispatch<T...>::template CreateDynamicDispatchMap<Handler>();
  auto it = dynamic_dispatch_map.find(component_id);
  if (it != dynamic_dispatch_map.end()) {
    it->second->InvokeHandler(std::forward<Handler>(handler));
  }
}

template <typename... T, typename Handler>
void ForEachComponent(const Components<T...>&, Handler&& handler) {
  detail::MultiDispatch<T...>::InvokeForAll(std::forward<Handler>(handler));
}

}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_DYNAMIC_I_H
