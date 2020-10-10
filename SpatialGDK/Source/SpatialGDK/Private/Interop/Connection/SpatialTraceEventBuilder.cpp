// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceEventBuilder.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Utils/SpatialActorUtils.h"

namespace SpatialGDK
{
FSpatialTraceEventBuilder::FSpatialTraceEventBuilder(FString InType)
	: SpatialTraceEvent(MoveTemp(InType), "")
{
}

FSpatialTraceEventBuilder::FSpatialTraceEventBuilder(FString InType, FString InMessage)
	: SpatialTraceEvent(MoveTemp(InType), MoveTemp(InMessage))
{
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddObject(FString Key, const UObject* Object)
{
	if (Object != nullptr)
	{
		if (const AActor* Actor = Cast<AActor>(Object))
		{
			AddKeyValue(TEXT("ActorPosition"), Actor->GetTransform().GetTranslation().ToString());
		}
		if (UWorld* World = Object->GetWorld())
		{
			if (USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver()))
			{
				AddKeyValue(TEXT("NetGuid"), NetDriver->PackageMap->GetNetGUIDFromObject(Object).ToString());
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

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddEntityId(FString Key, const FEntityId EntityId)
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

FSpatialTraceEvent FSpatialTraceEventBuilder::ProcessRPC(const UObject* Object, UFunction* Function)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "process_rpc")
		.AddObject(TEXT("Object"), Object)
		.AddFunction(TEXT("Function"), Function)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::SendRPC(const UObject* Object, UFunction* Function)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_rpc")
		.AddObject(TEXT("Object"), Object)
		.AddFunction(TEXT("Function"), Function)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::QueueRPC()
{
	return FSpatialTraceEventBuilder("queue_rpc").GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::RetryRPC()
{
	return FSpatialTraceEventBuilder("retry_rpc").GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::PropertyUpdate(const UObject* Object, const FEntityId EntityId,
															 const Worker_ComponentId ComponentId, const FString& PropertyName)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "property_update")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.AddComponentId(TEXT("ComponentId"), ComponentId)
		.AddKeyValue("PropertyName", PropertyName)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::MergeComponent(const FEntityId EntityId, const Worker_ComponentId ComponentId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "merge_component")
		.AddEntityId(TEXT("EntityId"), EntityId)
		.AddComponentId(TEXT("ComponentId"), ComponentId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::SendCommandRequest(const FString& Command, const int64 RequestId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_command_request")
		.AddCommand(TEXT("Command"), Command)
		.AddRequestId(TEXT("RequestId"), RequestId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::ReceiveCommandRequest(const FString& Command, const int64 RequestId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_request")
		.AddCommand(TEXT("Command"), Command)
		.AddRequestId(TEXT("RequestId"), RequestId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::ReceiveCommandRequest(const FString& Command, const UObject* Actor,
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

FSpatialTraceEvent FSpatialTraceEventBuilder::SendCommandResponse(const int64 RequestId, const bool bSuccess)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_command_response")
		.AddRequestId(TEXT("RequestId"), RequestId)
		.AddKeyValue(TEXT("Success"), BoolToString(bSuccess))
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::ReceiveCommandResponse(const FString& Command, const int64 RequestId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_response")
		.AddCommand(TEXT("Command"), Command)
		.AddRequestId(TEXT("RequestId"), RequestId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::ReceiveCommandResponse(const UObject* Actor, const int64 RequestId, const bool bSuccess)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_command_response")
		.AddObject(TEXT("Object"), Actor)
		.AddRequestId(TEXT("RequestId"), RequestId)
		.AddKeyValue(TEXT("Success"), BoolToString(bSuccess))
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::ReceiveCommandResponse(const UObject* Actor, const UObject* TargetObject,
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

FSpatialTraceEvent FSpatialTraceEventBuilder::SendRemoveEntity(const UObject* Object, const FEntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_remove_entity")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::ReceiveRemoveEntity(const FEntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_remove_entity").AddEntityId(TEXT("EntityId"), EntityId).GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::SendCreateEntity(const UObject* Object, const FEntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_create_entity")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::ReceiveCreateEntity(const FEntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_create_entity").AddEntityId(TEXT("EntityId"), EntityId).GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::ReceiveCreateEntitySuccess(const UObject* Object, const FEntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "receive_create_entity_success")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::SendRetireEntity(const UObject* Object, const FEntityId EntityId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "send_retire_entity")
		.AddObject(TEXT("Object"), Object)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::AuthorityIntentUpdate(VirtualWorkerId WorkerId, const UObject* Object)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "authority_intent_update")
		.AddObject(TEXT("Object"), Object)
		.AddNewWorkerId(TEXT("NewWorkerId"), WorkerId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::AuthorityChange(const FEntityId EntityId, const Worker_ComponentId ComponentId,
															  const Worker_Authority Authority)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "authority_loss_imminent")
		.AddEntityId(TEXT("EntityId"), EntityId)
		.AddComponentId(TEXT("ComponentId"), ComponentId)
		.AddAuthority(TEXT("Authority"), Authority)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::ComponentUpdate(const UObject* Object, const UObject* TargetObject, const FEntityId EntityId,
															  const Worker_ComponentId ComponentId)
{
	return FSpatialTraceEventBuilder(GDK_EVENT_NAMESPACE "component_update")
		.AddObject(TEXT("Object"), Object)
		.AddObject(TEXT("TargetObject"), TargetObject)
		.AddEntityId(TEXT("EntityId"), EntityId)
		.AddComponentId(TEXT("ComponentId"), ComponentId)
		.GetEvent();
}

FSpatialTraceEvent FSpatialTraceEventBuilder::GenericMessage(FString Message)
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
	case Worker_Authority::WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT:
		return TEXT("AuthorityLossIminent");
	default:
		return TEXT("Unknown");
	}
}

FString FSpatialTraceEventBuilder::BoolToString(bool bInput)
{
	return bInput ? TEXT("True") : TEXT("False");
}
} // namespace SpatialGDK
