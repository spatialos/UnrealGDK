#include "EventProcessor.h"

#include "GameFramework/Actor.h"
#include "Utils/SpatialStatics.h"

#include "Schema/RPCPayload.h"
#include "StructuredEventBuilder.h"

void GDKEventsToStructuredLogs::ReceiveAddEntity(AActor* Actor, const Worker_EntityId& EntityId)
{
	FAddEntityEvent Event = StructuredEventBuilder::ConstructNetReceiveEvent<FAddEntityEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(Actor, EntityId);

	Logger->LogEvent(Event);
}

void GDKEventsToStructuredLogs::ReceiveRemoveEntity(AActor* Actor, const Worker_EntityId& EntityId)
{
	FRemoveEntityEvent Event = StructuredEventBuilder::ConstructNetReceiveEvent<FRemoveEntityEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(Actor, EntityId);

	Logger->LogEvent(Event);
} //actor might be null

void GDKEventsToStructuredLogs::ReceiveAuthorityChange(AActor* Actor, ENetRole NewRole)
{
	FAuthorityChangeEvent Event = StructuredEventBuilder::ConstructNetReceiveEvent<FAuthorityChangeEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(Actor, USpatialStatics::GetActorEntityId(Actor));
	Event.Authority = (NewRole == ENetRole::ROLE_Authority) ? "Authoritative" : "Non-Authoritative";

	Logger->LogEvent(Event);
}
void GDKEventsToStructuredLogs::ReceiveComponentUpdate(AActor* Actor, UObject* SubObject, Worker_ComponentId ComponentId)
{
	FUpdateComponentEvent Event = StructuredEventBuilder::ConstructNetReceiveEvent<FUpdateComponentEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(Actor, USpatialStatics::GetActorEntityId(Actor));
	Event.Subobject = StructuredEventBuilder::ConstructSubobjectData(SubObject);
	Event.Component.Id = ComponentId;

	Logger->LogEvent(Event);
}

void GDKEventsToStructuredLogs::ReceiveCommandRequest(AActor* Actor, UObject* SubObject, UFunction* Function, SpatialGDK::RPCPayload& payload, Worker_RequestId LocalRequestId)
{
	FRPCRequestEvent Event = StructuredEventBuilder::ConstructNetReceiveEvent<FRPCRequestEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(Actor, USpatialStatics::GetActorEntityId(Actor));
	Event.Subobject = StructuredEventBuilder::ConstructSubobjectData(SubObject);
	Event.RPC = StructuredEventBuilder::ConstructUserRPCData(Function, &payload, LocalRequestId);

	Logger->LogEvent(Event);
}

void GDKEventsToStructuredLogs::ReceiveNamedCommandRequest(const FString& CommandName, Worker_RequestId LocalRequestId)
{
	FRPCRequestEvent Event = StructuredEventBuilder::ConstructNetReceiveEvent<FRPCRequestEvent>();
	//todo: some meaningful actor data? make separate command type?
	Event.RPC = StructuredEventBuilder::ConstructGdkRPCData(CommandName, LocalRequestId);

	Logger->LogEvent(Event);
}
void GDKEventsToStructuredLogs::ReceiveCommandResponse(AActor* Actor, UObject* SubObject, UFunction* Function, const Worker_CommandResponseOp& Op)
{
	FRPCResponseEvent Event = StructuredEventBuilder::ConstructNetReceiveEvent<FRPCResponseEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(Actor, USpatialStatics::GetActorEntityId(Actor));
	Event.Subobject = StructuredEventBuilder::ConstructSubobjectData(SubObject);
	Event.RPC = StructuredEventBuilder::ConstructUserRPCData(Function, nullptr, Op.request_id);
	Event.Success = Op.status_code == WORKER_STATUS_CODE_SUCCESS;
	Event.StatusCode = Op.status_code;
	Event.ErrorMessage = FString(Op.message);
	
	Logger->LogEvent(Event);
} // pointers can be null
void GDKEventsToStructuredLogs::ReceiveNamedCommandResponse(const FString& CommandName, const Worker_CommandResponseOp& Op)
{
	FRPCResponseEvent Event = StructuredEventBuilder::ConstructNetReceiveEvent<FRPCResponseEvent>();
	//todo: some meaningful actor data? make separate command type?
	Event.RPC = StructuredEventBuilder::ConstructGdkRPCData(CommandName, Op.request_id);
	Event.Success = Op.status_code == WORKER_STATUS_CODE_SUCCESS;
	Event.StatusCode = Op.status_code;
	Event.ErrorMessage = FString(Op.message);

	Logger->LogEvent(Event);
}

