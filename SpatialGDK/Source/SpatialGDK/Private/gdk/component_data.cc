#include "gdk/component_data.h"

#include "gdk/component_update.h"
#include <utility>

namespace gdk {
ComponentData::ComponentData(Schema_ComponentData* data, ComponentId id)
: componentId(id), data(data) {}

ComponentData::ComponentData(const Worker_ComponentData& data)
: ComponentData(Schema_CopyComponentData(data.schema_type), data.component_id) {}

ComponentData::ComponentData(ComponentId componentId)
: ComponentData(Schema_CreateComponentData(), componentId) {}

ComponentData ComponentData::Clone() const {
  return ComponentData{Schema_CopyComponentData(data.get()), componentId};
}

Schema_ComponentData* ComponentData::Release() && {
  return data.release();
}

bool ComponentData::ApplyUpdate(const ComponentUpdate& update) {
  if (update.GetComponentId() != GetComponentId() || update.GetUnderlying() == nullptr) {
    return false;
  }

  // todo check for errors
  return Schema_ApplyComponentUpdateToData(update.GetUnderlying(), data.get());
}

Schema_Object* ComponentData::GetFields() {
  if (data == nullptr) {
    return nullptr;
  }
  return Schema_GetComponentDataFields(data.get());
}

const Schema_Object* ComponentData::GetFields() const {
  if (data == nullptr) {
    return nullptr;
  }
  return Schema_GetComponentDataFields(data.get());
}

Schema_ComponentData* ComponentData::GetUnderlying() const {
  if (data == nullptr) {
    return nullptr;
  }
  return data.get();
}

ComponentId ComponentData::GetComponentId() const {
  return componentId;
}

void ComponentData::Deleter::operator()(Schema_ComponentData* componentData) const noexcept {
  Schema_DestroyComponentData(componentData);
}

}  // namespace gdk
