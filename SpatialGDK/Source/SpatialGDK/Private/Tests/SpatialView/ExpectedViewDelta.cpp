// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ComponentTestUtils.h"
#include "SpatialView/ViewDelta.h"
#include "Tests/SpatialView/ExpectedEntityDelta.h"
#include "Tests/SpatialView/ExptectedViewDelta.h"

using namespace SpatialGDK;

ExpectedViewDelta& ExpectedViewDelta::AddEntityDelta(Worker_EntityId EntityId, EntityChangeType Status)
{
	EntityDeltas.Add(EntityId, { EntityId, Status == ADD, Status == REMOVE });
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddComponentAdded(Worker_EntityId EntityId, ComponentData ComponentData)
{
	EntityDeltas[EntityId].ComponentsAdded.Push(ComponentChange(ComponentData.GetComponentId(), ComponentData.GetUnderlying()));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddComponentRemoved(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	EntityDeltas[EntityId].ComponentsRemoved.Push(ComponentChange(ComponentId));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddComponentUpdate(Worker_EntityId EntityId, ComponentUpdate ComponentUpdate)
{
	EntityDeltas[EntityId].ComponentUpdates.Push(ComponentChange(ComponentUpdate.GetComponentId(), ComponentUpdate.GetUnderlying()));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddComponentRefreshed(Worker_EntityId EntityId, ComponentUpdate ComponentUpdate,
															ComponentData ComponentData)
{
	EntityDeltas[EntityId].ComponentUpdates.Push(
		ComponentChange(ComponentUpdate.GetComponentId(), ComponentData.GetUnderlying(), ComponentUpdate.GetEvents()));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddAuthorityGained(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	EntityDeltas[EntityId].AuthorityGained.Add(AuthorityChange(ComponentId, AuthorityChange::AUTHORITY_GAINED));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddAuthorityLost(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	EntityDeltas[EntityId].AuthorityLost.Add(AuthorityChange(ComponentId, AuthorityChange::AUTHORITY_LOST));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddAuthorityLostTemporarily(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	EntityDeltas[EntityId].AuthorityLostTemporarily.Add(AuthorityChange(ComponentId, AuthorityChange::AUTHORITY_LOST_TEMPORARILY));
	return *this;
}

ExpectedViewDelta& ExpectedViewDelta::AddDisconnect(uint8_t StatusCode, FString StatusMessage)
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

bool CompareExpectedEntityDelta(ExpectedEntityDelta Lhs, ExpectedEntityDelta Rhs)
{
	return Lhs.EntityId < Rhs.EntityId;
}

TArray<ExpectedEntityDelta> ExpectedViewDelta::GetSortedEntityDeltas()
{
	TArray<ExpectedEntityDelta> SortedDeltas;

	for (const auto& Pair : EntityDeltas)
	{
		ExpectedEntityDelta Delta = Pair.Value;
		Delta.AuthorityGained.Sort(CompareAuthorityChange);
		Delta.AuthorityLost.Sort(CompareAuthorityChange);
		Delta.AuthorityLostTemporarily.Sort(CompareAuthorityChange);
		Delta.ComponentsAdded.Sort(CompareComponentChange);
		Delta.ComponentsRemoved.Sort(CompareComponentChange);
		Delta.ComponentsRefreshed.Sort(CompareComponentChange);
		Delta.ComponentUpdates.Sort(CompareComponentChange);
		SortedDeltas.Push(Delta);
	}

	SortedDeltas.Sort(CompareExpectedEntityDelta);
	return SortedDeltas;
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

	TArray<ExpectedEntityDelta> SortedDeltas = GetSortedEntityDeltas();

	for (int i = 0; i < SortedDeltas.Num(); i++)
	{
		ExpectedEntityDelta LhsEntityDelta = SortedDeltas[i];
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
