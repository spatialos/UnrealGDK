// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialOSWorkerInterface.h"

#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/ViewDelta.h"
#include "Utils/SpatialLatencyTracer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

// The SpatialOSWorkerConnectionSpy is intended to be a very minimal implementation of a WorkerConnection which just records then swallows
// any data sent through it. It is intended to be extended with methods to query for what data has been sent through it.
//
// Only a few methods have meaningful implementations. You are intended to extend the implementations whenever needed
// for testing code which uses the WorkerConnection.

class SpatialOSWorkerConnectionSpy : public SpatialOSWorkerInterface
{
public:
	SpatialOSWorkerConnectionSpy();

	virtual const TArray<SpatialGDK::EntityDelta>& GetEntityDeltas() override;
	virtual const TArray<Worker_Op>& GetWorkerMessages() override;
	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities, const SpatialGDK::FRetryData& RetryData) override;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const Worker_EntityId* EntityId,
													 const SpatialGDK::FRetryData& RetryData, const FSpatialGDKSpanId& SpanId) override;
	virtual Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId, const SpatialGDK::FRetryData& RetryData,
													 const FSpatialGDKSpanId& SpanId) override;
	virtual void SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData, const FSpatialGDKSpanId& SpanId) override;
	virtual void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const FSpatialGDKSpanId& SpanId) override;
	virtual void SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate,
									 const FSpatialGDKSpanId& SpanId) override;
	virtual Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
												const SpatialGDK::FRetryData& RetryData, const FSpatialGDKSpanId& SpanId) override;
	virtual void SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response,
									 const FSpatialGDKSpanId& SpanId) override;
	virtual void SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const FSpatialGDKSpanId& SpanId) override;
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) override;
	virtual Worker_RequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery,
													const SpatialGDK::FRetryData& RetryData) override;
	virtual void SendMetrics(SpatialGDK::SpatialMetrics Metrics) override;

	// The following methods are used to query for state in tests.
	const Worker_EntityQuery* GetLastEntityQuery();
	void ClearLastEntityQuery();

	Worker_RequestId GetLastRequestId();

private:
	Worker_RequestId NextRequestId;

	const Worker_EntityQuery* LastEntityQuery;

	TArray<SpatialGDK::EntityDelta> PlaceholderEntityDeltas;
	TArray<Worker_Op> PlaceholderWorkerMessages;
};
