#include "gdk/internal/component_range_request_storage.h"

#include <algorithm>

namespace gdk {

void ComponentRangeRequestStorage::Clear() {
  requests.clear();
}

void ComponentRangeRequestStorage::AddRequest(CommandRequestReceived&& request) {
  requests.emplace_back(std::move(request));
}

void ComponentRangeRequestStorage::RemoveEntityComponent(EntityId entityId,
                                                         ComponentId componentId) {
  const auto it = std::remove_if(requests.begin(), requests.end(),
                                 [entityId, componentId](const CommandRequestReceived& request) {
                                   return request.EntityId == entityId &&
                                       request.Request.GetComponentId() == componentId;
                                 });
  requests.erase(it, requests.end());
}

const std::vector<CommandRequestReceived>& ComponentRangeRequestStorage::GetRequests() const {
  return requests;
}

}  // namespace gdk
