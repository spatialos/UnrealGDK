// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialNetBitWriter.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Schema/RPCPayload.h"
#include "Utils/RepDataUtils.h"

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
class USpatialWorkerConnection;

// This is currently only really used for queuing outgoing RPCs in case they have unresolved target object
// or arguments. This is only possible for singletons in a multi-worker scenario.
// TODO: remove this logic when singletons can be referenced without entity IDs (UNR-1456).
struct FPendingRPCParams
{
	FPendingRPCParams(UObject* InTargetObject, UFunction* InFunction, void* InParameters, int InRetryIndex);
	~FPendingRPCParams();

	TWeakObjectPtr<UObject> TargetObject;
	UFunction* Function;
	TArray<uint8> Parameters;
	int RetryIndex; // Index for ordering reliable RPCs on subsequent tries
	int ReliableRPCIndex;
};

struct FReliableRPCForRetry
{
	FReliableRPCForRetry(UObject* InTargetObject, UFunction* InFunction, Worker_ComponentId InComponentId, Schema_FieldId InRPCIndex, TArray<uint8>&& InPayload, int InRetryIndex);

	TWeakObjectPtr<UObject> TargetObject;
	UFunction* Function;
	Worker_ComponentId ComponentId;
	Schema_FieldId RPCIndex;
	TArray<uint8> Payload;
	int Attempts; // For reliable RPCs

	int RetryIndex; // Index for ordering reliable RPCs on subsequent tries
};

// TODO: Clear TMap entries when USpatialActorChannel gets deleted - UNR:100
// care for actor getting deleted before actor channel
using FChannelObjectPair = TPair<TWeakObjectPtr<USpatialActorChannel>, TWeakObjectPtr<UObject>>;
using FOutgoingRPCMap = TMap<TWeakObjectPtr<const UObject>, TArray<TSharedRef<FPendingRPCParams>>>;
using FRPCsOnEntityCreationMap = TMap<TWeakObjectPtr<const UObject>, RPCsOnEntityCreation>;
using FUnresolvedEntry = TSharedPtr<TSet<TWeakObjectPtr<const UObject>>>;
using FHandleToUnresolved = TMap<uint16, FUnresolvedEntry>;
using FChannelToHandleToUnresolved = TMap<FChannelObjectPair, FHandleToUnresolved>;
using FOutgoingRepUpdates = TMap<TWeakObjectPtr<const UObject>, FChannelToHandleToUnresolved>;
using FUpdatesQueuedUntilAuthority = TMap<Worker_EntityId_Key, TArray<Worker_ComponentUpdate>>;

UCLASS()
class SPATIALGDK_API USpatialSender : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver);

	// Actor Updates
	void SendComponentUpdates(UObject* Object, const FClassInfo& Info, USpatialActorChannel* Channel, const FRepChangeState* RepChanges, const FHandoverChangeState* HandoverChanges);
	void SendComponentInterest(AActor* Actor, Worker_EntityId EntityId, bool bNetOwned);
	void SendPositionUpdate(Worker_EntityId EntityId, const FVector& Location);
	void SendRPC(TSharedRef<FPendingRPCParams> Params);
	void SendCommandResponse(Worker_RequestId request_id, Worker_CommandResponse& Response);

	void SendCreateEntityRequest(USpatialActorChannel* Channel);
	void SendDeleteEntityRequest(Worker_EntityId EntityId);

	void SendRequestToClearRPCsOnEntityCreation(Worker_EntityId EntityId);
	void ClearRPCsOnEntityCreation(Worker_EntityId EntityId);

	void EnqueueRetryRPC(TSharedRef<FReliableRPCForRetry> RetryRPC);
	void FlushRetryRPCs();
	void RetryReliableRPC(TSharedRef<FReliableRPCForRetry> RetryRPC);

	void ResolveOutgoingOperations(UObject* Object, bool bIsHandover);
	void ResolveOutgoingRPCs(UObject* Object);

	bool UpdateEntityACLs(Worker_EntityId EntityId, const FString& OwnerWorkerAttribute);
	void UpdateInterestComponent(AActor* Actor);

	void ProcessUpdatesQueuedUntilAuthority(Worker_EntityId EntityId);
private:
	// Actor Lifecycle
	Worker_RequestId CreateEntity(USpatialActorChannel* Channel);
	Worker_ComponentData CreateLevelComponentData(AActor* Actor);

	// Queuing
	void ResetOutgoingUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, bool bIsHandover);
	void QueueOutgoingUpdate(USpatialActorChannel* DependentChannel, UObject* ReplicatedObject, int16 Handle, const TSet<TWeakObjectPtr<const UObject>>& UnresolvedObjects, bool bIsHandover);
	void QueueOutgoingRPC(const UObject* UnresolvedObject, TSharedRef<FPendingRPCParams> Params);

	// RPC Construction
	FSpatialNetBitWriter PackRPCDataToSpatialNetBitWriter(UFunction* Function, void* Parameters, int ReliableRPCId, TSet<TWeakObjectPtr<const UObject>>& UnresolvedObjects) const;

	Worker_CommandRequest CreateRPCCommandRequest(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject, TArray<uint8>& OutPayload, int ReliableRPCIndex);
	Worker_CommandRequest CreateRetryRPCCommandRequest(const FReliableRPCForRetry& RPC, uint32 TargetObjectOffset);
	Worker_ComponentUpdate CreateUnreliableRPCUpdate(UObject* TargetObject, UFunction* Function, void* Parameters, Worker_ComponentId ComponentId, Schema_FieldId EventIndex, Worker_EntityId& OutEntityId, const UObject*& OutUnresolvedObject);

	TArray<Worker_InterestOverride> CreateComponentInterest(AActor* Actor, bool bIsNetOwned);

	RPCPayload CreateRPCPayloadFromParams(FPendingRPCParams& RPCParams);

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

	FChannelToHandleToUnresolved RepPropertyToUnresolved;
	FOutgoingRepUpdates RepObjectToUnresolved;

	FChannelToHandleToUnresolved HandoverPropertyToUnresolved;
	FOutgoingRepUpdates HandoverObjectToUnresolved;

	FOutgoingRPCMap OutgoingRPCs;
	FRPCsOnEntityCreationMap OutgoingOnCreateEntityRPCs;

	TMap<Worker_RequestId, USpatialActorChannel*> PendingActorRequests;

	TArray<TSharedRef<FReliableRPCForRetry>> RetryRPCs;

	FUpdatesQueuedUntilAuthority UpdatesQueuedUntilAuthorityMap;
};
