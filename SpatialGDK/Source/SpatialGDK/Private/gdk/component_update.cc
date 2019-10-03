#include "gdk/component_update.h"

#include <utility>

namespace gdk {

ComponentUpdate::ComponentUpdate(Schema_ComponentUpdate* update, ComponentId id)
: componentId(id), update(update) {}

ComponentUpdate::ComponentUpdate(const Worker_ComponentUpdate& update)
: ComponentUpdate(Schema_CopyComponentUpdate(update.schema_type), update.component_id) {}

ComponentUpdate::ComponentUpdate(ComponentId componentId)
: ComponentUpdate(Schema_CreateComponentUpdate(), componentId) {}

ComponentUpdate ComponentUpdate::Clone() const {
  return ComponentUpdate{Schema_CopyComponentUpdate(update.get()), componentId};
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

Schema_ComponentUpdate* ComponentUpdate::GetUnderlying() const {
  return update.get();
}

ComponentId ComponentUpdate::GetComponentId() const {
  return componentId;
}

void ComponentUpdate::Deleter::operator()(Schema_ComponentUpdate* update) const noexcept {
  Schema_DestroyComponentUpdate(update);
}

}  // namespace gdk
