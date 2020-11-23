// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/ViewDelta.h"

class SPATIALGDK_API SpatialOSWorkerInterface
{
public:
	virtual ~SpatialOSWorkerInterface() = default;

	// Worker Connection Interface
	virtual const TArray<SpatialGDK::EntityDelta>& GetEntityDeltas() = 0;
	virtual const TArray<Worker_Op>& GetWorkerMessages() = 0;
	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities) = 0;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const Worker_EntityId* EntityId,
													 const TOptional<Trace_SpanId>& SpanId = {}) = 0;
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId, const TOptional<Trace_SpanId>& SpanId = {}) = 0;
	virtual void SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData,
								  const TOptional<Trace_SpanId>& SpanId = {}) = 0;
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
									 const TOptional<Trace_SpanId>& SpanId = {}) = 0;
	virtual void SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate,
									 const TOptional<Trace_SpanId>& SpanId = {}) = 0;
	virtual Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request, uint32_t CommandId,
												const TOptional<Trace_SpanId>& SpanId) = 0;
	virtual void SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response,
									 const TOptional<Trace_SpanId>& SpanId = {}) = 0;
	virtual void SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const TOptional<Trace_SpanId>& SpanId = {}) = 0;
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) = 0;
	virtual Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery) = 0;
	virtual void SendMetrics(SpatialGDK::SpatialMetrics Metrics) = 0;
};
