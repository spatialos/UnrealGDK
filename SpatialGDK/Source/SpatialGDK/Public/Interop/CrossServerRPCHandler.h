// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "RPCExecutorInterface.h"
#include "SpatialView/ViewCoordinator.h"

namespace SpatialGDK
{
DECLARE_DELEGATE_RetVal_OneParam(bool, FProcessCrossServerRPC, const FCrossServerRPCParams&);
DECLARE_DELEGATE_RetVal_OneParam(FCrossServerRPCParams, FTryRetrieveCrossServerRPCParams, const Worker_CommandRequestOp&);

class CrossServerRPCHandler
{
public:
	CrossServerRPCHandler(ViewCoordinator& Coordinator, TUniquePtr<RPCExecutorInterface> RPCExecutor);

	void ProcessOps(const float TimeAdvancedS, const TArray<Worker_Op>& WorkerMessages);
	void ProcessPendingCommandOps();

private:
	ViewCoordinator& Coordinator;
	TUniquePtr<RPCExecutorInterface> RPCExecutor;

	double TimeElapsedS = 0.0;
	double CommandRetryTime = 0.0;
	TMap<Worker_EntityId_Key, TArray<FCrossServerRPCParams>> QueuedCrossServerRPCs;
	FProcessCrossServerRPC ProcessCrossServerRPCDelegate;
	FTryRetrieveCrossServerRPCParams TryRetrieveCrossServerRPCParamsDelegate;

	void HandleWorkerOp(const Worker_Op& Op);
	bool TryExecuteCommandRequest(const FCrossServerRPCParams& Params);
	void DropQueueForEntity(const Worker_EntityId_Key EntityId);
};
} // namespace SpatialGDK
