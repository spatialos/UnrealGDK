// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "HAL/Platform.h"
#include "Misc/Optional.h"
#include "Templates/UniquePtr.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/NameTypes.h"
#include "Utils/SpatialLatencyTracer.h"

#include <string>

#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
enum class EOutgoingMessageType : int32
{
	ReserveEntityIdsRequest,
	CreateEntityRequest,
	DeleteEntityRequest,
	AddComponent,
	RemoveComponent,
	ComponentUpdate,
	CommandRequest,
	CommandResponse,
	CommandFailure,
	LogMessage,
	EntityQueryRequest,
	Metrics
};

struct FOutgoingMessage
{
	FOutgoingMessage(const EOutgoingMessageType& InType)
		: Type(InType)
	{
	}
	virtual ~FOutgoingMessage() {}

	EOutgoingMessageType Type;
};

struct FReserveEntityIdsRequest : FOutgoingMessage
{
	FReserveEntityIdsRequest(uint32_t InNumOfEntities)
		: FOutgoingMessage(EOutgoingMessageType::ReserveEntityIdsRequest)
		, NumOfEntities(InNumOfEntities)
	{
	}

	uint32_t NumOfEntities;
};

struct FCreateEntityRequest : FOutgoingMessage
{
	FCreateEntityRequest(TArray<FWorkerComponentData>&& InComponents, const Worker_EntityId* InEntityId,
						 const TOptional<Trace_SpanId>& SpanId)
		: FOutgoingMessage(EOutgoingMessageType::CreateEntityRequest)
		, Components(MoveTemp(InComponents))
		, EntityId(InEntityId != nullptr ? *InEntityId : TOptional<Worker_EntityId>())
		, SpanId(SpanId)
	{
	}

	TArray<FWorkerComponentData> Components;
	TOptional<Worker_EntityId> EntityId;
	TOptional<Trace_SpanId> SpanId;
};

struct FDeleteEntityRequest : FOutgoingMessage
{
	FDeleteEntityRequest(Worker_EntityId InEntityId, const TOptional<Trace_SpanId>& SpanId)
		: FOutgoingMessage(EOutgoingMessageType::DeleteEntityRequest)
		, EntityId(InEntityId)
		, SpanId(SpanId)
	{
	}

	Worker_EntityId EntityId;
	const TOptional<Trace_SpanId> SpanId;
};

struct FAddComponent : FOutgoingMessage
{
	FAddComponent(Worker_EntityId InEntityId, const FWorkerComponentData& InData, const TOptional<Trace_SpanId>& SpanId)
		: FOutgoingMessage(EOutgoingMessageType::AddComponent)
		, EntityId(InEntityId)
		, Data(InData)
		, SpanId(SpanId)
	{
	}

	Worker_EntityId EntityId;
	FWorkerComponentData Data;
	TOptional<Trace_SpanId> SpanId;
};

struct FRemoveComponent : FOutgoingMessage
{
	FRemoveComponent(Worker_EntityId InEntityId, Worker_ComponentId InComponentId, const TOptional<Trace_SpanId>& SpanId)
		: FOutgoingMessage(EOutgoingMessageType::RemoveComponent)
		, EntityId(InEntityId)
		, ComponentId(InComponentId)
		, SpanId(SpanId)
	{
	}

	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;
	TOptional<Trace_SpanId> SpanId;
};

struct FComponentUpdate : FOutgoingMessage
{
	FComponentUpdate(Worker_EntityId InEntityId, const FWorkerComponentUpdate& InComponentUpdate, const TOptional<Trace_SpanId>& SpanId)
		: FOutgoingMessage(EOutgoingMessageType::ComponentUpdate)
		, EntityId(InEntityId)
		, Update(InComponentUpdate)
		, SpanId(SpanId)
	{
	}

	Worker_EntityId EntityId;
	FWorkerComponentUpdate Update;
	TOptional<Trace_SpanId> SpanId;
};

struct FCommandRequest : FOutgoingMessage
{
	FCommandRequest(Worker_EntityId InEntityId, const Worker_CommandRequest& InRequest, uint32_t InCommandId)
		: FOutgoingMessage(EOutgoingMessageType::CommandRequest)
		, EntityId(InEntityId)
		, Request(InRequest)
		, CommandId(InCommandId)
	{
	}

