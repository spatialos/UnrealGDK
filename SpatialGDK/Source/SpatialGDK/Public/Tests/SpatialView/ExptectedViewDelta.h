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
		ADD,
		REMOVE,
		UPDATE
	};

	ExpectedViewDelta& AddEntityDelta(Worker_EntityId EntityId, EntityChangeType Status);
	ExpectedViewDelta& AddComponentAdded(Worker_EntityId EntityId, ComponentData ComponentData);
	ExpectedViewDelta& AddComponentRemoved(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	ExpectedViewDelta& AddComponentUpdate(Worker_EntityId EntityId, ComponentUpdate ComponentUpdate);
	ExpectedViewDelta& AddComponentRefreshed(Worker_EntityId EntityId, ComponentUpdate ComponentUpdate, ComponentData ComponentData);
	ExpectedViewDelta& AddAuthorityGained(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	ExpectedViewDelta& AddAuthorityLost(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	ExpectedViewDelta& AddAuthorityLostTemporarily(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	ExpectedViewDelta& AddDisconnect(uint8_t StatusCode, FString StatusMessage);

	bool Compare(ViewDelta& Other);

private:
	TArray<ExpectedEntityDelta> GetSortedEntityDeltas();
	TMap<Worker_EntityId, ExpectedEntityDelta> EntityDeltas;
	uint8 ConnectionStatusCode = 0;
	FString ConnectionStatusMessage;

	template <typename T, typename Predicate>
	bool Compare(const TArray<T>& Lhs, const ComponentSpan<T>& Rhs, Predicate&& Compare);
};
} // namespace SpatialGDK
