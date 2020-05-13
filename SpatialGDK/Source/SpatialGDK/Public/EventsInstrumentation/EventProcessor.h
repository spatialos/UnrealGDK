#pragma once

#include "SpatialCommonTypes.h"
#include "WorkerSDK/improbable/c_worker.h"
#include <Runtime\Engine\Classes\Engine\EngineTypes.h>

#include "EventLogger.h"

namespace SpatialGDK {
	struct RPCPayload;
}

class AActor;
class USpatialActorChannel;

class GDKEventsToStructuredLogs
{
private:
	TSharedPtr<GDKStructuredEventLogger> Logger;
public:
	GDKEventsToStructuredLogs(){}
	GDKEventsToStructuredLogs(const TSharedPtr<GDKStructuredEventLogger>& EventLogger)
		: Logger(EventLogger)
	{
	}

//receive interface	
	void ReceiveAddEntity(AActor* Actor, const Worker_EntityId& EntityId); //actor might be null
	void ReceiveRemoveEntity(AActor* Actor, const Worker_EntityId& EntityId); //actor might be null
	
	/*missing add / remove components*/

	void ReceiveAuthorityChange(AActor* Actor, ENetRole NewRole);
	void ReceiveComponentUpdate(AActor* Actor, UObject* SubObject, Worker_ComponentId ComponentId);

	//todo: capture immediate request failures if entity async loading
	void ReceiveCommandRequest(AActor* Actor, UObject* SubObject, UFunction* Function, SpatialGDK::RPCPayload& payload, Worker_RequestId LocalRequestId);
	void ReceiveNamedCommandRequest(const FString& CommandName, Worker_RequestId LocalRequestId);
	void ReceiveCommandResponse(AActor* Actor, UObject* SubObject, UFunction* Function, const Worker_CommandResponseOp& Op); // pointers can be null
	void ReceiveNamedCommandResponse(const FString& CommandName, const Worker_CommandResponseOp& Op);

	void ReceiveCreateEntityResponse(AActor* Actor, const FString& ApplicationError, const Worker_CreateEntityResponseOp& Op); // actor can be null, see err


//send interface
	void SendCreateEntity(AActor* Actor, const Worker_RequestId& CreateEntityRequestId);
	void SendDeleteEntity(AActor* Actor, Worker_EntityId EntityId, const Worker_RequestId& DeleteEntityRequestId);
	//todo: tombstone not logged

	/*missing add / remove / update components*/

	void SendAuthorityIntentUpdate(const AActor& Actor, VirtualWorkerId NewAuthoritativeVirtualWorkerId);

	//todo: capture target subobject like we did in command receive logging
	void SendRPC(AActor* Target, UFunction* RPCName, const SpatialGDK::RPCPayload& Payload);
	//todo: RPCs have some internal failure conditions before send which look interesting to capture such as target having been destroyed already
	void SendRPCRetry(AActor* Target, UFunction* RPCName, int AttemptNumber);
	//todo: GDK currently doesn't seem to have tracking in place for the TraceKey which lives in SpatialGDK::RPCPayload (not available for teh retry flow)
	
	void SendCommandResponse(Worker_RequestId LocalRequestId, bool success);

	//todo: flow triggering these sometimes queue requests; log the actual flushes?
};
