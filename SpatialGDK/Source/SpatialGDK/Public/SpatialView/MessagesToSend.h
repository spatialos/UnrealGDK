// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "OutgoingMessages.h"
#include "SpatialView/OutgoingComponentMessage.h"
#include "SpatialView/CommandMessages.h"
#include "Containers/Array.h"

namespace SpatialGDK
{

struct MessagesToSend
{
	TArray<OutgoingComponentMessage> ComponentMessages;
	TArray<ReserveEntityIdsRequest> ReserveEntityIdsRequests;
	TArray<CreateEntityRequest> CreateEntityRequests;
	TArray<DeleteEntityRequest> DeleteEntityRequests;
	TArray<EntityQueryRequest> EntityQueryRequests;
	TArray<EntityCommandRequest> EntityCommandRequests;
	TArray<EntityCommandResponse> EntityCommandResponses;
	TArray<EntityCommandFailure> EntityCommandFailures;
	// todo should this be the metrics type from the cpp-gdk repo.
	TArray<SpatialMetrics> Metrics;
};

}  // namespace SpatialGDK
