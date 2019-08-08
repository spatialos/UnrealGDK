#include "gdk/internal/world_command_storage.h"

namespace gdk {

void WorldCommandStorage::AddReserveEntityIdsResponse(ReserveEntityIdsResponse&& response) {
  reserveIdsResponses.emplace_back(std::move(response));
}

void WorldCommandStorage::AddCreateEntityResponse(CreateEntityResponse&& response) {
  createResponses.emplace_back(std::move(response));
}

void WorldCommandStorage::AddDeleteEntityResponse(DeleteEntityResponse&& response) {
  deleteResponse.emplace_back(std::move(response));
}

void WorldCommandStorage::AddEntityQueryResponse(EntityQueryResponse&& response) {
  queryResponses.emplace_back(std::move(response));
}

const std::vector<ReserveEntityIdsResponse>&
WorldCommandStorage::GetReserveEntityIdsResponses() const {
  return reserveIdsResponses;
}

const std::vector<CreateEntityResponse>& WorldCommandStorage::GetCreateEntityResponses() const {
  return createResponses;
}

const std::vector<DeleteEntityResponse>& WorldCommandStorage::GetDeleteEntityResponses() const {
  return deleteResponse;
}

const std::vector<EntityQueryResponse>& WorldCommandStorage::GetEntityQueryResponses() const {
  return queryResponses;
}

void WorldCommandStorage::Clear() {
  reserveIdsResponses.clear();
  createResponses.clear();
  deleteResponse.clear();
  queryResponses.clear();
}

}  // namespace gdk
