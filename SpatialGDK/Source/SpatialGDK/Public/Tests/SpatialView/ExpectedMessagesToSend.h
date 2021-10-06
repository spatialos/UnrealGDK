// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ComponentTestUtils.h"
#include "SpatialView/MessagesToSend.h"
#include "SpatialView/OutgoingMessages.h"

namespace SpatialGDK
{
class ExpectedMessagesToSend
{
public:
	ExpectedMessagesToSend& AddCreateEntityRequest(Worker_RequestId RequestId, Worker_EntityId EntityId,
												   TArray<ComponentData> ComponentData);
	ExpectedMessagesToSend& AddEntityCommandRequest(Worker_RequestId RequestId, Worker_EntityId EntityId, Worker_ComponentId ComponentId,
													Worker_CommandIndex CommandIndex);
	ExpectedMessagesToSend& AddDeleteEntityCommandRequest(Worker_RequestId RequestId, Worker_EntityId EntityId);
	ExpectedMessagesToSend& AddReserveEntityIdsRequest(Worker_RequestId RequestId, uint32 NumOfEntities);
	ExpectedMessagesToSend& AddEntityQueryRequest(Worker_RequestId RequestId, EntityQuery Query);
	ExpectedMessagesToSend& AddEntityCommandResponse(Worker_RequestId RequestId, Worker_ComponentId ComponentId,
													 Worker_CommandIndex CommandIndex);
	ExpectedMessagesToSend& AddEntityCommandFailure(Worker_RequestId RequestId, FString Message);
	bool Compare(const MessagesToSend& MessagesToSend) const;

private:
	TArray<ReserveEntityIdsRequest> ReserveEntityIdsRequests;
	TArray<CreateEntityRequest> CreateEntityRequests;
	TArray<DeleteEntityRequest> DeleteEntityRequests;
	TArray<EntityQueryRequest> EntityQueryRequests;
	TArray<EntityCommandRequest> EntityCommandRequests;
	TArray<EntityCommandResponse> EntityCommandResponses;
	TArray<EntityCommandFailure> EntityCommandFailures;
};

} // namespace SpatialGDK
