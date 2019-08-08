#include "gdk/component_update.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>
#include <utility>

namespace gdk {

ComponentUpdate::ComponentUpdate(Schema_ComponentUpdate* update) : update(update) {}

ComponentUpdate::ComponentUpdate(const Worker_ComponentUpdate& update)
: ComponentUpdate(Schema_CopyComponentUpdate(update.schema_type)) {}

ComponentUpdate::ComponentUpdate(ComponentId componentId)
: ComponentUpdate(Schema_CreateComponentUpdate(componentId)) {}

ComponentUpdate::ComponentUpdate(ComponentUpdate&& other) noexcept : update(nullptr) {
  *this = std::move(other);
}

ComponentUpdate& ComponentUpdate::operator=(ComponentUpdate&& other) noexcept {
  update.reset(std::move(other).Release());
  return *this;
}

ComponentUpdate ComponentUpdate::Clone() const {
  return ComponentUpdate{Schema_CopyComponentUpdate(update.get())};
}

Schema_ComponentUpdate* ComponentUpdate::Release() && {
  return update.release();
}

bool ComponentUpdate::Merge(ComponentUpdate other) {
  if (other.GetComponentId() != GetComponentId() || other.update == nullptr) {
    return false;
  }

  return Schema_MergeComponentUpdateIntoUpdate(other.GetUnderlying(), update.get());
}

Schema_Object* ComponentUpdate::GetFields() {
  if (update == nullptr) {
    return nullptr;
  }
  return Schema_GetComponentUpdateFields(update.get());
}

const Schema_Object* ComponentUpdate::GetFields() const {
  if (update == nullptr) {
    return nullptr;
  }
  return Schema_GetComponentUpdateFields(update.get());
}

Schema_Object* ComponentUpdate::GetEvents() {
  if (update == nullptr) {
    return nullptr;
  }
  return Schema_GetComponentUpdateEvents(update.get());
}

const Schema_Object* ComponentUpdate::GetEvents() const {
  if (update == nullptr) {
    return nullptr;
  }
  return Schema_GetComponentUpdateEvents(update.get());
}

Schema_ComponentUpdate* ComponentUpdate::GetUnderlying() {
  return update.get();
}

const Schema_ComponentUpdate* ComponentUpdate::GetUnderlying() const {
  return update.get();
}

ComponentId ComponentUpdate::GetComponentId() const {
  if (update == nullptr) {
    return 0;
  }
  return Schema_GetComponentUpdateComponentId(update.get());
}

void ComponentUpdate::Deleter::operator()(Schema_ComponentUpdate* update) const noexcept {
  Schema_DestroyComponentUpdate(update);
}

}  // namespace gdk
