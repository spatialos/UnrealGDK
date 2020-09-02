// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

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

ExpectedViewDelta& ExpectedViewDelta::AddDisconnect(const uint8_t StatusCode, const FString StatusMessage)
{
	ConnectionStatusCode = StatusCode;
	ConnectionStatusMessage = StatusMessage;
	return *this;
}

bool CompareComponentChange(ComponentChange Lhs, ComponentChange Rhs)
{
	return Lhs.ComponentId < Rhs.ComponentId;
}

bool CompareAuthorityChange(AuthorityChange Lhs, AuthorityChange Rhs)
{
	return Lhs.ComponentId < Rhs.ComponentId;
}

void ExpectedViewDelta::SortEntityDeltas()
{
	for (auto& Pair : EntityDeltas)
	{
		Pair.Value.AuthorityGained.Sort(CompareAuthorityChange);
		Pair.Value.AuthorityLost.Sort(CompareAuthorityChange);
		Pair.Value.AuthorityLostTemporarily.Sort(CompareAuthorityChange);
		Pair.Value.ComponentsAdded.Sort(CompareComponentChange);
		Pair.Value.ComponentsRemoved.Sort(CompareComponentChange);
		Pair.Value.ComponentsRefreshed.Sort(CompareComponentChange);
		Pair.Value.ComponentUpdates.Sort(CompareComponentChange);
	}

	EntityDeltas.KeySort(CompareWorkerEntityIdKey);
}

template <typename T, typename Predicate>
bool ExpectedViewDelta::Compare(const TArray<T>& Lhs, const ComponentSpan<T>& Rhs, Predicate&& Comparator)
{
	if (Lhs.Num() != Rhs.Num())
	{
		return false;
	}

	auto LhsFirst = Lhs.GetData();
	auto LhsLast = Lhs.GetData() + Lhs.Num();
	auto RhsFirst = Rhs.GetData();
	while (LhsFirst != LhsLast)
	{
		if (!Comparator(*LhsFirst, *RhsFirst))
		{
			return false;
		}
		++LhsFirst, ++RhsFirst;
	}

	return true;
}

bool ExpectedViewDelta::Compare(ViewDelta& Other)
{
	TArray<EntityDelta> RhsEntityDeltas = Other.GetEntityDeltas();
	if (EntityDeltas.Num() != RhsEntityDeltas.Num())
	{
		return false;
	}

	SortEntityDeltas();
	TArray<Worker_EntityId> DeltaKeys;
	EntityDeltas.GetKeys(DeltaKeys);
	for (int i = 0; i < DeltaKeys.Num(); i++)
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

		if (!Compare(LhsEntityDelta.AuthorityGained, RhsEntityDelta.AuthorityGained, CompareAuthorityChanges))
		{
			return false;
		}

		if (!Compare(LhsEntityDelta.AuthorityLost, RhsEntityDelta.AuthorityLost, CompareAuthorityChanges))
		{
			return false;
		}

		if (!Compare(LhsEntityDelta.AuthorityLostTemporarily, RhsEntityDelta.AuthorityLostTemporarily, CompareAuthorityChanges))
		{
			return false;
		}

		if (!Compare(LhsEntityDelta.ComponentsAdded, RhsEntityDelta.ComponentsAdded, CompareComponentChanges))
		{
			return false;
		}

		if (!Compare(LhsEntityDelta.ComponentsRemoved, RhsEntityDelta.ComponentsRemoved, CompareComponentChanges))
		{
			return false;
		}

		if (!Compare(LhsEntityDelta.ComponentsRefreshed, RhsEntityDelta.ComponentsRefreshed, CompareComponentChanges))
		{
			return false;
		}

		if (!Compare(LhsEntityDelta.ComponentUpdates, RhsEntityDelta.ComponentUpdates, CompareComponentChanges))
		{
			return false;
		}
	}

	return true;
}
