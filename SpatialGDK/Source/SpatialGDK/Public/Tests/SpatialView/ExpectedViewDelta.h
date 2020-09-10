// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "CoreMinimal.h"
#include "ExpectedEntityDelta.h"

#include "SpatialView/ViewDelta.h"

namespace SpatialGDK
{
class ExpectedViewDelta
{
public:
	enum EntityChangeType
	{
		UPDATE,
		ADD,
		REMOVE,
		TEMPORARILY_REMOVED
	};

	ExpectedViewDelta& AddEntityDelta(const Worker_EntityId EntityId, const EntityChangeType ChangeType);
	ExpectedViewDelta& AddComponentAdded(const Worker_EntityId EntityId, ComponentData Data);
	ExpectedViewDelta& AddComponentRemoved(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId);
	ExpectedViewDelta& AddComponentUpdate(const Worker_EntityId EntityId, ComponentUpdate Update);
	ExpectedViewDelta& AddComponentRefreshed(const Worker_EntityId EntityId, ComponentUpdate Update, ComponentData Data);
	ExpectedViewDelta& AddAuthorityGained(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId);
	ExpectedViewDelta& AddAuthorityLost(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId);
	ExpectedViewDelta& AddAuthorityLostTemporarily(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId);
	ExpectedViewDelta& AddDisconnect(const uint8 StatusCode, FString StatusMessage);

	// Compares the stored Entity Deltas
	bool Compare(const ViewDelta& Other);
	bool Compare(const FSubViewDelta& Other);

private:
	void SortEntityDeltas();
	TMap<uint32, ExpectedEntityDelta> EntityDeltas;
	uint8 ConnectionStatusCode = 0;
	FString ConnectionStatusMessage;

	bool CompareDeltas(const TArray<EntityDelta>& Other);

	template <typename T, typename Predicate>
	bool CompareData(const TArray<T>& Lhs, const ComponentSpan<T>& Rhs, Predicate&& Comparator)
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
};
} // namespace SpatialGDK
