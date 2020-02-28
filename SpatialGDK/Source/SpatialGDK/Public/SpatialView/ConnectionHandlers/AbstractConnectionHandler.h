// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "MessagesToSend.h"
#include "OpList/AbstractOpList.h"
#include "Templates/UniquePtr.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{

class AbstractConnectionHandler
{
public:
	virtual ~AbstractConnectionHandler() = default;

	// Should be called to indicate a new tick has started.
	// Ensures all external messages, up to this, point have been received.
	virtual void Advance() = 0;

	// The number of OpList instances queued.
	virtual uint32 GetOpListCount() = 0;

	// Gets the next queued OpList. If there is no OpList queued then an empty one is returned.
	virtual TUniquePtr<AbstractOpList> GetNextOpList() = 0;

	// Consumes messages and sends them to the deployment.
	virtual void SendMessages(TUniquePtr<MessagesToSend> Messages) = 0;

	// todo implement this once spatial view can be used without the legacy worker connection.
	// Return the unique ID for the worker.
	// virtual const FString& GetWorkerId() const = 0;

	// todo implement this once spatial view can be used without the legacy worker connection.
	// Returns the attributes for the worker.
	// virtual const TArray<FString>& GetWorkerAttributes() const = 0;
};

}  // namespace SpatialGDK
