// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Interop/Connection/SpatialGDKSpanId.h"
#include "SpatialView/CommandRequest.h"
#include "SpatialView/CommandResponse.h"
#include "SpatialView/CommandRetryHandler.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"
#include "SpatialView/EntityQuery.h"
#include "SpatialView/ViewDelta.h"

namespace SpatialGDK
{
class SPATIALGDK_API ISpatialOSWorker
{
public:
	virtual ~ISpatialOSWorker() = default;

	virtual const TArray<EntityDelta>& GetEntityDeltas() const = 0;
	virtual const TArray<Worker_Op>& GetWorkerMessages() const = 0;
	virtual const EntityView& GetView() const = 0;

	virtual const FString& GetWorkerId() const = 0;
	virtual FSpatialEntityId GetWorkerSystemEntityId() const = 0;

	virtual const ComponentData* GetComponent(FSpatialEntityId EntityId, Worker_ComponentId ComponentId) const = 0;

	virtual void SendAddComponent(FSpatialEntityId EntityId, ComponentData Data, const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual void SendComponentUpdate(FSpatialEntityId EntityId, ComponentUpdate Update, const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual void SendRemoveComponent(FSpatialEntityId EntityId, Worker_ComponentId ComponentId, const FSpatialGDKSpanId& SpanId = {}) = 0;

	virtual Worker_RequestId SendEntityCommandRequest(FSpatialEntityId EntityId, CommandRequest Request, FRetryData RetryData = NO_RETRIES,
													  const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual void SendEntityCommandResponse(Worker_RequestId RequestId, CommandResponse Response, const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual void SendEntityCommandFailure(Worker_RequestId RequestId, FString Message, const FSpatialGDKSpanId& SpanId = {}) = 0;

	virtual Worker_RequestId SendReserveEntityIdsRequest(uint32 NumberOfEntityIds, FRetryData RetryData = NO_RETRIES) = 0;
	virtual Worker_RequestId SendCreateEntityRequest(TArray<ComponentData> EntityComponents, TOptional<FSpatialEntityId> EntityId,
													 FRetryData RetryData = NO_RETRIES, const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual Worker_RequestId SendDeleteEntityRequest(FSpatialEntityId EntityId, FRetryData RetryData = NO_RETRIES,
													 const FSpatialGDKSpanId& SpanId = {}) = 0;
	virtual Worker_RequestId SendEntityQueryRequest(EntityQuery Query, FRetryData RetryData = NO_RETRIES) = 0;

	virtual void SendMetrics(SpatialMetrics Metrics) = 0;
	virtual void SendLogMessage(Worker_LogLevel Level, const FName& LoggerName, FString Message) = 0;

	virtual bool HasEntity(FSpatialEntityId EntityId) const = 0;
	virtual bool HasComponent(FSpatialEntityId EntityId, Worker_ComponentId ComponentId) const = 0;
	virtual bool HasAuthority(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId) const = 0;
};
} // namespace SpatialGDK
