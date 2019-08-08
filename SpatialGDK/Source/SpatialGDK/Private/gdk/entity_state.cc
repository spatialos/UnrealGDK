#include "gdk/entity_state.h"

#include "gdk/component_update.h"
#include <algorithm>
#include <cstdint>

namespace gdk {

EntityState::EntityState(const Worker_Entity& entity) {
  for (std::uint32_t i = 0; i < entity.component_count; ++i) {
    components.emplace_back(entity.components[i]);
  }
}

EntityState EntityState::Clone() const {
  EntityState clone{};
  for (const auto& component : components) {
    clone.AddComponent(component.Clone());
  }
  return clone;
}

std::vector<Worker_ComponentData> EntityState::ReleaseComponentData() && {
  std::vector<Worker_ComponentData> data(components.size());
  for (size_t i = 0; i < components.size(); ++i) {
    data[i] = Worker_ComponentData{nullptr, components[i].GetComponentId(),
                                   std::move(components[i]).Release(), nullptr};
  }
  components.clear();
  return data;
}

void EntityState::AddComponent(ComponentData&& data) {
  components.emplace_back(std::move(data));
}

bool EntityState::TrySetComponentData(ComponentData&& data) {
  for (auto& component : components) {
    if (component.GetComponentId() == data.GetComponentId()) {
      component = std::move(data);
      return true;
    }
  }
  return false;
}

bool EntityState::TryApplyComponentUpdate(const ComponentUpdate& update) {
  for (auto& component : components) {
    if (component.GetComponentId() == update.GetComponentId()) {
      component.ApplyUpdate(update);
      return true;
    }
  }
  return false;
}

bool EntityState::TryRemoveComponent(ComponentId componentId) {
  for (size_t i = 0; i < components.size(); ++i) {
    if (components[i].GetComponentId() == componentId) {
      components.erase(components.begin() + i);
      return true;
    }
  }
  return false;
}

ComponentData* EntityState::GetComponentData(ComponentId componentId) {
  for (auto& component : components) {
    if (component.GetComponentId() == componentId) {
      return &component;
    }
  }
  return nullptr;
}

const ComponentData* EntityState::GetComponentData(ComponentId componentId) const {
  for (auto& component : components) {
    if (component.GetComponentId() == componentId) {
      return &component;
    }
  }
  return nullptr;
}

std::vector<ComponentData>& EntityState::GetComponents() {
  return components;
}

const std::vector<ComponentData>& EntityState::GetComponents() const {
  return components;
}

}  // namespace gdk
