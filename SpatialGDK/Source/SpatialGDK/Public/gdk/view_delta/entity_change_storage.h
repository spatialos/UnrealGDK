#ifndef GDK_ENTITY_CHANGE_STORAGE_H
#define GDK_ENTITY_CHANGE_STORAGE_H
#include "gdk/worker_sdk.h"
#include "gdk/collections/flat_set.h"
#include <vector>

namespace gdk {

class ComponentUpdate;

class EntityChangeStorage {
public:
  EntityChangeStorage() = default;
  ~EntityChangeStorage() = default;

  // Moveable, not copyable
  EntityChangeStorage(const EntityChangeStorage&) = delete;
  EntityChangeStorage(EntityChangeStorage&&) = default;
  EntityChangeStorage& operator=(const EntityChangeStorage&) = delete;
  EntityChangeStorage& operator=(EntityChangeStorage&&) = default;

  bool TryAddEntity(EntityId entityId);
  void RemoveEntity(EntityId entityId);

  void Clear();

  const std::vector<EntityId>& GetEntitiesAdded() const;
  const std::vector<EntityId>& GetEntitiesRemoved() const;

private:
  bool TryCancelEntityAdd(EntityId entityId);
  bool TryCancelEntityRemove(EntityId entityId);

  FlatSet<EntityId> entitiesAdded;
  FlatSet<EntityId> entitiesRemoved;
};

}  // namespace gdk
#endif  // GDK_ENTITY_CHANGE_STORAGE_H
