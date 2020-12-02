// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceEventBuilder.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Utils/SpatialActorUtils.h"

namespace SpatialGDK
{
FSpatialTraceEventBuilder::FSpatialTraceEventBuilder(FName InType)
	: SpatialTraceEvent(MoveTemp(InType), "")
{
}

FSpatialTraceEventBuilder::FSpatialTraceEventBuilder(FName InType, FString InMessage)
	: SpatialTraceEvent(MoveTemp(InType), MoveTemp(InMessage))
{
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddObject(FString Key, const UObject* Object)
{
	if (Object != nullptr)
	{
		if (const AActor* Actor = Cast<AActor>(Object))
		{
			AddKeyValue(Key + TEXT("ActorPosition"), Actor->GetTransform().GetTranslation().ToString());
		}
		if (UWorld* World = Object->GetWorld())
		{
			if (USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
			{
				AddKeyValue(Key + TEXT("NetGuid"), NetDriver->PackageMap->GetNetGUIDFromObject(Object).ToString());
			}
		}
		AddKeyValue(MoveTemp(Key), Object->GetName());
	}
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddFunction(FString Key, const UFunction* Function)
{
	if (Function != nullptr)
	{
		AddKeyValue(MoveTemp(Key), Function->GetName());
	}
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddEntityId(FString Key, const Worker_EntityId EntityId)
{
	AddKeyValue(MoveTemp(Key), FString::FromInt(EntityId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddComponentId(FString Key, const Worker_ComponentId ComponentId)
{
	AddKeyValue(MoveTemp(Key), FString::FromInt(ComponentId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddFieldId(FString Key, const uint32 FieldId)
{
	AddKeyValue(MoveTemp(Key), FString::FromInt(FieldId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddNewWorkerId(FString Key, const uint32 NewWorkerId)
{
	AddKeyValue(MoveTemp(Key), FString::FromInt(NewWorkerId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddCommand(FString Key, const FString& Command)
{
	AddKeyValue(MoveTemp(Key), Command);
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddRequestId(FString Key, const int64 RequestId)
{
	AddKeyValue(MoveTemp(Key), FString::FromInt(RequestId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddAuthority(FString Key, const Worker_Authority Authority)
{
	AddKeyValue(MoveTemp(Key), AuthorityToString(Authority));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddKeyValue(FString Key, FString Value)
{
	SpatialTraceEvent.AddData(MoveTemp(Key), MoveTemp(Value));
	return *this;
}

FSpatialTraceEvent FSpatialTraceEventBuilder::GetEvent() &&
{
	return MoveTemp(SpatialTraceEvent);
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateProcessRPC(const UObject* Object, UFunction* Function,
															   const EventTraceUniqueId& LinearTraceId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "process_rpc")
		.AddObject(TEXT("Object"), Object)
		.AddFunction(TEXT("Function"), Function)
		.AddKeyValue(TEXT("LinearTraceId"), LinearTraceId.ToString())
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreatePushRPC(const UObject* Object, UFunction* Function)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "push_rpc")
		.AddObject(TEXT("Object"), Object)
		.AddFunction(TEXT("Function"), Function)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateSendRPC(const EventTraceUniqueId& LinearTraceId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_rpc")
		.AddKeyValue(TEXT("LinearTraceId"), LinearTraceId.ToString())
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateQueueRPC()
{
	return FSpatialTraceEventBuilder("queue_rpc").GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateRetryRPC()
{
	return FSpatialTraceEventBuilder("retry_rpc").GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreatePropertyChanged(const UObject* Object, const Worker_EntityId EntityId,
																	const FString& PropertyName, EventTraceUniqueId LinearTraceId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "property_changed")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.AddKeyValue(TEXT("PropertyName"), PropertyName)
		.AddKeyValue(TEXT("LinearTraceId"), LinearTraceId.ToString())
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateSendPropertyUpdate(const UObject* Object, const Worker_EntityId EntityId,
																	   const Worker_ComponentId ComponentId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_property_update")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.AddEntityId(TEXT("ComponentId"), ComponentId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateReceivePropertyUpdate(const UObject* Object, const Worker_EntityId EntityId,
																		  const Worker_ComponentId ComponentId, const FString& PropertyName,
																		  EventTraceUniqueId LinearTraceId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_property_update")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.AddComponentId(TEXT("ComponentId"), ComponentId)
		.AddKeyValue("PropertyName", PropertyName)
		.AddKeyValue(TEXT("LinearTraceId"), LinearTraceId.ToString())
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateMergeSendRPCs(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "merge_send_rpcs")
		.AddEntityId(TEXT("EntityId"), EntityId)
		.AddComponentId(TEXT("ComponentId"), ComponentId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateMergeComponentUpdate(const Worker_EntityId EntityId,
																		 const Worker_ComponentId ComponentId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "merge_component_update")
		.AddEntityId(TEXT("EntityId"), EntityId)
		.AddComponentId(TEXT("ComponentId"), ComponentId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateObjectPropertyComponentUpdate(const UObject* Object)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "merge_property_update").AddObject(TEXT("Object"), Object).GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateSendCommandRequest(const FString& Command, const int64 RequestId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_command_request")
		.AddCommand(TEXT("Command"), Command)
		.AddRequestId(TEXT("RequestId"), RequestId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateReceiveCommandRequest(const FString& Command, const int64 RequestId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_request")
		.AddCommand(TEXT("Command"), Command)
		.AddRequestId(TEXT("RequestId"), RequestId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateReceiveCommandRequest(const FString& Command, const UObject* Actor,
																		  const UObject* TargetObject, const UFunction* Function,
																		  const int32 TraceId, const int64 RequestId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_request")
		.AddCommand(TEXT("Command"), Command)
		.AddObject(TEXT("Object"), Actor)
		.AddObject(TEXT("TargetObject"), TargetObject)
		.AddFunction(TEXT("Function"), Function)
		.AddKeyValue(TEXT("TraceId"), FString::FromInt(TraceId))
		.AddRequestId(TEXT("RequestId"), RequestId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateSendCommandResponse(const int64 RequestId, const bool bSuccess)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_command_response")
		.AddRequestId(TEXT("RequestId"), RequestId)
		.AddKeyValue(TEXT("Success"), BoolToString(bSuccess))
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateReceiveCommandResponse(const FString& Command, const int64 RequestId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_response")
		.AddCommand(TEXT("Command"), Command)
		.AddRequestId(TEXT("RequestId"), RequestId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateReceiveCommandResponse(const UObject* Actor, const int64 RequestId, const bool bSuccess)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_response")
		.AddObject(TEXT("Object"), Actor)
		.AddRequestId(TEXT("RequestId"), RequestId)
		.AddKeyValue(TEXT("Success"), BoolToString(bSuccess))
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateReceiveCommandResponse(const UObject* Actor, const UObject* TargetObject,
																		   const UFunction* Function, int64 RequestId, const bool bSuccess)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_response")
		.AddObject(TEXT("Object"), Actor)
		.AddObject(TEXT("TargetObject"), TargetObject)
		.AddFunction(TEXT("Function"), Function)
		.AddRequestId(TEXT("RequestId"), RequestId)
		.AddKeyValue(TEXT("Success"), BoolToString(bSuccess))
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateSendRemoveEntity(const UObject* Object, const Worker_EntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_remove_entity")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateReceiveRemoveEntity(const Worker_EntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_remove_entity").AddEntityId(TEXT("EntityId"), EntityId).GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateSendCreateEntity(const UObject* Object, const Worker_EntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_create_entity")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateReceiveCreateEntity(const Worker_EntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_create_entity").AddEntityId(TEXT("EntityId"), EntityId).GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateReceiveCreateEntitySuccess(const UObject* Object, const Worker_EntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_create_entity_success")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateSendRetireEntity(const UObject* Object, const Worker_EntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_retire_entity")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateAuthorityIntentUpdate(VirtualWorkerId WorkerId, const UObject* Object)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "authority_intent_update")
		.AddObject(TEXT("Object"), Object)
		.AddNewWorkerId(TEXT("NewWorkerId"), WorkerId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateAuthorityChange(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
																	const Worker_Authority Authority)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "authority_change")
		.AddEntityId(TEXT("EntityId"), EntityId)
		.AddComponentId(TEXT("ComponentId"), ComponentId)
		.AddAuthority(TEXT("Authority"), Authority)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateComponentUpdate(const UObject* Object, const UObject* TargetObject,
																	const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "component_update")
		.AddObject(TEXT("Object"), Object)
		.AddObject(TEXT("TargetObject"), TargetObject)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.AddComponentId(TEXT("ComponentId"), ComponentId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::CreateGenericMessage(FString Message)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "generic_message", MoveTemp(Message)).GetEvent();
}

FString FSpatialTraceEventBuilder::AuthorityToString(Worker_Authority Authority)
{
	switch (Authority)
	{
	case Worker_Authority::WORKER_AUTHORITY_NOT_AUTHORITATIVE:
		return TEXT("NotAuthoritative");
	case Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE:
		return TEXT("Authoritative");
	default:
		return TEXT("Unknown");
	}
}

FString FSpatialTraceEventBuilder::BoolToString(bool bInput)
{
	return bInput ? TEXT("True") : TEXT("False");
}
} // namespace SpatialGDK
