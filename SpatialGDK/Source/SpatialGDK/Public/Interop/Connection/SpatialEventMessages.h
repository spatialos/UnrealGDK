// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialEventMessages.generated.h"

static_assert(sizeof(Worker_EntityId) == sizeof(int64), "EntityId assumed 64-bit here");
static_assert(sizeof(Worker_ComponentId) == sizeof(uint32), "ComponentId assumed 32-bit here");
static_assert(sizeof(VirtualWorkerId) == sizeof(uint32), "VirtualWorkerId assumed 32-bit here");
static_assert(sizeof(TraceKey) == sizeof(int32), "TraceKey assumed 32-bit here");
static_assert(sizeof(Worker_RequestId) == sizeof(int64), "RequestId assumed 64-bit here");

#define GDK_EVENT_NAMESPACE "unreal_gdk."

// Tagged with cause - could
USTRUCT()
struct FEventMessage
{
	GENERATED_BODY()

	FEventMessage() = default;
	FEventMessage(const char* InType)
		: Type(InType)
	{
	}

	const char* GetType() const { return Type; }

private:
	const char* Type = "Null";
};

USTRUCT()
struct FEventGenericMessage : public FEventMessage
{
	GENERATED_BODY()

	FEventGenericMessage()
		: FEventMessage("generic_message")
	{
	}
	FEventGenericMessage(const FString& Message)
		: FEventMessage(GDK_EVENT_NAMESPACE "generic_message")
		, Message(Message)
	{
	}

	UPROPERTY() FString Message;
};

// Tagged with cause
USTRUCT()
struct FEventCreateEntity : public FEventMessage
{
	GENERATED_BODY()

	FEventCreateEntity()
		: FEventMessage(GDK_EVENT_NAMESPACE "create_entity")
	{
	}
	FEventCreateEntity(int64 EntityId, const AActor* Actor)
		: FEventMessage(GDK_EVENT_NAMESPACE "create_entity")
		, EntityId(EntityId)
		, Actor(Actor)
	{
	}

	UPROPERTY() int64 EntityId = -1;
	UPROPERTY() const AActor* Actor = nullptr;
};

// Tagged with cause
USTRUCT()
struct FEventRemoveEntity : public FEventMessage
{
	GENERATED_BODY()

	FEventRemoveEntity()
		: FEventMessage(GDK_EVENT_NAMESPACE "remove_entity")
	{
	}
	FEventRemoveEntity(int64 EntityId, const AActor* Actor)
		: FEventMessage(GDK_EVENT_NAMESPACE "remove_entity")
		, EntityId(EntityId)
		, Actor(Actor)
	{
	}

	UPROPERTY() int64 EntityId{ -1 };
	UPROPERTY() const AActor* Actor{ nullptr };
};

// Tagged with cause
USTRUCT()
struct FEventCreateEntitySuccess : public FEventMessage
{
	GENERATED_BODY()

	FEventCreateEntitySuccess()
		: FEventMessage(GDK_EVENT_NAMESPACE "create_entity_success")
	{
	}
	FEventCreateEntitySuccess(int64 EntityId, const AActor* Actor)
		: FEventMessage(GDK_EVENT_NAMESPACE "create_entity_success")
		, EntityId(EntityId)
		, Actor(Actor)
	{
	}

	UPROPERTY() int64 EntityId{ -1 };
	UPROPERTY() const AActor* Actor{ nullptr };
};

USTRUCT()
struct FEventAuthorityIntentUpdate : public FEventMessage
{
	GENERATED_BODY()

	FEventAuthorityIntentUpdate()
		: FEventMessage(GDK_EVENT_NAMESPACE "authority_intent_update")
	{
	}
	FEventAuthorityIntentUpdate(uint32 NewWorkerId, const AActor* Actor)
		: FEventMessage(GDK_EVENT_NAMESPACE "authority_intent_update")
		, NewWorkerId(NewWorkerId)
		, Actor(Actor)
	{
	}

	UPROPERTY() uint32 NewWorkerId{ 0xFFFFFFFF };
	UPROPERTY() const AActor* Actor{ nullptr };
};

