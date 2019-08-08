#ifndef GDK_ENTITY_COMPONENT_MESSAGES_H
#define GDK_ENTITY_COMPONENT_MESSAGES_H
#include "gdk/common_types.h"
#include "gdk/component_data.h"
#include "gdk/component_update.h"
#include "gdk/entity_component_id.h"

namespace gdk {

struct EntityComponentUpdate {
  EntityId EntityId;
  ComponentUpdate Update;

  EntityComponentId GetEntityComponentId() const {
    return EntityComponentId{EntityId, Update.GetComponentId()};
  }

  explicit operator EntityComponentId() const {
    return GetEntityComponentId();
  }
};

struct EntityComponentData {
  EntityId EntityId;
  ComponentData Data;

  EntityComponentId GetEntityComponentId() const {
    return EntityComponentId{EntityId, Data.GetComponentId()};
  }

  explicit operator EntityComponentId() const {
    return GetEntityComponentId();
  }
};

}  // namespace gdk
#endif  // GDK_ENTITY_COMPONENT_MESSAGES_H
