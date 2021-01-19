// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/RPCs/SpatialRPCService.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialReceiver.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialReceiver, Log, All);

class USpatialNetConnection;
class USpatialSender;
class UGlobalStateManager;
class SpatialLoadBalanceEnforcer;

namespace SpatialGDK
{
class SpatialEventTracer;
} // namespace SpatialGDK

UCLASS()
class USpatialReceiver : public UObject, public SpatialOSDispatcherInterface
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver, SpatialGDK::SpatialEventTracer* InEventTracer);

	// Dispatcher Calls
	virtual void OnCommandRequest(const Worker_Op& Op) override;
	virtual void OnCommandResponse(const Worker_Op& Op) override;

	virtual void OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op) override;
	virtual void OnCreateEntityResponse(const Worker_Op& Op) override;

	virtual void AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel) override;
	virtual void AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC) override;

	virtual void AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate) override;
	virtual void AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate) override;
	virtual void AddCreateEntityDelegate(Worker_RequestId RequestId, CreateEntityDelegate Delegate) override;
	virtual void AddSystemEntityCommandDelegate(Worker_RequestId RequestId, SystemEntityCommandDelegate Delegate) override;

	virtual void OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op) override;

	virtual void OnSystemEntityCommandResponse(const Worker_CommandResponseOp& Op) override;

	void OnDisconnect(uint8 StatusCode, const FString& Reason);

	bool IsPendingOpsOnChannel(USpatialActorChannel& Channel);

<<<<<<< HEAD
	void CleanupRepStateMap(FSpatialObjectRepState& Replicator);
	void MoveMappedObjectToUnmapped(const FUnrealObjectRef&);

	void RetireWhenAuthoritive(Worker_EntityId EntityId, Worker_ComponentId ActorClassId, bool bIsNetStartup, bool bNeedsTearOff);

	bool IsEntityWaitingForAsyncLoad(Worker_EntityId Entity);

	void ProcessActorsFromAsyncLoading();

=======
>>>>>>> 4763aa17c... squash
private:
	TWeakObjectPtr<USpatialActorChannel> PopPendingActorRequest(Worker_RequestId RequestId);

	void ReceiveWorkerDisconnectResponse(const Worker_CommandResponseOp& Op);
	void ReceiveClaimPartitionResponse(const Worker_CommandResponseOp& Op);

public:
	TMap<Worker_RequestId_Key, Worker_PartitionId> PendingPartitionAssignments;

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialSender* Sender;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	TMap<Worker_RequestId_Key, TWeakObjectPtr<USpatialActorChannel>> PendingActorRequests;
	FReliableRPCMap PendingReliableRPCs;

	TMap<Worker_RequestId_Key, EntityQueryDelegate> EntityQueryDelegates;
	TMap<Worker_RequestId_Key, ReserveEntityIDsDelegate> ReserveEntityIDsDelegates;
	TMap<Worker_RequestId_Key, CreateEntityDelegate> CreateEntityDelegates;
	TMap<Worker_RequestId_Key, SystemEntityCommandDelegate> SystemEntityCommandDelegates;

<<<<<<< HEAD
	// This will map PlayerController entities to the corresponding SpatialNetConnection
	// for PlayerControllers that this server has authority over. This is used for player
	// lifecycle logic (Heartbeat component updates, disconnection logic).
	TMap<Worker_EntityId_Key, TWeakObjectPtr<USpatialNetConnection>> AuthorityPlayerControllerConnectionMap;

	TMap<TPair<Worker_EntityId_Key, Worker_ComponentId>, PendingAddComponentWrapper> PendingDynamicSubobjectComponents;
	TMap<Worker_EntityId_Key, FString> WorkerConnectionEntities;

	// TODO: Refactor into a separate class so we can add automated tests for this. UNR-2649
	struct EntityWaitingForAsyncLoad
	{
		FString ClassPath;
		TArray<PendingAddComponentWrapper> InitialPendingAddComponents;
		SpatialGDK::EntityComponentOpListBuilder PendingOps;
	};
	TMap<Worker_EntityId_Key, EntityWaitingForAsyncLoad> EntitiesWaitingForAsyncLoad;
	TMap<FName, TArray<Worker_EntityId>> AsyncLoadingPackages;
	TSet<FName> LoadedPackages;
	// END TODO

	struct DeferredRetire
	{
		Worker_EntityId EntityId;
		Worker_ComponentId ActorClassId;
		bool bIsNetStartupActor;
		bool bNeedsTearOff;
	};
	TArray<DeferredRetire> EntitiesToRetireOnAuthorityGain;
	bool HasEntityBeenRequestedForDelete(Worker_EntityId EntityId);
	void HandleDeferredEntityDeletion(const DeferredRetire& Retire);
	void HandleEntityDeletedAuthority(Worker_EntityId EntityId);
	bool IsDynamicSubObject(AActor* Actor, uint32 SubObjectOffset);

=======
>>>>>>> 4763aa17c... squash
	SpatialGDK::SpatialEventTracer* EventTracer;
};