// Tagged with cause
USTRUCT()
struct FEventAuthorityLossImminent : public FEventMessage
{
	GENERATED_BODY()

	FEventAuthorityLossImminent()
		: FEventMessage(GDK_EVENT_NAMESPACE "authority_loss_imminent")
	{
	}
	FEventAuthorityLossImminent(TEnumAsByte<ENetRole> Role, const AActor* Actor)
		: FEventMessage(GDK_EVENT_NAMESPACE "authority_loss_imminent")
		, Role(Role)
		, Actor(Actor)
	{
	}

	UPROPERTY() TEnumAsByte<ENetRole> Role{ 0 };
	UPROPERTY() const AActor* Actor{ nullptr };
};

// Tagged with cause - could
USTRUCT()
struct FEventRetireEntityRequest : public FEventMessage
{
	GENERATED_BODY()

	FEventRetireEntityRequest()
		: FEventMessage(GDK_EVENT_NAMESPACE "retire_entity")
	{
	}
	FEventRetireEntityRequest(int64 EntityId, const AActor* Actor)
		: FEventMessage(GDK_EVENT_NAMESPACE "retire_entity")
		, EntityId(EntityId)
		, Actor(Actor)
	{
	}

	UPROPERTY() int64 EntityId{ -1 };
	UPROPERTY() const AActor* Actor{ nullptr };
};

USTRUCT()
struct FEventSendRPC : public FEventMessage
{
	GENERATED_BODY()

	FEventSendRPC()
		: FEventMessage(GDK_EVENT_NAMESPACE "send_rpc")
	{
	}
	FEventSendRPC(const UObject* TargetObject, const UFunction* Function)
		: FEventMessage(GDK_EVENT_NAMESPACE "send_rpc")
		, TargetObject(TargetObject)
		, Function(Function)
	{
	}

	UPROPERTY() const UObject* TargetObject{ nullptr };
	UPROPERTY() const UFunction* Function{ nullptr };
};

// Tagged with cause
USTRUCT()
struct FEventRPCQueued : public FEventMessage
{
	GENERATED_BODY()

	FEventRPCQueued()
		: FEventMessage(GDK_EVENT_NAMESPACE "queue_rpc")
	{
	}
	FEventRPCQueued(const UObject* TargetObject, const UFunction* Function)
		: FEventMessage(GDK_EVENT_NAMESPACE "queue_rpc")
		, TargetObject(TargetObject)
		, Function(Function)
	{
	}

	UPROPERTY() const UObject* TargetObject{ nullptr };
	UPROPERTY() const UFunction* Function{ nullptr };
};

// Tagged with cause
USTRUCT()
struct FEventRPCRetried : public FEventMessage
{
	GENERATED_BODY()

	FEventRPCRetried()
		: FEventMessage(GDK_EVENT_NAMESPACE "retire_rpc")
	{
	}
};

// Tagged with cause
USTRUCT()
struct FEventRPCProcessed : public FEventMessage
{
	GENERATED_BODY()

	FEventRPCProcessed()
		: FEventMessage(GDK_EVENT_NAMESPACE "process_rpc")
	{
	}
	FEventRPCProcessed(const UObject* TargetObject, const UFunction* Function)
		: FEventMessage(GDK_EVENT_NAMESPACE "process_rpc")
		, TargetObject(TargetObject)
		, Function(Function)
	{
	}

	UPROPERTY() const UObject* TargetObject{ nullptr };
	UPROPERTY() const UFunction* Function{ nullptr };
};

USTRUCT()
struct FEventComponentUpdate : public FEventMessage
{
	GENERATED_BODY()

	FEventComponentUpdate()
		: FEventMessage(GDK_EVENT_NAMESPACE "component_update")
	{
	}
	FEventComponentUpdate(const AActor* Actor, const UObject* TargetObject, uint32 ComponentId)
		: FEventMessage(GDK_EVENT_NAMESPACE "component_update")
		, Actor(Actor)
		, TargetObject(TargetObject)
		, ComponentId(ComponentId)
	{
	}

	UPROPERTY() const AActor* Actor{ nullptr };
	UPROPERTY() const UObject* TargetObject{ nullptr };
	UPROPERTY() uint32 ComponentId{ 0 };
};