	Worker_EntityId EntityId;
	Worker_CommandRequest Request;
	uint32_t CommandId;
};

struct FCommandResponse : FOutgoingMessage
{
	FCommandResponse(Worker_RequestId InRequestId, const Worker_CommandResponse& InResponse, const TOptional<Trace_SpanId>& SpanId)
		: FOutgoingMessage(EOutgoingMessageType::CommandResponse)
		, RequestId(InRequestId)
		, Response(InResponse)
		, SpanId(SpanId)
	{
	}

	Worker_RequestId RequestId;
	Worker_CommandResponse Response;
	TOptional<Trace_SpanId> SpanId;
};

struct FCommandFailure : FOutgoingMessage
{
	FCommandFailure(Worker_RequestId InRequestId, const FString& InMessage, const TOptional<Trace_SpanId>& SpanId)
		: FOutgoingMessage(EOutgoingMessageType::CommandFailure)
		, RequestId(InRequestId)
		, Message(InMessage)
		, SpanId(SpanId)
	{
	}

	Worker_RequestId RequestId;
	FString Message;
	TOptional<Trace_SpanId> SpanId;
};

struct FLogMessage : FOutgoingMessage
{
	FLogMessage(uint8_t InLevel, const FName& InLoggerName, const FString& InMessage)
		: FOutgoingMessage(EOutgoingMessageType::LogMessage)
		, Level(InLevel)
		, LoggerName(InLoggerName)
		, Message(InMessage)
	{
	}

	uint8_t Level;
	FName LoggerName;
	FString Message;
};

struct FEntityQueryRequest : FOutgoingMessage
{
	FEntityQueryRequest(const Worker_EntityQuery& InEntityQuery)
		: FOutgoingMessage(EOutgoingMessageType::EntityQueryRequest)
		, EntityQuery(InEntityQuery)
	{
		if (EntityQuery.snapshot_result_type_component_ids != nullptr)
		{
			ComponentIdStorage.SetNum(EntityQuery.snapshot_result_type_component_id_count);
			FMemory::Memcpy(static_cast<void*>(ComponentIdStorage.GetData()),
							static_cast<const void*>(EntityQuery.snapshot_result_type_component_ids), ComponentIdStorage.Num());
		}

		TraverseConstraint(&EntityQuery.constraint);
	}

	void TraverseConstraint(Worker_Constraint* Constraint);

	Worker_EntityQuery EntityQuery;
	TArray<TUniquePtr<Worker_Constraint[]>> ConstraintStorage;
	TArray<Worker_ComponentId> ComponentIdStorage;
};

/** Parameters for a gauge metric. */
struct GaugeMetric
{
	/* The name of the metric. */
	std::string Key;
	/* The current value of the metric. */
	double Value;
};

/* Parameters for a histogram metric bucket. */
struct HistogramMetricBucket
{
	/* The upper bound. */
	double UpperBound;
	/* The number of observations that were less than or equal to the upper bound. */
	uint32 Samples;
};

/* Parameters for a histogram metric. */
struct HistogramMetric
{
	/* The name of the metric. */
	std::string Key;
	/* The sum of all observations. */
	double Sum;
	/* Array of buckets. */
	TArray<HistogramMetricBucket> Buckets;
};

/** Parameters for sending metrics to SpatialOS. */
struct SpatialMetrics
{
	/** The load value of this worker. If NULL, do not report load. */
	TOptional<double> Load;
	/** Array of gauge metrics. */
	TArray<GaugeMetric> GaugeMetrics;
	/** Array of histogram metrics. */
	TArray<HistogramMetric> HistogramMetrics;

	void SendToConnection(Worker_Connection* Connection);
};

struct FMetrics : FOutgoingMessage
{
	FMetrics(SpatialMetrics InMetrics)
		: FOutgoingMessage(EOutgoingMessageType::Metrics)
		, Metrics(MoveTemp(InMetrics))
	{
	}

	SpatialMetrics Metrics;
};

} // namespace SpatialGDK
