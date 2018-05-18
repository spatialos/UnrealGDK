// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#ifndef WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_SNAPSHOT_I_H
#define WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_SNAPSHOT_I_H
#include <improbable/collections.h>
#include <improbable/detail/client_handle.i.h>
#include <improbable/detail/extract.i.h>
#include <improbable/worker.h>
#include <cstdint>
#include <string>

namespace worker {
namespace detail {

inline internal::WorkerProtocol_SnapshotInputStream*
MakeSnapshotInputStream(const detail::ComponentInfo& component_info, const std::string& path) {
  internal::WorkerProtocol_SnapshotParameters parameters = {};
  parameters.ComponentVtableCount = static_cast<std::uint32_t>(component_info.Vtables.size());
  parameters.ComponentVtable = reinterpret_cast<const internal::WorkerProtocol_ComponentVtable*>(
      component_info.Vtables.data());
  internal::WorkerProtocol_SnapshotInputStream* input_stream =
      internal::WorkerProtocol_SnapshotInputStream_Create(path.c_str(), &parameters);
  return input_stream;
}

inline internal::WorkerProtocol_SnapshotOutputStream*
MakeSnapshotOutputStream(const detail::ComponentInfo& component_info, const std::string& path) {
  internal::WorkerProtocol_SnapshotParameters parameters = {};
  parameters.ComponentVtableCount = static_cast<std::uint32_t>(component_info.Vtables.size());
  parameters.ComponentVtable = reinterpret_cast<const internal::WorkerProtocol_ComponentVtable*>(
      component_info.Vtables.data());
  internal::WorkerProtocol_SnapshotOutputStream* output_stream =
      internal::WorkerProtocol_SnapshotOutputStream_Create(path.c_str(), &parameters);
  return output_stream;
}

}  // ::detail

inline Option<std::string> LoadSnapshot(const ComponentRegistry& registry, const std::string& path,
                                        std::unordered_map<EntityId, Entity>& entities_output) {
  Option<std::string> opt_error;
  SnapshotInputStream input_stream{registry, path};
  worker::EntityId entity_id_read;
  worker::Entity entity_read;
  while (input_stream.HasNext()) {
    opt_error = input_stream.ReadEntity(entity_id_read, entity_read);
    if (!opt_error.empty()) {
      return opt_error;
    }
    entities_output.emplace(std::move(entity_id_read), std::move(entity_read));
  }
  return {};
}

inline Option<std::string> SaveSnapshot(const ComponentRegistry& registry, const std::string& path,
                                        const std::unordered_map<EntityId, Entity>& entities) {
  Option<std::string> opt_error;
  SnapshotOutputStream output_stream{registry, path};
  for (const auto& id_entity : entities) {
    opt_error = output_stream.WriteEntity(id_entity.first, id_entity.second);
    if (!opt_error.empty()) {
      return opt_error;
    }
  }
  return {};
}

inline SnapshotInputStream::SnapshotInputStream(const ComponentRegistry& registry,
                                                const std::string& path)
: component_info{registry.GetInternalComponentInfo()}
, input_stream{MakeSnapshotInputStream(component_info, path),
               &detail::internal::WorkerProtocol_SnapshotInputStream_Destroy} {}

inline bool SnapshotInputStream::HasNext() {
  return 0 != detail::internal::WorkerProtocol_SnapshotInputStream_HasNext(input_stream.get());
}

inline Option<std::string> SnapshotInputStream::ReadEntity(EntityId& entity_id, Entity& entity) {
  entity = Entity{};
  // WorkerProtocol_SnapshotInputStream_ReadEntity manages the memory for wp_entity internally.
  const auto* wp_entity =
      detail::internal::WorkerProtocol_SnapshotInputStream_ReadEntity(input_stream.get());

  const char* error =
      detail::internal::WorkerProtocol_SnapshotInputStream_GetError(input_stream.get());
  if (error) {
    return {error};
  }
  entity_id = wp_entity->EntityId;
  for (std::uint32_t i = 0; i < wp_entity->ComponentCount; ++i) {
    const auto& component = wp_entity->Component[i];
    auto it = component_info.MoveSnapshotIntoEntity.find(component.ComponentId);
    if (it != component_info.MoveSnapshotIntoEntity.end()) {
      // Moves the snapshot data out of the C-owned object. Fine to steal the data here.
      it->second(const_cast<detail::ClientHandleBase*>(
                     static_cast<const detail::ClientHandleBase*>(component.Handle)),
                 entity);
    }
  }
  return {};
}

inline SnapshotOutputStream::SnapshotOutputStream(const ComponentRegistry& registry,
                                                  const std::string& path)
: component_info{registry.GetInternalComponentInfo()}
, output_stream{MakeSnapshotOutputStream(component_info, path),
                &detail::internal::WorkerProtocol_SnapshotOutputStream_Destroy} {
  // We are never handling failure in constructor, the caller will be notified when they
  // attempt to use stream again.
}

inline Option<std::string> SnapshotOutputStream::WriteEntity(EntityId entity_id,
                                                             const Entity& entity) {
  detail::internal::WorkerProtocol_Entity snapshot_entity;
  snapshot_entity.EntityId = entity_id;
  std::vector<std::unique_ptr<detail::ClientHandleBase>> handle_storage;
  std::unique_ptr<detail::internal::WorkerProtocol_ComponentHandle[]> component_snapshots;
  ExtractEntityComponents(component_info, entity, snapshot_entity.ComponentCount, handle_storage,
                          component_snapshots);
  snapshot_entity.Component = component_snapshots.get();
  if (0 == detail::internal::WorkerProtocol_SnapshotOutputStream_WriteEntity(output_stream.get(),
                                                                             &snapshot_entity)) {
    const char* error =
        detail::internal::WorkerProtocol_SnapshotOutputStream_GetError(output_stream.get());
    if (error) {
      return {error};
    }
  }
  return {};
}

}  // ::worker

#endif  // WORKER_SDK_CPP_INCLUDE_IMPROBABLE_DETAIL_SNAPSHOT_I_H
