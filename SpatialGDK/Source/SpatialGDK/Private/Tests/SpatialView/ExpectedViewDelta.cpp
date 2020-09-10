// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/SpatialView/ExpectedViewDelta.h"

#include "Algo/Compare.h"
#include "ComponentTestUtils.h"
#include "SpatialView/ViewDelta.h"
#include "Tests/SpatialView/ExpectedEntityDelta.h"

using namespace SpatialGDK;

ExpectedViewDelta& ExpectedViewDelta::AddEntityDelta(const Worker_EntityId EntityId, const EntityChangeType ChangeType)
{
	EntityDeltas.Add(
		EntityId, { EntityId, ChangeType == UPDATE ? ExpectedEntityDelta::UPDATE
												   : ChangeType == ADD ? ExpectedEntityDelta::ADD
																	   : ChangeType == REMOVE ? ExpectedEntityDelta::REMOVE
																							  : ExpectedEntityDelta::TEMPORARILY_REMOVED });
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

	EntityDeltas.KeySort(CompareWorkerEntityId);
}

bool ExpectedViewDelta::Compare(const ViewDelta& Other)
{
	return CompareDeltas(Other.GetEntityDeltas());
}

bool ExpectedViewDelta::Compare(const FSubViewDelta& Other)
{
	return CompareDeltas(Other.EntityDeltas);
}

bool ExpectedViewDelta::CompareDeltas(const TArray<EntityDelta>& Other)
{
	if (EntityDeltas.Num() != Other.Num())
	{
		return false;
	}

	SortEntityDeltas();
	TArray<uint32> DeltaKeys;
	EntityDeltas.GetKeys(DeltaKeys);
	for (int32 i = 0; i < DeltaKeys.Num(); ++i)
	{
		ExpectedEntityDelta& LhsEntityDelta = EntityDeltas[DeltaKeys[i]];
		EntityDelta RhsEntityDelta = Other[i];
		if (LhsEntityDelta.EntityId != RhsEntityDelta.EntityId)
		{
			return false;
		}

		switch (LhsEntityDelta.Type)
		{
		case ExpectedEntityDelta::UPDATE:
			if (!(RhsEntityDelta.Type == EntityDelta::UPDATE))
			{
				return false;
			}
			break;
		case ExpectedEntityDelta::ADD:
			if (!(RhsEntityDelta.Type == EntityDelta::ADD))
			{
				return false;
			}
			break;
		case ExpectedEntityDelta::REMOVE:
			if (!(RhsEntityDelta.Type == EntityDelta::REMOVE))
			{
				return false;
			}
			break;
		case ExpectedEntityDelta::TEMPORARILY_REMOVED:
			if (!(RhsEntityDelta.Type == EntityDelta::TEMPORARILY_REMOVED))
			{
				return false;
			}
			break;
		default:
			checkNoEntry();
		}

		if (!CompareData(LhsEntityDelta.AuthorityGained, RhsEntityDelta.AuthorityGained, CompareAuthorityChanges))
		{
			return false;
		}

		if (!CompareData(LhsEntityDelta.AuthorityLost, RhsEntityDelta.AuthorityLost, CompareAuthorityChanges))
		{
			return false;
		}

		if (!CompareData(LhsEntityDelta.AuthorityLostTemporarily, RhsEntityDelta.AuthorityLostTemporarily, CompareAuthorityChanges))
		{
			return false;
		}

		if (!CompareData(LhsEntityDelta.ComponentsAdded, RhsEntityDelta.ComponentsAdded, CompareComponentChanges))
		{
			return false;
		}

		if (!CompareData(LhsEntityDelta.ComponentsRemoved, RhsEntityDelta.ComponentsRemoved, CompareComponentChanges))
		{
			return false;
		}

		if (!CompareData(LhsEntityDelta.ComponentsRefreshed, RhsEntityDelta.ComponentsRefreshed, CompareComponentChanges))
		{
			return false;
		}

		if (!CompareData(LhsEntityDelta.ComponentUpdates, RhsEntityDelta.ComponentUpdates, CompareComponentChanges))
		{
			return false;
		}
	}

	return true;
}
