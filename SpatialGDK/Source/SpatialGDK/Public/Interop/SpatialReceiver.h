// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "Interop/SpatialRPCService.h"
#include "Schema/DynamicComponent.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/RPCPayload.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/OpList/EntityComponentOpList.h"
#include "Utils/GDKPropertyMacros.h"
#include "Utils/RPCContainer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialReceiver.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialReceiver, Log, All);

class USpatialNetConnection;
class USpatialSender;
class UGlobalStateManager;
class SpatialLoadBalanceEnforcer;

struct PendingAddComponentWrapper
{
	PendingAddComponentWrapper() = default;
	PendingAddComponentWrapper(Worker_EntityId InEntityId, Worker_ComponentId InComponentId,
							   TUniquePtr<SpatialGDK::DynamicComponent>&& InData)
		: EntityId(InEntityId)
		, ComponentId(InComponentId)
		, Data(MoveTemp(InData))
	{
	}

	// We define equality to cover just entity and component IDs since duplicated AddComponent ops
	// will be moved into unique pointers and we cannot equate the underlying Worker_ComponentData.
	bool operator==(const PendingAddComponentWrapper& Other) const
	{
		return EntityId == Other.EntityId && ComponentId == Other.ComponentId;
	}

	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;
	TUniquePtr<SpatialGDK::DynamicComponent> Data;
};

UCLASS()
class USpatialReceiver : public UObject, public SpatialOSDispatcherInterface
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver, FTimerManager* InTimerManager, SpatialGDK::SpatialRPCService* InRPCService);

	// Dispatcher Calls
	virtual void OnCriticalSection(bool InCriticalSection) override;
	virtual void OnAddEntity(const Worker_AddEntityOp& Op) override;
	virtual void OnAddComponent(const Worker_AddComponentOp& Op) override;
	virtual void OnRemoveEntity(const Worker_RemoveEntityOp& Op) override;
	virtual void OnRemoveComponent(const Worker_RemoveComponentOp& Op) override;
	virtual void FlushRemoveComponentOps() override;
	virtual void DropQueuedRemoveComponentOpsForEntity(Worker_EntityId EntityId) override;
	virtual void OnAuthorityChange(const Worker_AuthorityChangeOp& Op) override;

	virtual void OnComponentUpdate(const Worker_ComponentUpdateOp& Op) override;

	// This gets bound to a delegate in SpatialRPCService and is called for each RPC extracted when calling
	// SpatialRPCService::ExtractRPCsForEntity.
	virtual bool OnExtractIncomingRPC(Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload) override;

	virtual void OnCommandRequest(const Worker_CommandRequestOp& Op) override;
	virtual void OnCommandResponse(const Worker_CommandResponseOp& Op) override;

	virtual void OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op) override;
	virtual void OnCreateEntityResponse(const Worker_CreateEntityResponseOp& Op) override;

	virtual void AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel) override;
	virtual void AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC) override;

	virtual void AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate) override;
	virtual void AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate) override;
	virtual void AddCreateEntityDelegate(Worker_RequestId RequestId, CreateEntityDelegate Delegate) override;

	virtual void OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op) override;

	void ResolvePendingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef);
	void FlushRetryRPCs();

	void OnDisconnect(Worker_DisconnectOp& Op);

	void RemoveActor(Worker_EntityId EntityId);
	bool IsPendingOpsOnChannel(USpatialActorChannel& Channel);

	void ClearPendingRPCs(Worker_EntityId EntityId);

	void CleanupRepStateMap(FSpatialObjectRepState& Replicator);
	void MoveMappedObjectToUnmapped(const FUnrealObjectRef&);

	void RetireWhenAuthoritive(Worker_EntityId EntityId, Worker_ComponentId ActorClassId, bool bIsNetStartup, bool bNeedsTearOff);

	FRPCErrorInfo ApplyRPC(const FPendingRPCParams& Params);

