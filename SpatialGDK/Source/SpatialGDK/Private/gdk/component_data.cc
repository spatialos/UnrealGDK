#include "gdk/component_data.h"

#include "gdk/component_update.h"
#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>
#include <utility>

namespace gdk {
ComponentData::ComponentData(Schema_ComponentData* data) : data(data) {}

ComponentData::ComponentData(const Worker_ComponentData& data)
: ComponentData(Schema_CopyComponentData(data.schema_type)) {}

ComponentData::ComponentData(ComponentId componentId)
: ComponentData(Schema_CreateComponentData(componentId)) {}

ComponentData::ComponentData(ComponentData&& other) noexcept : data(nullptr) {
  *this = std::move(other);
}

ComponentData& ComponentData::operator=(ComponentData&& other) noexcept {
  data.reset(std::move(other).Release());
  return *this;
}

ComponentData ComponentData::Clone() const {
  return ComponentData{Schema_CopyComponentData(data.get())};
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

Schema_ComponentData* ComponentData::GetUnderlying() {
  if (data == nullptr) {
    return nullptr;
  }
  return data.get();
}

const Schema_ComponentData* ComponentData::GetUnderlying() const {
  return data.get();
}

ComponentId ComponentData::GetComponentId() const {
  if (data == nullptr) {
    return 0;
  }
  return Schema_GetComponentDataComponentId(data.get());
}

void ComponentData::Deleter::operator()(Schema_ComponentData* data) const noexcept {
  Schema_DestroyComponentData(data);
}

}  // namespace gdk
