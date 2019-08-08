#ifndef GDK_ENTITY_TEMPLATE_H
#define GDK_ENTITY_TEMPLATE_H
#include "gdk/common_types.h"
#include "gdk/component_data.h"
#include "gdk/component_update.h"
#include "gdk/entity_state.h"
#include <string>
#include <utility>
#include <vector>
namespace gdk {

class EntityTemplate {
public:
  EntityTemplate() = default;
  ~EntityTemplate() = default;

  // Moveable, not copyable
  EntityTemplate(const EntityTemplate&) = delete;
  EntityTemplate(EntityTemplate&&) = default;
  EntityTemplate& operator=(const EntityTemplate&) = delete;
  EntityTemplate& operator=(EntityTemplate&&) = default;

  EntityTemplate Clone() const;

  EntityState CreateEntityState(const std::string& metadata) const;
  EntityState CreateEntityState() const;

  void AddReadAttribute(const std::string& attribute);
  void AddComponent(ComponentData&& data, const std::string& layer = {});
  void UpdateComponent(const ComponentUpdate& update);
  void RemoveComponent(ComponentId id);

private:
  ComponentData CreateAcl() const;

  EntityState entityState;
  std::string metadata;
  std::vector<std::string> readAttributes;
  std::vector<std::pair<ComponentId, std::string>> idToAttribute;
};

}  // namespace gdk
#endif  // GDK_ENTITY_TEMPLATE_H
