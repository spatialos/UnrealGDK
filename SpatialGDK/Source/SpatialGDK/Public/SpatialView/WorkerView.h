// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "MessagesToSend.h"
#include "ViewDelta.h"
#include "Templates/UniquePtr.h"
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

class WorkerView
{
public:
	WorkerView();

	// Process queued op lists to create a new view delta.
	// The view delta will exist until the next call to advance.
	const ViewDelta* GenerateViewDelta();

	// Add an OpList to generate the next ViewDelta.
	void EnqueueOpList(TUniquePtr<AbstractOpList> OpList);

	// Ensure all local changes have been applied and return the resulting MessagesToSend.
	TUniquePtr<MessagesToSend> FlushLocalChanges();

	void SendCreateEntityRequest(CreateEntityRequest Request);

private:
	void ProcessOp(const Worker_Op& Op);

	void HandleCreateEntityResponse(const Worker_CreateEntityResponseOp& Response);

	TArray<TUniquePtr<AbstractOpList>> QueuedOps;

	ViewDelta Delta;
	TUniquePtr<MessagesToSend> LocalChanges;
};

}  // namespace SpatialGDK
