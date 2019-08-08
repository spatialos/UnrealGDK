#include "gdk/visible_entity.h"

#include "gdk/component_data.h"
#include "gdk/component_update.h"

namespace gdk {

VisibleEntity::VisibleEntity(EntityId entityId) : entityId(entityId) {}

VisibleEntity::VisibleEntity(const Worker_Entity& entity)
: entityId(entity.entity_id), startingComponents(entity), allComponents(entity.component_count) {}

VisibleEntity::VisibleEntity(EntityId entityId, EntityState&& startingEntity)
: entityId(entityId)
, startingComponents(std::move(startingEntity))
, allComponents(startingComponents.GetComponents().size()) {
  for (const auto& component : startingComponents.GetComponents()) {
    AddComponentId(component.GetComponentId());
  }
}

void VisibleEntity::AddComponentId(ComponentId id) {
  allComponents.insert(id);
}

void VisibleEntity::RemoveComponentId(ComponentId id) {
  if (allComponents.erase_first(id)) {
    startingComponents.TryRemoveComponent(id);
  }
}

void VisibleEntity::AddStartingComponent(ComponentData&& data) {
  AddComponentId(data.GetComponentId());
  startingComponents.AddComponent(std::move(data));
}

bool VisibleEntity::TrySetStartingComponent(ComponentData&& data) {
  return startingComponents.TrySetComponentData(std::move(data));
}

bool VisibleEntity::TryUpdateStartingComponent(const ComponentUpdate& update) {
  return startingComponents.TryApplyComponentUpdate(update);
}

EntityId VisibleEntity::GetEntityId() const {
  return entityId;
}

const EntityState& VisibleEntity::GetStartingComponents() const {
  return startingComponents;
}

const std::vector<ComponentId>& VisibleEntity::GetAllComponentIds() const {
  return allComponents.GetUnderlying();
}

}  // namespace gdk
