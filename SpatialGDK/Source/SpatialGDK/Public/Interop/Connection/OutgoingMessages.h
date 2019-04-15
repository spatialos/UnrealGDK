// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include <WorkerSDK/improbable/c_worker.h>

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
	FComponentUpdate(const Worker_EntityId& InEntityId, const Worker_ComponentUpdate& InComponentUpdate)
		: FOutgoingMessage(EOutgoingMessageType::ComponentUpdate)
		, EntityId(InEntityId)
		, Update(InComponentUpdate)
	{}

	Worker_EntityId EntityId;
	Worker_ComponentUpdate Update;
};

struct FCommandRequest : FOutgoingMessage
{
	FCommandRequest(const Worker_EntityId& InEntityId, const Worker_CommandRequest& InRequest, const uint32_t& InCommandId)
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
	FCommandResponse(const Worker_RequestId& InRequestId, const Worker_CommandResponse& InResponse)
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
		, Interests(InInterests)
	{}

	Worker_EntityId EntityId;
	TArray<Worker_InterestOverride> Interests;
};

struct FEntityQueryRequest : FOutgoingMessage
{
	FEntityQueryRequest(const Worker_EntityQuery& InEntityQuery)
		: FOutgoingMessage(EOutgoingMessageType::EntityQueryRequest)
		, EntityQuery(InEntityQuery)
	{}

	Worker_EntityQuery EntityQuery;
};

struct FMetrics : FOutgoingMessage
{
	FMetrics(const Worker_Metrics& InMetrics)
		: FOutgoingMessage(EOutgoingMessageType::Metrics)
		, Metrics(InMetrics)
	{}

	Worker_Metrics Metrics;
};