// Tagged with cause
USTRUCT()
struct FEventCommandResponse : public FEventMessage
{
	GENERATED_BODY()

	FEventCommandResponse()
		: FEventMessage(GDK_EVENT_NAMESPACE "command_response")
	{
	}
	FEventCommandResponse(const FString& Command)
		: FEventMessage(GDK_EVENT_NAMESPACE "command_response")
		, Command(Command)
	{
	}
	FEventCommandResponse(const FString& Command, int64 RequestID)
		: FEventMessage(GDK_EVENT_NAMESPACE "command_response")
		, Command(Command)
		, RequestID(RequestID)
	{
	}
	FEventCommandResponse(const FString& Command, const AActor* Actor, const UObject* TargetObject, const UFunction* Function,
						  int64 RequestID, bool bSuccess)
		: FEventMessage(GDK_EVENT_NAMESPACE "command_response")
		, Command(Command)
		, Actor(Actor)
		, TargetObject(TargetObject)
		, Function(Function)
		, RequestID(RequestID)
		, bSuccess(bSuccess)
	{
	}

	UPROPERTY() FString Command;
	UPROPERTY() const AActor* Actor{ nullptr };
	UPROPERTY() const UObject* TargetObject{ nullptr };
	UPROPERTY() const UFunction* Function{ nullptr };
	UPROPERTY() int64 RequestID{ -1 };
	UPROPERTY() bool bSuccess{ false };
};

// Tagged with cause
USTRUCT()
struct FEventCommandRequest : public FEventMessage
{
	GENERATED_BODY()

	FEventCommandRequest()
		: FEventMessage(GDK_EVENT_NAMESPACE "command_request")
	{
	}
	FEventCommandRequest(const FString& Command, int64 RequestID)
		: FEventMessage(GDK_EVENT_NAMESPACE "command_request")
		, Command(Command)
		, RequestID(RequestID)
	{
	}
	FEventCommandRequest(const FString& Command, const AActor* Actor, const UObject* TargetObject, const UFunction* Function, int32 TraceId,
						 int64 RequestID)
		: FEventMessage(GDK_EVENT_NAMESPACE "command_request")
		, Command(Command)
		, Actor(Actor)
		, TargetObject(TargetObject)
		, Function(Function)
		, TraceId(TraceId)
		, RequestID(RequestID)
	{
	}

	UPROPERTY() FString Command;
	UPROPERTY() const AActor* Actor{ nullptr };
	UPROPERTY() const UObject* TargetObject{ nullptr };
	UPROPERTY() const UFunction* Function{ nullptr };
	UPROPERTY() int32 TraceId{ -1 };
	UPROPERTY() int64 RequestID{ -1 };
};

USTRUCT()
struct FEventMergeComponentUpdate : public FEventMessage
{
	GENERATED_BODY()

	FEventMergeComponentUpdate()
		: FEventMessage(GDK_EVENT_NAMESPACE "merge_component_update")
	{
	}
	FEventMergeComponentUpdate(const int64 EntityId, uint32 ComponentId)
		: FEventMessage(GDK_EVENT_NAMESPACE "merge_component_update")
		, EntityId(EntityId)
		, ComponentId(ComponentId)
	{
	}

	UPROPERTY() int64 EntityId{ -1 };
	UPROPERTY() uint32 ComponentId{ 0 };
};

USTRUCT()
struct FEventPropertyUpdated: public FEventMessage
{
	GENERATED_BODY()

	FEventPropertyUpdated()
		: FEventMessage("property_updated")
	{
	}
	FEventPropertyUpdated(const int64 EntityId, uint32 ComponentId, const FString& PropertyName)
		: FEventMessage(GDK_EVENT_NAMESPACE "property_updated")
		, EntityId(EntityId)
		, ComponentId(ComponentId)
		, PropertyName(PropertyName)
	{
	}

	UPROPERTY() int64 EntityId { -1 };
	UPROPERTY() uint32 ComponentId { 0 };
	UPROPERTY() FString PropertyName;
};

#undef GDK_EVENT_NAMESPACE
