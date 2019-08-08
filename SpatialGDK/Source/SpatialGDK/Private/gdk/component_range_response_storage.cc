#include "gdk/internal/component_range_response_storage.h"

namespace gdk {

void ComponentRangeResponseStorage::Clear() {
  responses.clear();
}

void ComponentRangeResponseStorage::AddResponse(CommandResponseReceived&& request) {
  responses.emplace_back(std::move(request));
}

const std::vector<CommandResponseReceived>& ComponentRangeResponseStorage::GetResponses() const {
  return responses;
}

}  // namespace gdk