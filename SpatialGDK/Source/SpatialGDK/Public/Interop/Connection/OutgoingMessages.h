// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include <WorkerSDK/improbable/c_worker.h>

enum class EOutgoingMessageType : int32
{
	Invalid,
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

	const EOutgoingMessageType Type;
};

struct FReserveEntityIdsRequest : FOutgoingMessage
{
	FReserveEntityIdsRequest(const uint32_t& InNumOfEntities)
		: FOutgoingMessage(EOutgoingMessageType::ReserveEntityIdsRequest)
		, NumOfEntities(InNumOfEntities)
	{}

	const uint32_t NumOfEntities;
};

struct FCreateEntityRequest : FOutgoingMessage
{
	FCreateEntityRequest(TArray<Worker_ComponentData>&& InComponents, const Worker_EntityId* InEntityId)
		: FOutgoingMessage(EOutgoingMessageType::CreateEntityRequest)
		, Components(InComponents)
		, EntityId(InEntityId != nullptr ? *InEntityId : TOptional<Worker_EntityId>())
	{}

	const TArray<Worker_ComponentData> Components;
	const TOptional<Worker_EntityId> EntityId;
};

struct FDeleteEntityRequest : FOutgoingMessage
{
	FDeleteEntityRequest(const Worker_EntityId& InEntityId)
		: FOutgoingMessage(EOutgoingMessageType::DeleteEntityRequest)
		, EntityId(InEntityId)
	{}

	const Worker_EntityId EntityId;
};

struct FComponentUpdate : FOutgoingMessage
{
	FComponentUpdate(const Worker_EntityId& InEntityId, const Worker_ComponentUpdate& InComponentUpdate)
		: FOutgoingMessage(EOutgoingMessageType::ComponentUpdate)
		, EntityId(InEntityId)
		, Update(InComponentUpdate)
	{}

	const Worker_EntityId EntityId;
	const Worker_ComponentUpdate Update;
};

struct FCommandRequest : FOutgoingMessage
{
	FCommandRequest(const Worker_EntityId& InEntityId, const Worker_CommandRequest& InRequest, const uint32_t& InCommandId)
		: FOutgoingMessage(EOutgoingMessageType::CommandRequest)
		, EntityId(InEntityId)
		, Request(InRequest)
		, CommandId(InCommandId)
	{}
	
	const Worker_EntityId EntityId;
	const Worker_CommandRequest Request;
	const uint32_t CommandId;
};

struct FCommandResponse : FOutgoingMessage
{
	FCommandResponse(const Worker_RequestId& InRequestId, const Worker_CommandResponse& InResponse)
		: FOutgoingMessage(EOutgoingMessageType::CommandResponse)
		, RequestId(InRequestId)
		, Response(InResponse)
	{}

	const Worker_RequestId RequestId;
	const Worker_CommandResponse Response;
};

struct FCommandFailure : FOutgoingMessage
{
	FCommandFailure(const Worker_RequestId& InRequestId, const FString& InMessage)
		: FOutgoingMessage(EOutgoingMessageType::CommandFailure)
		, RequestId(InRequestId)
		, Message(InMessage)
	{}
	
	const Worker_RequestId RequestId;
	const FString Message;
};

struct FLogMessage : FOutgoingMessage
{
	FLogMessage(const uint8_t& InLevel, const FString& InLoggerName, const FString& InMessage)
		: FOutgoingMessage(EOutgoingMessageType::LogMessage)
		, Level(InLevel)
		, LoggerName(InLoggerName)
		, Message(InMessage)
	{}

	const uint8_t Level;
	const FString LoggerName;
	const FString Message;
};

struct FComponentInterest : FOutgoingMessage
{
	FComponentInterest(const Worker_EntityId& InEntityId, TArray<Worker_InterestOverride>&& InInterests)
		: FOutgoingMessage(EOutgoingMessageType::ComponentInterest)
		, EntityId(InEntityId)
		, Interests(InInterests)
	{}

	const Worker_EntityId EntityId;
	const TArray<Worker_InterestOverride> Interests;
};

struct FEntityQueryRequest : FOutgoingMessage
{
	FEntityQueryRequest(const Worker_EntityQuery& InEntityQuery)
		: FOutgoingMessage(EOutgoingMessageType::EntityQueryRequest)
		, EntityQuery(InEntityQuery)
	{}

	const Worker_EntityQuery EntityQuery;
};

struct FMetrics : FOutgoingMessage
{
	FMetrics(const Worker_Metrics& InMetrics)
		: FOutgoingMessage(EOutgoingMessageType::Metrics)
		, Metrics(InMetrics)
	{}

	const Worker_Metrics Metrics;
};

using FOutgoingMessageWrapper = TUniquePtr<FOutgoingMessage>;
