// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/SpatialView/ExpectedMessagesToSend.h"
#include "Tests/SpatialView/CommandTestUtils.h"

using namespace SpatialGDK;

ExpectedMessagesToSend& ExpectedMessagesToSend::AddCreateEntityRequest(Worker_RequestId RequestId, Worker_EntityId EntityId,
																	   Worker_ComponentId ComponentId, double ComponentValue)
{
	CreateEntityRequest TestCreateEntityRequest;
	TestCreateEntityRequest.RequestId = RequestId;
	TestCreateEntityRequest.EntityId = EntityId;
	TestCreateEntityRequest.EntityComponents.Add(CreateTestComponentData(ComponentId, ComponentValue));
	TestCreateEntityRequest.TimeoutMillis = 0;
	CreateEntityRequests.Push(MoveTemp(TestCreateEntityRequest));
	return *this;
}

ExpectedMessagesToSend& ExpectedMessagesToSend::AddEntityCommandRequest(Worker_RequestId RequestId, Worker_EntityId EntityId)
{
	EntityCommandRequests.Push({ EntityId, RequestId, CommandRequest(1, 1) });
	return *this;
}

ExpectedMessagesToSend& ExpectedMessagesToSend::AddDeleteEntityCommandRequest(Worker_RequestId RequestId, Worker_EntityId EntityId)
{
	DeleteEntityRequests.Push({ RequestId, EntityId });
	return *this;
}

ExpectedMessagesToSend& ExpectedMessagesToSend::AddReserveEntityIdsRequest(Worker_RequestId RequestId, uint32 NumOfEntities)
{
	ReserveEntityIdsRequests.Push({ RequestId, NumOfEntities });
	return *this;
}

ExpectedMessagesToSend& ExpectedMessagesToSend::AddEntityQueryRequest(Worker_RequestId RequestId)
{
	// TODO;
	return *this;
}

ExpectedMessagesToSend& ExpectedMessagesToSend::AddEntityCommandResponse(Worker_RequestId RequestId, Worker_ComponentId ComponentId,
																		 Worker_CommandIndex CommandIndex)
{
	EntityCommandResponse Response{ RequestId, CommandResponse(ComponentId, CommandIndex) };
	EntityCommandResponses.Push(MoveTemp(Response));
	return *this;
}

ExpectedMessagesToSend& ExpectedMessagesToSend::AddEntityCommandFailure(Worker_RequestId RequestId, FString Message)
{
	EntityCommandFailures.Push({ RequestId, Message });
	return *this;
}

bool ExpectedMessagesToSend::Compare(const TUniquePtr<MessagesToSend> MessagesToSend) const
{
	if (MessagesToSend == nullptr)
	{
		return false;
	}

	if (!AreEquivalent(ReserveEntityIdsRequests, MessagesToSend->ReserveEntityIdsRequests, CompareReseverEntityIdsRequests))
	{
		return false;
	}

	if (!AreEquivalent(CreateEntityRequests, MessagesToSend->CreateEntityRequests, CompareCreateEntityRequests))
	{
		return false;
	}

	if (!AreEquivalent(DeleteEntityRequests, MessagesToSend->DeleteEntityRequests, CompareDeleteEntityRequests))
	{
		return false;
	}

	if (!AreEquivalent(EntityQueryRequests, MessagesToSend->EntityQueryRequests, CompareEntityQueryRequests))
	{
		return false;
	}

	if (!AreEquivalent(EntityCommandRequests, MessagesToSend->EntityCommandRequests, CompareEntityCommandRequests))
	{
		return false;
	}

	if (!AreEquivalent(EntityCommandResponses, MessagesToSend->EntityCommandResponses, CompareEntityCommandResponses))
	{
		return false;
	}

	return AreEquivalent(EntityCommandFailures, MessagesToSend->EntityCommandFailures, CompareEntityCommandFailuers);
}
