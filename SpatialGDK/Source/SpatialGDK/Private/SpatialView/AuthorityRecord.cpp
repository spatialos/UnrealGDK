#include "SpatialView/AuthorityRecord.h"

namespace SpatialGDK
{

void AuthorityRecord::SetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Worker_Authority Authority)
{
	const EntityComponentId Id = {EntityId, ComponentId};

	switch (Authority)
	{
	case WORKER_AUTHORITY_NOT_AUTHORITATIVE:
		if (AuthorityGained.RemoveSingle(Id) == 0)
		{
			AuthorityLossTemporary.RemoveSingle(Id);
			AuthorityLost.Push(Id);
		}
		break;
	case WORKER_AUTHORITY_AUTHORITATIVE:
		if (AuthorityLost.RemoveSingle(Id) > 0)
		{
			AuthorityLossTemporary.Push(Id);
		}
		else
		{
			AuthorityGained.Push(Id);
		}
		break;
	case WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT:
		// Deliberately ignore loss imminent.
		break;
	}
}

void AuthorityRecord::Clear()
{
	AuthorityGained.Empty();
	AuthorityLost.Empty();
	AuthorityLossTemporary.Empty();
}

const TArray<EntityComponentId>& AuthorityRecord::GetAuthorityGained() const
{
	return AuthorityGained;
}

const TArray<EntityComponentId>& AuthorityRecord::GetAuthorityLost() const
{
	return AuthorityLost;
}

const TArray<EntityComponentId>& AuthorityRecord::GetAuthorityLostTemporarily() const
{
	return AuthorityLossTemporary;
}

} // namespace SpatialGDK
