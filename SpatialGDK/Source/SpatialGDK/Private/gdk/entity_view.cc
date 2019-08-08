#include "gdk/internal/entity_view.h"

#include <algorithm>

namespace gdk {

void EntityView::AddEntity(EntityId entityId) {
  entities.emplace(entityId, std::vector<ComponentId>());
}

void EntityView::RemoveEntity(EntityId entityId) {
  entities.erase(entityId);
}

void EntityView::AddComponent(EntityId entityId, ComponentId componentId) {
  entities[entityId].emplace_back(componentId);
}

void EntityView::RemoveComponent(EntityId entityId, ComponentId componentId) {
  auto& components = entities[entityId];
  const auto it = std::find(components.begin(), components.end(), componentId);
  if (it != components.end()) {
    components.erase(it);
  }
}

bool EntityView::HasComponent(EntityId entityId, ComponentId componentId) const {
  const auto entityIt = entities.find(entityId);
  if (entityIt == entities.end()) {
    return false;
  }
  const auto componentIt = std::find(entityIt->second.begin(), entityIt->second.end(), componentId);
  return componentIt != entityIt->second.end();
}

const std::vector<ComponentId>& EntityView::GetComponentIds(EntityId entityId) const {
  return entities.find(entityId)->second;
}

}  // namespace gdk
