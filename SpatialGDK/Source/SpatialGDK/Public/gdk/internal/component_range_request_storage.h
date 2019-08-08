#ifndef GDK_COMPONENT_RANGE_REQUEST_STORAGE_H
#define GDK_COMPONENT_RANGE_REQUEST_STORAGE_H
#include "gdk/command_messages.h"
#include <vector>

namespace gdk {

class ComponentRangeRequestStorage {
public:
  ComponentRangeRequestStorage() = default;
  ~ComponentRangeRequestStorage() = default;

  // Moveable, not copyable
  ComponentRangeRequestStorage(const ComponentRangeRequestStorage&) = delete;
  ComponentRangeRequestStorage(ComponentRangeRequestStorage&&) = default;
  ComponentRangeRequestStorage& operator=(const ComponentRangeRequestStorage&) = delete;
  ComponentRangeRequestStorage& operator=(ComponentRangeRequestStorage&&) = default;

  void Clear();

  void AddRequest(CommandRequestReceived&& request);
  void RemoveEntityComponent(EntityId entityId, ComponentId componentId);

  const std::vector<CommandRequestReceived>& GetRequests() const;

private:
  std::vector<CommandRequestReceived> requests;
};

}  // namespace gdk
#endif  // GDK_COMPONENT_RANGE_REQUEST_STORAGE_H
