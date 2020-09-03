// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Algo/Compare.h"
#include "ComponentTestUtils.h"
#include "SpatialView/ViewDelta.h"
#include "Tests/SpatialView/ExpectedEntityDelta.h"
#include "Tests/SpatialView/ExptectedViewDelta.h"

using namespace SpatialGDK;

ExpectedViewDelta& ExpectedViewDelta::AddEntityDelta(const Worker_EntityId EntityId, const EntityChangeType ChangeType)
{
	EntityDeltas.Add(EntityId, { EntityId, ChangeType == ADD, ChangeType == REMOVE });
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddComponentAdded(const Worker_EntityId EntityId, ComponentData Data)
{
	EntityDeltas[EntityId].ComponentsAdded.Push(ComponentChange(Data.GetComponentId(), Data.GetUnderlying()));
	EntityDeltas[EntityId].DataStorage.Push(MoveTemp(Data));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddComponentRemoved(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	EntityDeltas[EntityId].ComponentsRemoved.Push(ComponentChange(ComponentId));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddComponentUpdate(const Worker_EntityId EntityId, ComponentUpdate Update)
{
	EntityDeltas[EntityId].ComponentUpdates.Push(ComponentChange(Update.GetComponentId(), Update.GetUnderlying()));
	EntityDeltas[EntityId].UpdateStorage.Push(MoveTemp(Update));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddComponentRefreshed(const Worker_EntityId EntityId, ComponentUpdate Update, ComponentData Data)
{
	EntityDeltas[EntityId].ComponentsRefreshed.Push(ComponentChange(Update.GetComponentId(), Data.GetUnderlying(), Update.GetEvents()));
	EntityDeltas[EntityId].DataStorage.Push(MoveTemp(Data));
	EntityDeltas[EntityId].UpdateStorage.Push(MoveTemp(Update));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddAuthorityGained(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	EntityDeltas[EntityId].AuthorityGained.Add(AuthorityChange(ComponentId, AuthorityChange::AUTHORITY_GAINED));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddAuthorityLost(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	EntityDeltas[EntityId].AuthorityLost.Add(AuthorityChange(ComponentId, AuthorityChange::AUTHORITY_LOST));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddAuthorityLostTemporarily(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	EntityDeltas[EntityId].AuthorityLostTemporarily.Add(AuthorityChange(ComponentId, AuthorityChange::AUTHORITY_LOST_TEMPORARILY));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddDisconnect(const uint8 StatusCode, FString StatusMessage)
{
	ConnectionStatusCode = StatusCode;
	ConnectionStatusMessage = MoveTemp(StatusMessage);
	return *this;
}

void ExpectedViewDelta::SortEntityDeltas()
{
	for (auto& Pair : EntityDeltas)
	{
		Pair.Value.AuthorityGained.Sort(CompareAuthorityChangeById);
		Pair.Value.AuthorityLost.Sort(CompareAuthorityChangeById);
		Pair.Value.AuthorityLostTemporarily.Sort(CompareAuthorityChangeById);
		Pair.Value.ComponentsAdded.Sort(CompareComponentChangeById);
		Pair.Value.ComponentsRemoved.Sort(CompareComponentChangeById);
		Pair.Value.ComponentsRefreshed.Sort(CompareComponentChangeById);
		Pair.Value.ComponentUpdates.Sort(CompareComponentChangeById);
	}

	EntityDeltas.KeySort(CompareWorkerEntityIdKey);
}

bool ExpectedViewDelta::Compare(const ViewDelta& Other)
{
	TArray<EntityDelta> RhsEntityDeltas = Other.GetEntityDeltas();
	if (EntityDeltas.Num() != RhsEntityDeltas.Num())
	{
		return false;
	}

	SortEntityDeltas();
	TArray<uint32> DeltaKeys;
	EntityDeltas.GetKeys(DeltaKeys);
	for (int32 i = 0; i < DeltaKeys.Num(); ++i)
	{
		ExpectedEntityDelta& LhsEntityDelta = EntityDeltas[DeltaKeys[i]];
		EntityDelta RhsEntityDelta = RhsEntityDeltas[i];
		if (LhsEntityDelta.EntityId != RhsEntityDelta.EntityId)
		{
			return false;
		}

		if (LhsEntityDelta.bAdded != RhsEntityDelta.bAdded)
		{
			return false;
		}

		if (LhsEntityDelta.bRemoved != RhsEntityDelta.bRemoved)
		{
			return false;
		}

		if (!Algo::CompareByPredicate(LhsEntityDelta.AuthorityGained, RhsEntityDelta.AuthorityGained, CompareAuthorityChanges))
		{
			return false;
		}

		if (!Algo::CompareByPredicate(LhsEntityDelta.AuthorityLost, RhsEntityDelta.AuthorityLost, CompareAuthorityChanges))
		{
			return false;
		}

		if (!Algo::CompareByPredicate(LhsEntityDelta.AuthorityLostTemporarily, RhsEntityDelta.AuthorityLostTemporarily,
									  CompareAuthorityChanges))
		{
			return false;
		}

		if (!Algo::CompareByPredicate(LhsEntityDelta.ComponentsAdded, RhsEntityDelta.ComponentsAdded, CompareComponentChanges))
		{
			return false;
		}

		if (!Algo::CompareByPredicate(LhsEntityDelta.ComponentsRemoved, RhsEntityDelta.ComponentsRemoved, CompareComponentChanges))
		{
			return false;
		}

		if (!Algo::CompareByPredicate(LhsEntityDelta.ComponentsRefreshed, RhsEntityDelta.ComponentsRefreshed, CompareComponentChanges))
		{
			return false;
		}

		if (!Algo::CompareByPredicate(LhsEntityDelta.ComponentUpdates, RhsEntityDelta.ComponentUpdates, CompareComponentChanges))
		{
			return false;
		}
	}

	return true;
}
