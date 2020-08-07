// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/OpList/OpList.h"
#include "Utils/SpatialLatencyTracer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

class SPATIALGDK_API SpatialOSWorkerInterface
{
public:
	// 	FORCEINLINE bool IsConnected() { return bIsConnected; }

	// Worker Connection Interface
	virtual TArray<SpatialGDK::OpList> GetOpList()
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::GetOpList, return TArray<SpatialGDK::OpList>(););
	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendReserveEntityIdsRequest, return 0;);
	virtual Worker_RequestId SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const Worker_EntityId* EntityId)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendCreateEntityRequest, return 0;);
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendDeleteEntityRequest, return 0;);
	virtual void SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendAddComponent, return;);
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendRemoveComponent, return;);
	virtual void SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendComponentUpdate, return;);
	virtual Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request, uint32_t CommandId)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendCommandRequest, return 0;);
	virtual void SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendCommandResponse, return;);
	virtual void SendCommandFailure(Worker_RequestId RequestId, const FString& Message)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendCommandFailure, return;);
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendLogMessage, return;);
	virtual void SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendEntityQueryRequest, return;);
	virtual Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
		PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendEntityQueryRequest, return 0;);
	virtual void SendMetrics(SpatialGDK::SpatialMetrics Metrics) PURE_VIRTUAL(AbstractSpatialWorkerConnection::SendMetrics, return;);
};
