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
	virtual FRequestId SendReserveEntityIdsRequest(uint32_t NumOfEntities) override;
	virtual FRequestId SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const FEntityId* EntityId,
											   const TOptional<Trace_SpanId>& SpanId) override;
	virtual FRequestId SendDeleteEntityRequest(FEntityId EntityId, const TOptional<Trace_SpanId>& SpanId) override;
	virtual void SendAddComponent(FEntityId EntityId, FWorkerComponentData* ComponentData, const TOptional<Trace_SpanId>& SpanId) override;
	virtual void SendRemoveComponent(FEntityId EntityId, FComponentId ComponentId, const TOptional<Trace_SpanId>& SpanId) override;
	virtual void SendComponentUpdate(FEntityId EntityId, FWorkerComponentUpdate* ComponentUpdate,
									 const TOptional<Trace_SpanId>& SpanId) override;
	virtual FRequestId SendCommandRequest(FEntityId EntityId, Worker_CommandRequest* Request, uint32_t CommandId,
										  const TOptional<Trace_SpanId>& SpanId) override;
	virtual void SendCommandResponse(FRequestId RequestId, Worker_CommandResponse* Response,
									 const TOptional<Trace_SpanId>& SpanId) override;
	virtual void SendCommandFailure(FRequestId RequestId, const FString& Message, const TOptional<Trace_SpanId>& SpanId) override;
	virtual void SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) override;
	virtual void SendComponentInterest(FEntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest) override;
	virtual FRequestId SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery) override;
	virtual void SendMetrics(SpatialGDK::SpatialMetrics Metrics) override;

	// The following methods are used to query for state in tests.
	const Worker_EntityQuery* GetLastEntityQuery();
	void ClearLastEntityQuery();

	FRequestId GetLastRequestId();

private:
	FRequestId NextRequestId;

	const Worker_EntityQuery* LastEntityQuery;

	TArray<SpatialGDK::EntityDelta> PlaceholderEntityDeltas;
	TArray<Worker_Op> PlaceholderWorkerMessages;
};
