// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "HAL/Platform.h"
#include "Misc/Optional.h"
#include "Templates/UnrealTemplate.h"
#include "Templates/UniquePtr.h"
#include "UObject/NameTypes.h"

#include <string>

#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

enum class EOutgoingMessageType : int32
{
	ReserveEntityIdsRequest,
	CreateEntityRequest,
	DeleteEntityRequest,
	ComponentUpdate,
	CommandRequest,
	CommandResponse,
	CommandFailure,
	LogMessage,
	ComponentInterest,
	EntityQueryRequest,
	Metrics
};

struct FOutgoingMessage
{
	FOutgoingMessage(const EOutgoingMessageType& InType) : Type(InType) {}
	virtual ~FOutgoingMessage() {}

	EOutgoingMessageType Type;
};

struct FReserveEntityIdsRequest : FOutgoingMessage
{
	FReserveEntityIdsRequest(uint32_t InNumOfEntities)
		: FOutgoingMessage(EOutgoingMessageType::ReserveEntityIdsRequest)
		, NumOfEntities(InNumOfEntities)
	{}

	uint32_t NumOfEntities;
};

struct FCreateEntityRequest : FOutgoingMessage
{
	FCreateEntityRequest(TArray<Worker_ComponentData>&& InComponents, const Worker_EntityId* InEntityId)
		: FOutgoingMessage(EOutgoingMessageType::CreateEntityRequest)
		, Components(MoveTemp(InComponents))
		, EntityId(InEntityId != nullptr ? *InEntityId : TOptional<Worker_EntityId>())
	{}

	TArray<Worker_ComponentData> Components;
	TOptional<Worker_EntityId> EntityId;
};

struct FDeleteEntityRequest : FOutgoingMessage
{
	FDeleteEntityRequest(Worker_EntityId InEntityId)
		: FOutgoingMessage(EOutgoingMessageType::DeleteEntityRequest)
		, EntityId(InEntityId)
	{}

	Worker_EntityId EntityId;
};

struct FComponentUpdate : FOutgoingMessage
{
	FComponentUpdate(Worker_EntityId InEntityId, const Worker_ComponentUpdate& InComponentUpdate)
		: FOutgoingMessage(EOutgoingMessageType::ComponentUpdate)
		, EntityId(InEntityId)
		, Update(InComponentUpdate)
	{}

	Worker_EntityId EntityId;
	Worker_ComponentUpdate Update;
};

struct FCommandRequest : FOutgoingMessage
{
	FCommandRequest(Worker_EntityId InEntityId, const Worker_CommandRequest& InRequest, uint32_t InCommandId)
		: FOutgoingMessage(EOutgoingMessageType::CommandRequest)
		, EntityId(InEntityId)
		, Request(InRequest)
		, CommandId(InCommandId)
	{}

	Worker_EntityId EntityId;
	Worker_CommandRequest Request;
	uint32_t CommandId;
};

struct FCommandResponse : FOutgoingMessage
{
	FCommandResponse(Worker_RequestId InRequestId, const Worker_CommandResponse& InResponse)
		: FOutgoingMessage(EOutgoingMessageType::CommandResponse)
		, RequestId(InRequestId)
		, Response(InResponse)
	{}

	Worker_RequestId RequestId;
	Worker_CommandResponse Response;
};

struct FCommandFailure : FOutgoingMessage
{
	FCommandFailure(Worker_RequestId InRequestId, const FString& InMessage)
		: FOutgoingMessage(EOutgoingMessageType::CommandFailure)
		, RequestId(InRequestId)
		, Message(InMessage)
	{}

	Worker_RequestId RequestId;
	FString Message;
};

struct FLogMessage : FOutgoingMessage
{
	FLogMessage(uint8_t InLevel, const FName& InLoggerName, const FString& InMessage)
		: FOutgoingMessage(EOutgoingMessageType::LogMessage)
		, Level(InLevel)
		, LoggerName(InLoggerName)
		, Message(InMessage)
	{}

	uint8_t Level;
	FName LoggerName;
	FString Message;
};

struct FComponentInterest : FOutgoingMessage
{
	FComponentInterest(Worker_EntityId InEntityId, TArray<Worker_InterestOverride>&& InInterests)
		: FOutgoingMessage(EOutgoingMessageType::ComponentInterest)
		, EntityId(InEntityId)
		, Interests(MoveTemp(InInterests))
	{}

	Worker_EntityId EntityId;
	TArray<Worker_InterestOverride> Interests;
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
			FMemory::Memcpy(static_cast<void*>(ComponentIdStorage.GetData()), static_cast<const void*>(EntityQuery.snapshot_result_type_component_ids), ComponentIdStorage.Num());
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
};

struct FMetrics : FOutgoingMessage
{
	FMetrics(const SpatialMetrics& InMetrics)
		: FOutgoingMessage(EOutgoingMessageType::Metrics)
		, Metrics(InMetrics)
	{}

	SpatialMetrics Metrics;
};

}