void GDKEventsToStructuredLogs::ReceiveCreateEntityResponse(AActor* Actor, const FString& ApplicationError, const Worker_CreateEntityResponseOp& Op)
{
	FRPCResponseEvent Event = StructuredEventBuilder::ConstructNetReceiveEvent<FRPCResponseEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(Actor, Op.entity_id);
	Event.RPC = StructuredEventBuilder::ConstructGdkRPCData(TEXT("CREATE_ENTITY"), Op.request_id);
	Event.Success = Op.status_code == WORKER_STATUS_CODE_SUCCESS;
	Event.StatusCode = Op.status_code;
	Event.ErrorMessage = FString(Op.message);

	Logger->LogEvent(Event);
}

void GDKEventsToStructuredLogs::SendCreateEntity(AActor* Actor, const Worker_RequestId& CreateEntityRequestId)
{
	FRPCRequestEvent Event = StructuredEventBuilder::ConstructNetSendEvent<FRPCRequestEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(Actor, USpatialStatics::GetActorEntityId(Actor));
	Event.RPC.LocalRequestId = CreateEntityRequestId;
	Event.RPC.Name = TEXT("CREATE_ENTITY");
	Event.RPC.Type = TEXT("SYSTEM");

	Logger->LogEvent(Event);
}
void GDKEventsToStructuredLogs::SendDeleteEntity(AActor* Actor, Worker_EntityId EntityId, const Worker_RequestId& DeleteEntityRequestId)
{
	FRPCRequestEvent Event = StructuredEventBuilder::ConstructNetSendEvent<FRPCRequestEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(Actor, EntityId); //EntityId manually fed as mapping somehow might already have been removed (even though the Id->Actor mapping was just fetched in calling code)
	Event.RPC.LocalRequestId = DeleteEntityRequestId;
	Event.RPC.Name = TEXT("DELETE_ENTITY");
	Event.RPC.Type = TEXT("SYSTEM");

	Logger->LogEvent(Event);
}

void GDKEventsToStructuredLogs::SendAuthorityIntentUpdate(const AActor& Actor, VirtualWorkerId NewAuthoritativeVirtualWorkerId)
{
	FAuthorityIntentChangeEvent Event = StructuredEventBuilder::ConstructNetSendEvent<FAuthorityIntentChangeEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(&Actor, USpatialStatics::GetActorEntityId(&Actor));
	Event.NewIntendedAuthority = NewAuthoritativeVirtualWorkerId;

	Logger->LogEvent(Event);
}

void GDKEventsToStructuredLogs::SendRPC(AActor* Actor, UFunction* Function, const SpatialGDK::RPCPayload& Payload)
{
	FRPCRequestEvent Event = StructuredEventBuilder::ConstructNetSendEvent<FRPCRequestEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(Actor, USpatialStatics::GetActorEntityId(Actor));
	//todo: reintroduce - Event.Subobject = LogEvents::ConstructSubobjectData(SubObject);
	Event.RPC = StructuredEventBuilder::ConstructUserRPCData(Function, &Payload, -1); //todo: pipe the local request ID

	Logger->LogEvent(Event);
}
void GDKEventsToStructuredLogs::SendRPCRetry(AActor* Actor, UFunction* Function, int AttemptNumber)
{
	FRPCRequestEvent Event = StructuredEventBuilder::ConstructNetSendEvent<FRPCRequestEvent>();
	Event.Actor = StructuredEventBuilder::ConstructActorData(Actor, USpatialStatics::GetActorEntityId(Actor));
	//todo: reintroduce - Event.Subobject = LogEvents::ConstructSubobjectData(SubObject);
	Event.RPC = StructuredEventBuilder::ConstructUserRPCData(Function, nullptr, -1); //todo: pipe the local request ID; can we get payload?
	Event.RetryAttempt = AttemptNumber;
	
	Logger->LogEvent(Event);
}

void GDKEventsToStructuredLogs::SendCommandResponse(Worker_RequestId LocalRequestId, bool success)
{
	FRPCResponseEvent Event = StructuredEventBuilder::ConstructNetSendEvent<FRPCResponseEvent>();
	// todo: can we get actor, subobject, RPCPayload?
	// Event.Actor = LogEvents::ConstructActorData(Actor, USpatialStatics::GetActorEntityId(Actor));
	// Event.Subobject = LogEvents::ConstructSubobjectData(SubObject);
	Event.RPC = StructuredEventBuilder::ConstructUserRPCData(nullptr, nullptr, LocalRequestId);
	Event.Success = success;
	Event.StatusCode = success ? WORKER_STATUS_CODE_SUCCESS : WORKER_STATUS_CODE_APPLICATION_ERROR;
	Event.ErrorMessage = "";

	Logger->LogEvent(Event);
}
