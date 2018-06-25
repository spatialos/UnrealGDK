// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_COMPONENTS_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_COMPONENTS_I_H
#include <improbable/detail/client_handle.i.h>
#include <improbable/worker.h>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace worker {
namespace detail {

struct ComponentInfo {
  std::vector<internal::WorkerProtocol_ComponentVtable> Vtables;
  std::unordered_map<ComponentId, std::function<std::unique_ptr<ClientHandleBase>(const Entity&)>>
      ExtractSnapshot;
  std::unordered_map<ComponentId, std::function<void(ClientHandleBase*, Entity&)>>
      MoveSnapshotIntoEntity;
};

struct CreateComponentInfoHandler {
  ComponentInfo& component_info;
  template <typename T>
  void Accept() const {
    using DataHandle = ClientHandle<typename T::Data>;
    component_info.Vtables.emplace_back(T::Vtable());
    component_info.ExtractSnapshot[T::ComponentId] = [](const Entity& entity) {
      return std::unique_ptr<ClientHandleBase>{
          new DataHandle{DataHandle::copyable(entity.Get<T>().data())}};
    };
    component_info.MoveSnapshotIntoEntity[T::ComponentId] = [](ClientHandleBase* snapshot,
                                                               Entity& entity) {
      entity.Add<T>(std::move(DataHandle::get_mutable(snapshot)));
    };
  }
};

// Storage for arbitrary component data.
class ComponentStorageBase {
public:
  virtual ~ComponentStorageBase(){};
  virtual std::unique_ptr<ComponentStorageBase> Copy() const = 0;
};

template <typename T>
class ComponentStorage : public ComponentStorageBase {
public:
  explicit ComponentStorage(const typename T::Data& data) : data{data} {}
  explicit ComponentStorage(typename T::Data&& data) : data{std::move(data)} {}
  ~ComponentStorage() override {}

  std::unique_ptr<ComponentStorageBase> Copy() const override {
    return std::unique_ptr<ComponentStorageBase>{new ComponentStorage{data}};
  }

  const typename T::Data& Get() const {
    return data;
  }

  typename T::Data& Get() {
    return data;
  }

private:
  typename T::Data data;
};

}  // ::detail

template <typename... Args>
detail::ComponentInfo& Components<Args...>::GetInternalComponentInfo() const {
  static auto component_info = [this] {
    detail::ComponentInfo result;
    ForEachComponent(*this, detail::CreateComponentInfoHandler{result});
    return result;
  }();
  return component_info;
}

}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_COMPONENTS_I_H
