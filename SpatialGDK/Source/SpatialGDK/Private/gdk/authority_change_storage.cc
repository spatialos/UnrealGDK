#include "gdk/internal/authority_change_storage.h"

#include <algorithm>

namespace gdk {

void AuthorityChangeStorage::SetAuthority(EntityId entityId, ComponentId componentId,
                                          Worker_Authority authority) {
  EntityComponentId entityComponent{entityId, componentId};
  switch (authority) {
  case WORKER_AUTHORITY_NOT_AUTHORITATIVE:
    TryRemoveEntityComponent(&authorityLossImminent, entityComponent);
    TryRemoveEntityComponent(&authorityLossTemporary, entityComponent);
    if (!TryRemoveEntityComponent(&authorityGained, entityComponent)) {
      authorityLost.emplace_back(entityComponent);
    }
    break;
  case WORKER_AUTHORITY_AUTHORITATIVE:
    if (!TryRemoveEntityComponent(&authorityLost, entityComponent)) {
      authorityGained.emplace_back(entityComponent);
    } else {
      authorityLossTemporary.emplace_back(entityComponent);
    }
    break;
  case WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT:
    authorityLossImminent.emplace_back(entityComponent);
    break;
  }
}

void AuthorityChangeStorage::RemoveComponent(EntityId entityId, ComponentId componentId) {
  const EntityComponentId entityComponent{entityId, componentId};
  TryRemoveEntityComponent(&authorityGained, entityComponent);
  TryRemoveEntityComponent(&authorityLost, entityComponent);
  TryRemoveEntityComponent(&authorityLossImminent, entityComponent);
  TryRemoveEntityComponent(&authorityLossTemporary, entityComponent);
}

void AuthorityChangeStorage::RemoveEntity(EntityId entityId) {
  TryRemoveEntity(&authorityGained, entityId);
  TryRemoveEntity(&authorityLost, entityId);
  TryRemoveEntity(&authorityLossImminent, entityId);
  TryRemoveEntity(&authorityLossTemporary, entityId);
}

void AuthorityChangeStorage::Clear() {
  authorityGained.clear();
  authorityLost.clear();
  authorityLossImminent.clear();
  authorityLossTemporary.clear();
}

const std::vector<EntityComponentId>& AuthorityChangeStorage::GetAuthorityGained() const {
  return authorityGained;
}

const std::vector<EntityComponentId>& AuthorityChangeStorage::GetAuthorityLost() const {
  return authorityLost;
}

const std::vector<EntityComponentId>& AuthorityChangeStorage::GetAuthorityLossImminent() const {
  return authorityLossImminent;
}

const std::vector<EntityComponentId>& AuthorityChangeStorage::GetAuthorityLostTemporarily() const {
  return authorityLossTemporary;
}

bool AuthorityChangeStorage::TryRemoveEntityComponent(std::vector<EntityComponentId>* vec,
                                                      const EntityComponentId& entityComponentId) {
  const auto it = std::find(vec->begin(), vec->end(), entityComponentId);
  if (it == vec->end()) {
    return false;
  }
  vec->erase(it);
  return true;
}

void AuthorityChangeStorage::TryRemoveEntity(std::vector<EntityComponentId>* vec,
                                             EntityId entityId) {
  const auto it = std::remove_if(vec->begin(), vec->end(),
                                 [entityId](const EntityComponentId& entityComponent) {
                                   return entityComponent.EntityId == entityId;
                                 });
  vec->erase(it, vec->end());
}

}  // namespace gdk
