// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialNetBitWriter.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Schema/RPCPayload.h"
#include "TimerManager.h"
#include "Utils/RepDataUtils.h"
#include "Utils/RPCContainer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialSender.generated.h"

using namespace SpatialGDK;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialSender, Log, All);

class USpatialActorChannel;
class USpatialDispatcher;
class USpatialNetDriver;
class USpatialPackageMapClient;
class USpatialReceiver;
class USpatialStaticComponentView;
class USpatialClassInfoManager;
class UActorGroupManager;
class USpatialWorkerConnection;

struct FReliableRPCForRetry
{
	FReliableRPCForRetry(UObject* InTargetObject, UFunction* InFunction, Worker_ComponentId InComponentId, Schema_FieldId InRPCIndex, const TArray<uint8>& InPayload, int InRetryIndex);

	TWeakObjectPtr<UObject> TargetObject;
	UFunction* Function;
	Worker_ComponentId ComponentId;
	Schema_FieldId RPCIndex;
	TArray<uint8> Payload;
	int Attempts; // For reliable RPCs

	int RetryIndex; // Index for ordering reliable RPCs on subsequent tries
};

struct FPendingRPC
{
	FPendingRPC() = default;
	FPendingRPC(FPendingRPC&& Other);

	uint32 Offset;
	Schema_FieldId Index;
	TArray<uint8> Data;
	Schema_EntityId Entity;
};

// TODO: Clear TMap entries when USpatialActorChannel gets deleted - UNR:100
// care for actor getting deleted before actor channel
using FChannelObjectPair = TPair<TWeakObjectPtr<USpatialActorChannel>, TWeakObjectPtr<UObject>>;
using FRPCsOnEntityCreationMap = TMap<TWeakObjectPtr<const UObject>, RPCsOnEntityCreation>;
using FUnresolvedEntry = TSharedPtr<TSet<TWeakObjectPtr<const UObject>>>;
using FHandleToUnresolved = TMap<uint16, FUnresolvedEntry>;
using FChannelToHandleToUnresolved = TMap<FChannelObjectPair, FHandleToUnresolved>;
using FOutgoingRepUpdates = TMap<TWeakObjectPtr<const UObject>, FChannelToHandleToUnresolved>;
using FUpdatesQueuedUntilAuthority = TMap<Worker_EntityId_Key, TArray<Worker_ComponentUpdate>>;
using FChannelsToUpdatePosition = TSet<TWeakObjectPtr<USpatialActorChannel>>;

UCLASS()
class SPATIALGDK_API USpatialSender : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager);

	// Actor Updates
	void SendComponentUpdates(UObject* Object, const FClassInfo& Info, USpatialActorChannel* Channel, const FRepChangeState* RepChanges, const FHandoverChangeState* HandoverChanges);
	void SendComponentInterestForActor(USpatialActorChannel* Channel, Worker_EntityId EntityId, bool bNetOwned);
	void SendComponentInterestForSubobject(const FClassInfo& Info, Worker_EntityId EntityId, bool bNetOwned);
	void SendPositionUpdate(Worker_EntityId EntityId, const FVector& Location);
	bool SendRPC(const FPendingRPCParams& Params);
	void SendCommandResponse(Worker_RequestId request_id, Worker_CommandResponse& Response);
	void SendEmptyCommandResponse(Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_RequestId RequestId);
	void SendAddComponent(USpatialActorChannel* Channel, UObject* Subobject, const FClassInfo& Info);
	void SendRemoveComponent(Worker_EntityId EntityId, const FClassInfo& Info);

	void SendCreateEntityRequest(USpatialActorChannel* Channel);
	void SendDeleteEntityRequest(Worker_EntityId EntityId);

	void SendRequestToClearRPCsOnEntityCreation(Worker_EntityId EntityId);
	void ClearRPCsOnEntityCreation(Worker_EntityId EntityId);

	void SendClientEndpointReadyUpdate(Worker_EntityId EntityId);
	void SendServerEndpointReadyUpdate(Worker_EntityId EntityId);

	void EnqueueRetryRPC(TSharedRef<FReliableRPCForRetry> RetryRPC);
	void FlushRetryRPCs();
	void RetryReliableRPC(TSharedRef<FReliableRPCForRetry> RetryRPC);

	void RegisterChannelForPositionUpdate(USpatialActorChannel* Channel);
	void ProcessPositionUpdates();

	void ResolveOutgoingOperations(UObject* Object, bool bIsHandover);
	void SendOutgoingRPCs();

	bool UpdateEntityACLs(Worker_EntityId EntityId, const FString& OwnerWorkerAttribute);
	void UpdateInterestComponent(AActor* Actor);

	void ProcessRPC(FPendingRPCParamsPtr Params);
	void QueueOutgoingRPC(FPendingRPCParamsPtr Params);
	void ProcessUpdatesQueuedUntilAuthority(Worker_EntityId EntityId);

	void FlushPackedRPCs();

	RPCPayload CreateRPCPayloadFromParams(UObject* TargetObject, UFunction* Function, int ReliableRPCIndex, void* Params, TSet<TWeakObjectPtr<const UObject>>& UnresolvedObjects);
	void GainAuthorityThenAddComponent(USpatialActorChannel* Channel, UObject* Object, const FClassInfo* Info);

	// Creates an entity authoritative on this server worker, ensuring it will be able to receive updates for the GSM.
	void CreateServerWorkerEntity(int AttemptCounter = 1);

