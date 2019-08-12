#include "gdk/entity_snapshot.h"

#include <utility>

namespace gdk {

EntitySnapshot::EntitySnapshot(const Worker_Entity& entity)
: entityId(entity.entity_id), entityState(entity) {}

EntitySnapshot::EntitySnapshot(EntityId entityId, EntityState&& entityState)
: entityId(entityId), entityState(std::move(entityState)) {}

EntitySnapshot EntitySnapshot::Clone() const {
  return EntitySnapshot{entityId, entityState.Clone()};
}

EntityId EntitySnapshot::GetEntityId() const {
  return entityId;
}

EntityState& EntitySnapshot::GetEntityState() {
  return entityState;
}

const EntityState& EntitySnapshot::GetEntityState() const {
  return entityState;
}

const ComponentData* EntitySnapshot::GetComponentData(ComponentId componentId) const {
  return entityState.GetComponentData(componentId);
}

}  // namespace gdk
