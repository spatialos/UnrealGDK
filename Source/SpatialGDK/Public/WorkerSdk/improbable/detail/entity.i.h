// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_ENTITY_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_ENTITY_I_H
#include <improbable/collections.h>
#include <improbable/detail/components.i.h>
#include <improbable/worker.h>

namespace worker {

inline Entity::Entity(const Entity& entity) {
  for (const auto& pair : entity.components) {
    components.emplace(pair.first, pair.second->Copy());
  }
}

inline Entity& Entity::operator=(const Entity& entity) {
  if (&entity != this) {
    components.clear();
    for (const auto& pair : entity.components) {
      components.emplace(pair.first, pair.second->Copy());
    }
  }
  return *this;
}

template <typename T>
Option<typename T::Data&> Entity::Get() {
  auto it = components.find(T::ComponentId);
  if (it == components.end()) {
    return {};
  }
  return static_cast<detail::ComponentStorage<T>&>(*it->second).Get();
}

template <typename T>
Option<const typename T::Data&> Entity::Get() const {
  auto it = components.find(T::ComponentId);
  if (it == components.end()) {
    return {};
  }
  return static_cast<const detail::ComponentStorage<T>&>(*it->second).Get();
}

template <typename T>
void Entity::Add(const typename T::Data& data) {
  auto it = components.find(T::ComponentId);
  if (it == components.end()) {
    components.emplace(T::ComponentId, std::unique_ptr<detail::ComponentStorageBase>{
                                           new detail::ComponentStorage<T>{data}});
  }
}

template <typename T>
void Entity::Add(typename T::Data&& data) {
  auto it = components.find(T::ComponentId);
  if (it == components.end()) {
    components.emplace(T::ComponentId, std::unique_ptr<detail::ComponentStorageBase>{
                                           new detail::ComponentStorage<T>{std::move(data)}});
  }
}

template <typename T>
void Entity::Update(const typename T::Update& update) {
  auto it = components.find(T::ComponentId);
  if (it != components.end()) {
    update.ApplyTo(static_cast<detail::ComponentStorage<T>&>(*it->second).Get());
  }
}

template <typename T>
void Entity::Remove() {
  components.erase(T::ComponentId);
}

inline List<ComponentId> Entity::GetComponentIds() const {
  List<ComponentId> ids;
  for (const auto& kv : components) {
    ids.emplace_back(kv.first);
  }
  return ids;
}

}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_ENTITY_I_H
