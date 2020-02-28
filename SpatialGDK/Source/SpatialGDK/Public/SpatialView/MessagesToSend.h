// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "CommandMessages.h"
#include "Containers/Array.h"

namespace SpatialGDK
{

// todo Placeholder for vertical slice. This should be revisited when we have more complicated messages.
struct MessagesToSend
{
	TArray<CreateEntityRequest> CreateEntityRequests;
};

}  // SpatialView
