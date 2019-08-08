#ifndef GDK_ENTITY_SNAPSHOT_H
#define GDK_ENTITY_SNAPSHOT_H
#include "gdk/common_types.h"
#include "gdk/component_data.h"
#include "gdk/entity_state.h"
#include <WorkerSDK/improbable/c_worker.h>
namespace gdk {

/** An entity with immutable state. */
class EntitySnapshot {
public:
  explicit EntitySnapshot(const Worker_Entity& entity);
  EntitySnapshot(EntityId entityId, EntityState&& entityState);

  ~EntitySnapshot() = default;

  // Moveable, not copyable
  EntitySnapshot(const EntitySnapshot&) = delete;
  EntitySnapshot(EntitySnapshot&&) = default;
  EntitySnapshot& operator=(const EntitySnapshot&) = delete;
  EntitySnapshot& operator=(EntitySnapshot&&) = default;

  EntitySnapshot Clone() const;

  EntityId GetEntityId() const;
  const EntityState& GetEntityState() const;
  const ComponentData* GetComponentData(ComponentId componentId) const;

private:
  EntityId entityId;
  EntityState entityState;
};

}  // namespace gdk
#endif  // GDK_ENTITY_SNAPSHOT_H
