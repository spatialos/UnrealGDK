#ifndef GDK_COMPONENT_RANGE_RESPONSE_STORAGE_H
#define GDK_COMPONENT_RANGE_RESPONSE_STORAGE_H
#include "command_messages.h"
#include <vector>

namespace gdk {

class ComponentRangeResponseStorage {
public:
  ComponentRangeResponseStorage() = default;
  ~ComponentRangeResponseStorage() = default;

  // Moveable, not copyable
  ComponentRangeResponseStorage(const ComponentRangeResponseStorage&) = delete;
  ComponentRangeResponseStorage(ComponentRangeResponseStorage&&) = default;
  ComponentRangeResponseStorage& operator=(const ComponentRangeResponseStorage&) = delete;
  ComponentRangeResponseStorage& operator=(ComponentRangeResponseStorage&&) = default;

  void Clear();

  void AddResponse(CommandResponseReceived&& request);

  const std::vector<CommandResponseReceived>& GetResponses() const;

private:
  std::vector<CommandResponseReceived> responses;
};

}  // namespace gdk
#endif  // GDK_COMPONENT_RANGE_RESPONSE_STORAGE_H