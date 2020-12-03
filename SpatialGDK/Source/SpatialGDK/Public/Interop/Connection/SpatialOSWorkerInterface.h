// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/CommandRetryHandler.h"
#include "SpatialView/ViewDelta.h"

class SPATIALGDK_API SpatialOSWorkerInterface
{
public:
	virtual ~SpatialOSWorkerInterface() = default;

	// Worker Connection Interface
	virtual const TArray<SpatialGDK::EntityDelta>& GetEntityDeltas() = 0;
	virtual const TArray<Worker_Op>& GetWorkerMessages() = 0;
	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities, const SpatialGDK::FRetryData& Data) = 0;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const Worker_EntityId* EntityId,
													 const SpatialGDK::FRetryData& RetryData, const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId, const SpatialGDK::FRetryData& RetryData,
													 const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual void SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData, const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual void SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate,
									 const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
												const SpatialGDK::FRetryData& RetryData, const FSpatialGDKSpanId& SpanId) = 0;
	virtual void SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response,
									 const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual void SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) = 0;
	virtual Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery, const SpatialGDK::FRetryData& RetryData) = 0;
	virtual void SendMetrics(SpatialGDK::SpatialMetrics Metrics) = 0;
};
