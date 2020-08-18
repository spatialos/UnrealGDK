#pragma once

#include "Containers/Array.h"
#include "SpatialView/EntityComponentId.h"

namespace SpatialGDK
{
// A record of authority changes to entity-components.
// Authority for an entity-component can be in at most one of the following states:
//  Recorded as gained.
//  Recorded as lost.
//  Recorded as lost-temporarily.
class AuthorityRecord
{
public:
	// Record an authority change for an entity-component.
	//
	// The following values of `authority` will cause these respective state transitions:
	//  WORKER_AUTHORITY_NOT_AUTHORITATIVE
	//    not recorded -> lost
	//    gained -> not recorded
	//    lost -> UNDEFINED
	//    lost-temporarily -> lost
	//  WORKER_AUTHORITY_AUTHORITATIVE
	//    not recorded -> gained
	//    gained -> UNDEFINED
	//    lost -> lost-temporarily
	//    lost-temporarily -> UNDEFINED
	//  WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT
	//    ignored
	void SetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Worker_Authority Authority);

	// Remove all records.
	void Clear();

	// Get all entity-components with an authority change recorded as gained.
	const TArray<EntityComponentId>& GetAuthorityGained() const;
	// Get all entity-components with an authority change recorded as lost.
	const TArray<EntityComponentId>& GetAuthorityLost() const;
	// Get all entity-components with an authority change recorded as lost-temporarily.
	const TArray<EntityComponentId>& GetAuthorityLostTemporarily() const;

private:
	TArray<EntityComponentId> AuthorityGained;
	TArray<EntityComponentId> AuthorityLost;
	TArray<EntityComponentId> AuthorityLossTemporary;
};

} // namespace SpatialGDK