private:
	void EnterCriticalSection();
	void LeaveCriticalSection();

	void ReceiveActor(Worker_EntityId EntityId);
	void DestroyActor(AActor* Actor, Worker_EntityId EntityId);

	AActor* TryGetOrCreateActor(SpatialGDK::UnrealMetadata* UnrealMetadata, SpatialGDK::SpawnData* SpawnData,
								SpatialGDK::NetOwningClientWorker* NetOwningClientWorkerData);
	AActor* CreateActor(SpatialGDK::UnrealMetadata* UnrealMetadata, SpatialGDK::SpawnData* SpawnData,
						SpatialGDK::NetOwningClientWorker* NetOwningClientWorkerData);

	USpatialActorChannel* GetOrRecreateChannelForDomantActor(AActor* Actor, Worker_EntityId EntityID);
	void ProcessRemoveComponent(const Worker_RemoveComponentOp& Op);

	static FTransform GetRelativeSpawnTransform(UClass* ActorClass, FTransform SpawnTransform);

	void HandlePlayerLifecycleAuthority(const Worker_AuthorityChangeOp& Op, class APlayerController* PlayerController);
	void HandleActorAuthority(const Worker_AuthorityChangeOp& Op);

	void HandleRPCLegacy(const Worker_ComponentUpdateOp& Op);
	void ProcessRPCEventField(Worker_EntityId EntityId, const Worker_ComponentUpdateOp& Op,
							  const Worker_ComponentId RPCEndpointComponentId);
	void HandleRPC(const Worker_ComponentUpdateOp& Op);

	void ApplyComponentDataOnActorCreation(Worker_EntityId EntityId, const Worker_ComponentData& Data, USpatialActorChannel& Channel,
										   const FClassInfo& ActorClassInfo, TArray<ObjectPtrRefPair>& OutObjectsToResolve);
	void ApplyComponentData(USpatialActorChannel& Channel, UObject& TargetObject, const Worker_ComponentData& Data);

	void HandleIndividualAddComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
									  TUniquePtr<SpatialGDK::DynamicComponent> Data);
	void AttachDynamicSubobject(AActor* Actor, Worker_EntityId EntityId, const FClassInfo& Info);

	void ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject& TargetObject, USpatialActorChannel& Channel,
							  bool bIsHandover);

	FRPCErrorInfo ApplyRPCInternal(UObject* TargetObject, UFunction* Function, const FPendingRPCParams& PendingRPCParams);

	void ReceiveCommandResponse(const Worker_CommandResponseOp& Op);

	bool IsReceivedEntityTornOff(Worker_EntityId EntityId);

	void ProcessOrQueueIncomingRPC(const FUnrealObjectRef& InTargetObjectRef, SpatialGDK::RPCPayload InPayload);

	void ResolveIncomingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef);

	void ResolveObjectReferences(FRepLayout& RepLayout, UObject* ReplicatedObject, FSpatialObjectRepState& RepState,
								 FObjectReferencesMap& ObjectReferencesMap, uint8* RESTRICT StoredData, uint8* RESTRICT Data,
								 int32 MaxAbsOffset, TArray<GDK_PROPERTY(Property) *>& RepNotifies, bool& bOutSomeObjectsWereMapped);

	void ProcessQueuedActorRPCsOnEntityCreation(Worker_EntityId EntityId, SpatialGDK::RPCsOnEntityCreation& QueuedRPCs);
	void UpdateShadowData(Worker_EntityId EntityId);
	TWeakObjectPtr<USpatialActorChannel> PopPendingActorRequest(Worker_RequestId RequestId);

	void OnHeartbeatComponentUpdate(const Worker_ComponentUpdateOp& Op);
	void CloseClientConnection(USpatialNetConnection* ClientConnection, Worker_EntityId PlayerControllerEntityId);

	void PeriodicallyProcessIncomingRPCs();

	// TODO: Refactor into a separate class so we can add automated tests for this. UNR-2649
	static bool NeedToLoadClass(const FString& ClassPath);
	static FString GetPackagePath(const FString& ClassPath);

	void StartAsyncLoadingClass(const FString& ClassPath, Worker_EntityId EntityId);
	void OnAsyncPackageLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result);

	bool IsEntityWaitingForAsyncLoad(Worker_EntityId Entity);

	void QueueAddComponentOpForAsyncLoad(const Worker_AddComponentOp& Op);
	void QueueRemoveComponentOpForAsyncLoad(const Worker_RemoveComponentOp& Op);
	void QueueAuthorityOpForAsyncLoad(const Worker_AuthorityChangeOp& Op);
	void QueueComponentUpdateOpForAsyncLoad(const Worker_ComponentUpdateOp& Op);

	TArray<PendingAddComponentWrapper> ExtractAddComponents(Worker_EntityId Entity);
	SpatialGDK::EntityComponentOpListBuilder ExtractAuthorityOps(Worker_EntityId Entity);

	struct CriticalSectionSaveState
	{
		CriticalSectionSaveState(USpatialReceiver& InReceiver);
		~CriticalSectionSaveState();

		USpatialReceiver& Receiver;

		bool bInCriticalSection;
		TArray<Worker_EntityId> PendingAddActors;
		TArray<Worker_AuthorityChangeOp> PendingAuthorityChanges;
		TArray<PendingAddComponentWrapper> PendingAddComponents;
	};

	void HandleQueuedOpForAsyncLoad(const Worker_Op& Op);
	// END TODO

public:
	TMap<TPair<Worker_EntityId_Key, Worker_ComponentId>, TSharedRef<FPendingSubobjectAttachment>> PendingEntitySubobjectDelegations;

	FOnEntityAddedDelegate OnEntityAddedDelegate;
	FOnEntityRemovedDelegate OnEntityRemovedDelegate;

	FRPCContainer& GetRPCContainer() { return IncomingRPCs; }

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	UPROPERTY()
	USpatialSender* Sender;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	UPROPERTY()
	USpatialClassInfoManager* ClassInfoManager;

	UPROPERTY()
	UGlobalStateManager* GlobalStateManager;

	SpatialLoadBalanceEnforcer* LoadBalanceEnforcer;

	FTimerManager* TimerManager;

	SpatialGDK::SpatialRPCService* RPCService;

	// Helper struct to manage FSpatialObjectRepState update cycle.
	struct RepStateUpdateHelper;

	// Map from references to replicated objects to properties using these references.
	// Useful to manage entities going in and out of interest, in order to recover references to actors.
	FObjectToRepStateMap ObjectRefToRepStateMap;

	FRPCContainer IncomingRPCs{ ERPCQueueType::Receive };

	bool bInCriticalSection;
	TArray<Worker_EntityId> PendingAddActors;
	TArray<Worker_AuthorityChangeOp> PendingAuthorityChanges;
	TArray<PendingAddComponentWrapper> PendingAddComponents;
	TArray<Worker_RemoveComponentOp> QueuedRemoveComponentOps;

	TMap<Worker_RequestId_Key, TWeakObjectPtr<USpatialActorChannel>> PendingActorRequests;
	FReliableRPCMap PendingReliableRPCs;

	TMap<Worker_RequestId_Key, EntityQueryDelegate> EntityQueryDelegates;
	TMap<Worker_RequestId_Key, ReserveEntityIDsDelegate> ReserveEntityIDsDelegates;
	TMap<Worker_RequestId_Key, CreateEntityDelegate> CreateEntityDelegates;

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
};
