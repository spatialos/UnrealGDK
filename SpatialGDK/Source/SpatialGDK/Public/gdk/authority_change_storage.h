#ifndef AUTHORITY_CHANGE_STORAGE_H
#define AUTHORITY_CHANGE_STORAGE_H
#include "entity_component_id.h"
#include <WorkerSDK/improbable/c_worker.h>
#include <vector>

namespace gdk {

class AuthorityChangeStorage {
public:
  void SetAuthority(EntityId entityId, ComponentId componentId, Worker_Authority authority);
  void RemoveComponent(EntityId entityId, ComponentId componentId);
  void RemoveEntity(EntityId entityId);

  void Clear();

  const std::vector<EntityComponentId>& GetAuthorityGained() const;
  const std::vector<EntityComponentId>& GetAuthorityLost() const;
  const std::vector<EntityComponentId>& GetAuthorityLossImminent() const;
  const std::vector<EntityComponentId>& GetAuthorityLostTemporarily() const;

private:
  static bool TryRemoveEntityComponent(std::vector<EntityComponentId>* vec,
                                       const EntityComponentId& entityComponentId);
  static void TryRemoveEntity(std::vector<EntityComponentId>* vec, EntityId entityId);

  std::vector<EntityComponentId> authorityGained;
  std::vector<EntityComponentId> authorityLost;
  std::vector<EntityComponentId> authorityLossImminent;
  std::vector<EntityComponentId> authorityLossTemporary;
};

}  // namespace gdk
#endif  // AUTHORITY_CHANGE_STORAGE_H
