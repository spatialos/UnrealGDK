// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Utils/RepDataUtils.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

#include "SpatialSender.generated.h"

class USpatialNetDriver;
class USpatialConnection;
class USpatialActorChannel;
class USpatialPackageMapClient;
class USpatialTypebindingManager;
class USpatialReceiver;

struct FPendingRPCParams
{
	FPendingRPCParams(UObject* InTargetObject, UFunction* InFunction, void* InParameters)
		: TargetObject(InTargetObject), Function(InFunction), Parameters(InParameters) {}

	UObject* TargetObject;
	UFunction* Function;
	void* Parameters;
};

// TODO: Clear TMap entries when USpatialActorChannel gets deleted
// care for actor getting deleted before actor channel
using FChannelObjectPair = TPair<USpatialActorChannel*, UObject*>;
using FOutgoingRPCMap = TMap<const UObject*, TArray<FPendingRPCParams>>;
using FUnresolvedEntry = TSharedPtr<TSet<const UObject*>>;
using FHandleToUnresolved = TMap<uint16, FUnresolvedEntry>;
using FChannelToHandleToUnresolved = TMap<FChannelObjectPair, FHandleToUnresolved>;
using FOutgoingRepUpdates = TMap<const UObject*, FChannelToHandleToUnresolved>;

UCLASS()
class SPATIALGDK_API USpatialSender : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver);

	// Actor Updates
	void SendComponentUpdates(UObject* Object, USpatialActorChannel* Channel, const FPropertyChangeState& Changes);
	void SendPositionUpdate(Worker_EntityId EntityId, const FVector& Location);
	void SendRPC(UObject* TargetObject, UFunction* Function, void* Parameters, bool bOwnParameters);
	void SendCommandResponse(Worker_RequestId request_id, Worker_CommandResponse& Response);

	void SendReserveEntityIdRequest(USpatialActorChannel* Channel);
	void SendCreateEntityRequest(USpatialActorChannel* Channel, const FVector& Location, const FString& PlayerWorkerId, const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged);
	void SendDeleteEntityRequest(Worker_EntityId EntityId);

	void ResolveOutgoingOperations(UObject* Object);
	void ResolveOutgoingRPCs(UObject* Object);

private:
	// Actor Lifecycle
	Worker_RequestId CreateEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel);

	// Queuing
	void ResetOutgoingRepUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle);
	void QueueOutgoingRepUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, const TSet<const UObject*>& UnresolvedObjects);
	void QueueOutgoingRPC(const UObject* UnresolvedObject, UObject* TargetObject, UFunction* Function, void* Parameters);

	// RPC Construction
	Worker_CommandRequest CreateRPCCommandRequest(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject);
	Worker_ComponentUpdate CreateMulticastUpdate(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId EventIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject);

private:
	USpatialNetDriver* NetDriver;
	USpatialConnection* Connection;
	USpatialReceiver* Receiver;
	USpatialPackageMapClient* PackageMap;
	USpatialTypebindingManager* TypebindingManager;

	FChannelToHandleToUnresolved PropertyToUnresolved;
	FOutgoingRepUpdates ObjectToUnresolved;
	FOutgoingRPCMap OutgoingRPCs;

	TMap<Worker_RequestId, USpatialActorChannel*> PendingActorRequests;
};
