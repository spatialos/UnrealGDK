// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/OutgoingComponentMessage.h"
#include "SpatialView/CommandMessages.h"
#include "Containers/Array.h"

namespace SpatialGDK
{

struct MessagesToSend
{
	TArray<CreateEntityRequest> CreateEntityRequests;
	TArray<OutgoingComponentMessage> ComponentMessages;
};

}  // namespace SpatialGDK
