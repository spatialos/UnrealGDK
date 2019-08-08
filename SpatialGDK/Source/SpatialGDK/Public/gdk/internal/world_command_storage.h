#ifndef GDK_WORLD_COMMAND_STORAGE_H
#define GDK_WORLD_COMMAND_STORAGE_H
#include "gdk/command_messages.h"
#include <vector>

namespace gdk {

class WorldCommandStorage {
public:
  void AddReserveEntityIdsResponse(ReserveEntityIdsResponse&& response);
  void AddCreateEntityResponse(CreateEntityResponse&& response);
  void AddDeleteEntityResponse(DeleteEntityResponse&& response);
  void AddEntityQueryResponse(EntityQueryResponse&& response);

  const std::vector<ReserveEntityIdsResponse>& GetReserveEntityIdsResponses() const;
  const std::vector<CreateEntityResponse>& GetCreateEntityResponses() const;
  const std::vector<DeleteEntityResponse>& GetDeleteEntityResponses() const;
  const std::vector<EntityQueryResponse>& GetEntityQueryResponses() const;

  void Clear();

private:
  std::vector<ReserveEntityIdsResponse> reserveIdsResponses;
  std::vector<CreateEntityResponse> createResponses;
  std::vector<DeleteEntityResponse> deleteResponse;
  std::vector<EntityQueryResponse> queryResponses;
};

}  // namespace gdk
#endif  // GDK_WORLD_COMMAND_STORAGE_H