private:
	// Actor Lifecycle
	Worker_RequestId CreateEntity(USpatialActorChannel* Channel);
	Worker_ComponentData CreateLevelComponentData(AActor* Actor);

	// Queuing
	void ResetOutgoingUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, bool bIsHandover);
	void QueueOutgoingUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, const TSet<TWeakObjectPtr<const UObject>>& UnresolvedObjects, bool bIsHandover);

	// RPC Construction
	FSpatialNetBitWriter PackRPCDataToSpatialNetBitWriter(UFunction* Function, void* Parameters, int ReliableRPCId, TSet<TWeakObjectPtr<const UObject>>& UnresolvedObjects) const;

	Worker_CommandRequest CreateRPCCommandRequest(UObject* TargetObject, const RPCPayload& Payload, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject);
	Worker_CommandRequest CreateRetryRPCCommandRequest(const FReliableRPCForRetry& RPC, uint32 TargetObjectOffset);
	Worker_ComponentUpdate CreateRPCEventUpdate(UObject* TargetObject, const RPCPayload& Payload, Worker_ComponentId ComponentId, Schema_FieldId EventIndex, const UObject*& OutUnresolvedObject);
	bool AddPendingRPC(UObject* TargetObject, const FPendingRPCParams& Parameters, Worker_ComponentId ComponentId, Schema_FieldId RPCIndex, const UObject*& OutUnresolvedObject);

	TArray<Worker_InterestOverride> CreateComponentInterestForActor(USpatialActorChannel* Channel, bool bIsNetOwned);

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
	USpatialClassInfoManager* ClassInfoManager;

	UPROPERTY()
	UActorGroupManager* ActorGroupManager;

	FTimerManager* TimerManager;

	FChannelToHandleToUnresolved RepPropertyToUnresolved;
	FOutgoingRepUpdates RepObjectToUnresolved;

	FChannelToHandleToUnresolved HandoverPropertyToUnresolved;
	FOutgoingRepUpdates HandoverObjectToUnresolved;

	FRPCContainer OutgoingRPCs;
	FRPCsOnEntityCreationMap OutgoingOnCreateEntityRPCs;

	TMap<Worker_RequestId, USpatialActorChannel*> PendingActorRequests;

	TArray<TSharedRef<FReliableRPCForRetry>> RetryRPCs;

	FUpdatesQueuedUntilAuthority UpdatesQueuedUntilAuthorityMap;

	FChannelsToUpdatePosition ChannelsToUpdatePosition;

	TMap<Worker_EntityId_Key, TArray<FPendingRPC>> RPCsToPack;
};
