// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Interop/Connection/SpatialTraceEvent.h"
#include "SpatialCommonTypes.h"
#include "WorkerSDK/improbable/c_worker.h"

#define GDK_EVENT_NAMESPACE "unreal_gdk."

namespace SpatialGDK
{
class FSpatialTraceEventBuilder
{
public:
	FSpatialTraceEventBuilder(FString InType);
	FSpatialTraceEventBuilder(FString InType, FString InMessage);

	// --- Builder Functions ---

	FSpatialTraceEventBuilder AddObject(FString Key, const UObject* Object);
	FSpatialTraceEventBuilder AddFunction(FString Key, const UFunction* Function);
	FSpatialTraceEventBuilder AddEntityId(FString Key, const Worker_EntityId EntityId);
	FSpatialTraceEventBuilder AddComponentId(FString Key, const Worker_ComponentId ComponentId);
	FSpatialTraceEventBuilder AddFieldId(FString Key, const uint32 FieldId);
	FSpatialTraceEventBuilder AddNewWorkerId(FString Key, const uint32 NewWorkerId);
	FSpatialTraceEventBuilder AddCommand(FString Key, const FString& Command);
	FSpatialTraceEventBuilder AddRequestID(FString Key, const int64 RequestID);
	FSpatialTraceEventBuilder AddAuthority(FString Key, const Worker_Authority Role);
	FSpatialTraceEventBuilder AddKeyValue(FString Key, FString Value);
	FSpatialTraceEvent GetEvent() &&;

	// --- Static Functions ---

