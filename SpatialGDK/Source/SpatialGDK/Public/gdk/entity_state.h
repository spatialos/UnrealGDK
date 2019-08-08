#ifndef GDK_ENTITY_STATE_H
#define GDK_ENTITY_STATE_H
#include "gdk/common_types.h"
#include "gdk/component_data.h"
#include <WorkerSDK/improbable/c_worker.h>
#include <vector>
namespace gdk {

class ComponentUpdate;

/** A mutable collection of entity components. */
class EntityState {
public:
  EntityState() = default;
  explicit EntityState(const Worker_Entity& entity);

  ~EntityState() = default;

  // Moveable, not copyable
  EntityState(const EntityState& other) = delete;
  EntityState(EntityState&& other) = default;
  EntityState& operator=(const EntityState& other) = delete;
  EntityState& operator=(EntityState&& other) = default;

  EntityState Clone() const;
  std::vector<Worker_ComponentData> ReleaseComponentData() &&;

  // todo should this detect if the data is already there
  void AddComponent(ComponentData&& data);
  bool TrySetComponentData(ComponentData&& data);
  bool TryApplyComponentUpdate(const ComponentUpdate& update);
  bool TryRemoveComponent(ComponentId componentId);

  ComponentData* GetComponentData(ComponentId componentId);
  const ComponentData* GetComponentData(ComponentId componentId) const;

  std::vector<ComponentData>& GetComponents();
  const std::vector<ComponentData>& GetComponents() const;

private:
  std::vector<ComponentData> components;
};

}  // namespace gdk
#endif  // GDK_ENTITY_STATE_H
