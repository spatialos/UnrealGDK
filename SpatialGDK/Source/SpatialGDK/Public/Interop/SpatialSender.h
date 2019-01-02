// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialTypebindingManager.h"
#include "Utils/RepDataUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialSender.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialSender, Log, All);

class USpatialActorChannel;
class USpatialDispatcher;
class USpatialNetDriver;
class USpatialPackageMapClient;
class USpatialReceiver;
class USpatialStaticComponentView;
class USpatialTypebindingManager;
class USpatialWorkerConnection;

struct FPendingRPCParams
{
	FPendingRPCParams(UObject* InTargetObject, UFunction* InFunction, void* InParameters);
	~FPendingRPCParams();

	TWeakObjectPtr<UObject> TargetObject;
	UFunction* Function;
	TArray<uint8> Parameters;
	int Attempts; // For reliable RPCs
};

// TODO: Clear TMap entries when USpatialActorChannel gets deleted - UNR:100
// care for actor getting deleted before actor channel
using FChannelObjectPair = TPair<TWeakObjectPtr<USpatialActorChannel>, TWeakObjectPtr<UObject>>;
using FOutgoingRPCMap = TMap<TWeakObjectPtr<const UObject>, TArray<TSharedRef<FPendingRPCParams>>>;
using FUnresolvedEntry = TSharedPtr<TSet<TWeakObjectPtr<const UObject>>>;
using FHandleToUnresolved = TMap<uint16, FUnresolvedEntry>;
using FChannelToHandleToUnresolved = TMap<FChannelObjectPair, FHandleToUnresolved>;
using FOutgoingRepUpdates = TMap<TWeakObjectPtr<const UObject>, FChannelToHandleToUnresolved>;

UCLASS()
class SPATIALGDK_API USpatialSender : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver);

	// Actor Updates
	void SendComponentUpdates(UObject* Object, FClassInfo* Info, USpatialActorChannel* Channel, const FRepChangeState* RepChanges, const FHandoverChangeState* HandoverChanges);
	void SendComponentInterest(AActor* Actor, Worker_EntityId EntityId);
	void SendPositionUpdate(Worker_EntityId EntityId, const FVector& Location);
	void EnqueueRPC(TSharedRef<FPendingRPCParams> Params);
	void FlushQueuedRPCs();
	void SendRPC(TSharedRef<FPendingRPCParams> Params);
	void SendCommandResponse(Worker_RequestId request_id, Worker_CommandResponse& Response);

	void SendReserveEntityIdRequest(USpatialActorChannel* Channel);
	void SendCreateEntityRequest(USpatialActorChannel* Channel);
	void SendDeleteEntityRequest(Worker_EntityId EntityId);

	void ResolveOutgoingOperations(UObject* Object, bool bIsHandover);
	void ResolveOutgoingRPCs(UObject* Object);

	bool UpdateEntityACLs(AActor* Actor, Worker_EntityId EntityId);
private:
	// Actor Lifecycle
	Worker_RequestId CreateEntity(USpatialActorChannel* Channel);

	// Queuing
	void ResetOutgoingUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, bool bIsHandover);
	void QueueOutgoingUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, const TSet<TWeakObjectPtr<const UObject>>& UnresolvedObjects, bool bIsHandover);
	void QueueOutgoingRPC(const UObject* UnresolvedObject, TSharedRef<FPendingRPCParams> Params);

	// RPC Construction
	Worker_CommandRequest CreateRPCCommandRequest(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject);
	Worker_ComponentUpdate CreateMulticastUpdate(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId EventIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject);

	TArray<Worker_InterestOverride> CreateComponentInterest(AActor* Actor);
	FString GetOwnerWorkerAttribute(AActor* Actor);

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	UPROPERTY()
	USpatialWorkerConnection* Connection;

	UPROPERTY()
	USpatialReceiver* Receiver;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	UPROPERTY()
	USpatialTypebindingManager* TypebindingManager;

	FChannelToHandleToUnresolved RepPropertyToUnresolved;
	FOutgoingRepUpdates RepObjectToUnresolved;

	FChannelToHandleToUnresolved HandoverPropertyToUnresolved;
	FOutgoingRepUpdates HandoverObjectToUnresolved;

	FOutgoingRPCMap OutgoingRPCs;

	TMap<Worker_RequestId, USpatialActorChannel*> PendingActorRequests;

	TArray<TSharedRef<FPendingRPCParams>> QueuedRPCs;
};