	static FSpatialTraceEvent ProcessRPC(const UObject* Object, UFunction* Function)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "process_rpc")
			.AddObject(TEXT("Object"), Object)
			.AddFunction(TEXT("Function"), Function)
			.GetEvent();
	}

	static FSpatialTraceEvent SendRPC(const UObject* Object, UFunction* Function)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_rpc")
			.AddObject(TEXT("Object"), Object)
			.AddFunction(TEXT("Function"), Function)
			.GetEvent();
	}

	static FSpatialTraceEvent QueueRPC() { return FSpatialTraceEventBuilder("queue_rpc").GetEvent(); }

	static FSpatialTraceEvent RetryRPC() { return FSpatialTraceEventBuilder("retry_rpc").GetEvent(); }

	static FSpatialTraceEvent PropertyUpdate(const UObject* Object, const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
											 const FString& PropertyName)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "property_update")
			.AddObject(TEXT("Object"), Object)
			.AddEntityId(TEXT("EntityId"), EntityId)
			.AddComponentId(TEXT("ComponentId"), ComponentId)
			.AddKeyValue("PropertyName", PropertyName)
			.GetEvent();
	}

	static FSpatialTraceEvent MergeComponent(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "merge_component")
			.AddEntityId(TEXT("EntityId"), EntityId)
			.AddComponentId(TEXT("ComponentId"), ComponentId)
			.GetEvent();
	}

	static FSpatialTraceEvent SendCommandRequest(const FString& Command, const int64 RequestID)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_command_request")
			.AddCommand(TEXT("Command"), Command)
			.AddRequestID(TEXT("RequestID"), RequestID)
			.GetEvent();
	}

	static FSpatialTraceEvent ReceiveCommandRequest(const FString& Command, const int64 RequestID)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_request")
			.AddCommand(TEXT("Command"), Command)
			.AddRequestID(TEXT("RequestID"), RequestID)
			.GetEvent();
	}

	static FSpatialTraceEvent ReceiveCommandRequest(const FString& Command, const UObject* Actor, const UObject* TargetObject,
													const UFunction* Function, const int32 TraceId, const int64 RequestID)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_request")
			.AddCommand(TEXT("Command"), Command)
			.AddObject(TEXT("Object"), Actor)
			.AddObject(TEXT("TargetObject"), TargetObject)
			.AddFunction(TEXT("Function"), Function)
			.AddKeyValue(TEXT("TraceId"), FString::FromInt(TraceId))
			.AddRequestID(TEXT("RequestID"), RequestID)
			.GetEvent();
	}

	static FSpatialTraceEvent SendCommandResponse(const int64 RequestID, const bool bSuccess)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_command_response")
			.AddRequestID(TEXT("RequestID"), RequestID)
			.AddKeyValue(TEXT("Success"), BoolToString(bSuccess))
			.GetEvent();
	}

	static FSpatialTraceEvent ReceiveCommandResponse(const FString& Command, const int64 RequestID)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_response")
			.AddCommand(TEXT("Command"), Command)
			.AddRequestID(TEXT("RequestID"), RequestID)
			.GetEvent();
	}

	static FSpatialTraceEvent ReceiveCommandResponse(const UObject* Actor, const int64 RequestID, const bool bSuccess)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_response")
			.AddObject(TEXT("Object"), Actor)
			.AddRequestID(TEXT("RequestID"), RequestID)
			.AddKeyValue(TEXT("Success"), BoolToString(bSuccess))
			.GetEvent();
	}

	static FSpatialTraceEvent ReceiveCommandResponse(const UObject* Actor, const UObject* TargetObject, const UFunction* Function,
													 int64 RequestID, const bool bSuccess)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_response")
			.AddObject(TEXT("Object"), Actor)
			.AddObject(TEXT("TargetObject"), TargetObject)
			.AddFunction(TEXT("Function"), Function)
			.AddRequestID(TEXT("RequestID"), RequestID)
			.AddKeyValue(TEXT("Success"), BoolToString(bSuccess))
			.GetEvent();
	}

	static FSpatialTraceEvent SendRemoveEntity(const UObject* Object, const Worker_EntityId EntityId)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_remove_entity")
			.AddObject(TEXT("Object"), Object)
			.AddEntityId(TEXT("EntityId"), EntityId)
			.GetEvent();
	}

	static FSpatialTraceEvent ReceiveRemoveEntity(const Worker_EntityId EntityId)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_remove_entity")
			.AddEntityId(TEXT("EntityId"), EntityId)
			.GetEvent();
	}

	static FSpatialTraceEvent SendCreateEntity(const UObject* Object, const Worker_EntityId EntityId)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_create_entity")
			.AddObject(TEXT("Object"), Object)
			.AddEntityId(TEXT("EntityId"), EntityId)
			.GetEvent();
	}

	static FSpatialTraceEvent ReceiveCreateEntity(const Worker_EntityId EntityId)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_create_entity")
			.AddEntityId(TEXT("EntityId"), EntityId)
			.GetEvent();
	}

	static FSpatialTraceEvent ReceiveCreateEntitySuccess(const UObject* Object, const Worker_EntityId EntityId)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_create_entity_success")
			.AddObject(TEXT("Object"), Object)
			.AddEntityId(TEXT("EntityId"), EntityId)
			.GetEvent();
	}

	static FSpatialTraceEvent SendRetireEntity(const UObject* Object, const Worker_EntityId EntityId)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_retire_entity")
			.AddObject(TEXT("Object"), Object)
			.AddEntityId(TEXT("EntityId"), EntityId)
			.GetEvent();
	}

	static FSpatialTraceEvent AuthorityIntentUpdate(VirtualWorkerId WorkerId, const UObject* Object)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "authority_intent_update")
			.AddObject(TEXT("Object"), Object)
			.AddNewWorkerId(TEXT("NewWorkerId"), WorkerId)
			.GetEvent();
	}

	static FSpatialTraceEvent AuthorityChange(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, const Worker_Authority Authority)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "authority_loss_imminent")
			.AddEntityId(TEXT("EntityId"), EntityId)
			.AddComponentId(TEXT("ComponentId"), ComponentId)
			.AddAuthority(TEXT("Authority"), Authority)
			.GetEvent();
	}

	static FSpatialTraceEvent ComponentUpdate(const UObject* Object, const UObject* TargetObject, const Worker_EntityId EntityId,
											  const Worker_ComponentId ComponentId)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "component_update")
			.AddObject(TEXT("Object"), Object)
			.AddObject(TEXT("TargetObject"), TargetObject)
			.AddEntityId(TEXT("EntityId"), EntityId)
			.AddComponentId(TEXT("ComponentId"), ComponentId)
			.GetEvent();
	}

	static FSpatialTraceEvent GenericMessage(FString Message)
	{
		return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "generic_message", MoveTemp(Message)).GetEvent();
	}

	static FString AuthorityToString(Worker_Authority Authority)
	{
		switch (Authority)
		{
		case Worker_Authority::WORKER_AUTHORITY_NOT_AUTHORITATIVE:
			return TEXT("NotAuthoritative");
		case Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE:
			return TEXT("Authoritative");
		case Worker_Authority::WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT:
			return TEXT("AuthorityLossIminent");
		default:
			return TEXT("Unknown");
		}
	}

	static FString BoolToString(bool bInput) { return bInput ? TEXT("True") : TEXT("False"); }

private:
	FSpatialTraceEvent SpatialTraceEvent;
};
} // namespace SpatialGDK
