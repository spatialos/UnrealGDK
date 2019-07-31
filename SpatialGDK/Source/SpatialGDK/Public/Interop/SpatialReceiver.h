// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Schema/DynamicComponent.h"
#include "Schema/RPCPayload.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialCommonTypes.h"
#include "Utils/RPCContainer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialReceiver.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialReceiver, Log, All);

class USpatialNetConnection;
class USpatialSender;
class UGlobalStateManager;

struct PendingAddComponentWrapper
{
	PendingAddComponentWrapper() = default;
	PendingAddComponentWrapper(Worker_EntityId InEntityId, Worker_ComponentId InComponentId, TUniquePtr<SpatialGDK::DynamicComponent>&& InData)
		: EntityId(InEntityId), ComponentId(InComponentId), Data(MoveTemp(InData)) {}

	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;
	TUniquePtr<SpatialGDK::DynamicComponent> Data;
};

struct FObjectReferences
{
	FObjectReferences() = default;
	FObjectReferences(FObjectReferences&& Other)
		: UnresolvedRefs(MoveTemp(Other.UnresolvedRefs))
		, bSingleProp(Other.bSingleProp)
		, bFastArrayProp(Other.bFastArrayProp)
		, Buffer(MoveTemp(Other.Buffer))
		, NumBufferBits(Other.NumBufferBits)
		, Array(MoveTemp(Other.Array))
		, ShadowOffset(Other.ShadowOffset)
		, ParentIndex(Other.ParentIndex)
		, Property(Other.Property) {}

	// Single property constructor
	FObjectReferences(const FUnrealObjectRef& InUnresolvedRef, int32 InCmdIndex, int32 InParentIndex, UProperty* InProperty)
		: bSingleProp(true), bFastArrayProp(false), ShadowOffset(InCmdIndex), ParentIndex(InParentIndex), Property(InProperty)
	{
		UnresolvedRefs.Add(InUnresolvedRef);
	}

	// Struct (memory stream) constructor
	FObjectReferences(const TArray<uint8>& InBuffer, int32 InNumBufferBits, const TSet<FUnrealObjectRef>& InUnresolvedRefs, int32 InCmdIndex, int32 InParentIndex, UProperty* InProperty, bool InFastArrayProp = false)
		: UnresolvedRefs(InUnresolvedRefs), bSingleProp(false), bFastArrayProp(InFastArrayProp), Buffer(InBuffer), NumBufferBits(InNumBufferBits), ShadowOffset(InCmdIndex), ParentIndex(InParentIndex), Property(InProperty) {}

	// Array constructor
	FObjectReferences(FObjectReferencesMap* InArray, int32 InCmdIndex, int32 InParentIndex, UProperty* InProperty)
		: bSingleProp(false), bFastArrayProp(false), Array(InArray), ShadowOffset(InCmdIndex), ParentIndex(InParentIndex), Property(InProperty) {}

	TSet<FUnrealObjectRef>				UnresolvedRefs;

	bool								bSingleProp;
	bool								bFastArrayProp;
	TArray<uint8>						Buffer;
	int32								NumBufferBits;

	TUniquePtr<FObjectReferencesMap>	Array;
	int32								ShadowOffset;
	int32								ParentIndex;
	UProperty*							Property;
};

struct FPendingIncomingRPC
{
	FPendingIncomingRPC(const TSet<FUnrealObjectRef>& InUnresolvedRefs, UObject* InTargetObject, UFunction* InFunction, const SpatialGDK::RPCPayload& InPayload)
		: UnresolvedRefs(InUnresolvedRefs), TargetObject(InTargetObject), Function(InFunction), Payload(InPayload) {}

	TSet<FUnrealObjectRef> UnresolvedRefs;
	TWeakObjectPtr<UObject> TargetObject;
	UFunction* Function;
	SpatialGDK::RPCPayload Payload;
	FString SenderWorkerId;
};

struct FPendingSubobjectAttachment
{
	USpatialActorChannel* Channel;
	const FClassInfo* Info;
	TWeakObjectPtr<UObject> Subobject;

	TSet<Worker_ComponentId> PendingAuthorityDelegations;
};

using FIncomingRPCArray = TArray<TSharedPtr<FPendingIncomingRPC>>;

DECLARE_DELEGATE_OneParam(EntityQueryDelegate, const Worker_EntityQueryResponseOp&);
DECLARE_DELEGATE_OneParam(ReserveEntityIDsDelegate, const Worker_ReserveEntityIdsResponseOp&);
DECLARE_DELEGATE_OneParam(CreateEntityDelegate, const Worker_CreateEntityResponseOp&);

UCLASS()
class USpatialReceiver : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver, FTimerManager* InTimerManager);

	// Dispatcher Calls
	void OnCriticalSection(bool InCriticalSection);
	void OnAddEntity(const Worker_AddEntityOp& Op);
	void OnAddComponent(const Worker_AddComponentOp& Op);
	void OnRemoveEntity(const Worker_RemoveEntityOp& Op);
	void OnRemoveComponent(const Worker_RemoveComponentOp& Op);
	void FlushRemoveComponentOps();
	void RemoveComponentOpsForEntity(Worker_EntityId EntityId);
	void OnAuthorityChange(const Worker_AuthorityChangeOp& Op);

	void OnComponentUpdate(const Worker_ComponentUpdateOp& Op);
	void HandleRPC(const Worker_ComponentUpdateOp& Op);

	void ProcessRPCEventField(Worker_EntityId EntityId, const Worker_ComponentUpdateOp &Op, const Worker_ComponentId RPCEndpointComponentId, bool bPacked);

	void OnCommandRequest(const Worker_CommandRequestOp& Op);
	void OnCommandResponse(const Worker_CommandResponseOp& Op);

	void OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op);
	void OnCreateEntityResponse(const Worker_CreateEntityResponseOp& Op);

	void AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel);
	void AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC);

	void AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate);
	void AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate);
	void AddCreateEntityDelegate(Worker_RequestId RequestId, const CreateEntityDelegate& Delegate);

	void OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op);

	void CleanupDeletedEntity(Worker_EntityId EntityId);

	void ResolvePendingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef);
	void FlushRetryRPCs();

	void OnDisconnect(Worker_DisconnectOp& Op);

private:
	void EnterCriticalSection();
	void LeaveCriticalSection();

	void ReceiveActor(Worker_EntityId EntityId);
	void RemoveActor(Worker_EntityId EntityId);
	void DestroyActor(AActor* Actor, Worker_EntityId EntityId);

	AActor* TryGetOrCreateActor(SpatialGDK::UnrealMetadata* UnrealMetadata, SpatialGDK::SpawnData* SpawnData);
	AActor* CreateActor(SpatialGDK::UnrealMetadata* UnrealMetadata, SpatialGDK::SpawnData* SpawnData);

	void ProcessRemoveComponent(const Worker_RemoveComponentOp& Op);

	static FTransform GetRelativeSpawnTransform(UClass* ActorClass, FTransform SpawnTransform);

	void QueryForStartupActor(AActor* Actor, Worker_EntityId EntityId);

	void HandlePlayerLifecycleAuthority(const Worker_AuthorityChangeOp& Op, class APlayerController* PlayerController);
	void HandleActorAuthority(const Worker_AuthorityChangeOp& Op);

	void ApplyComponentDataOnActorCreation(Worker_EntityId EntityId, const Worker_ComponentData& Data, USpatialActorChannel* Channel);
	void ApplyComponentData(UObject* TargetObject, USpatialActorChannel* Channel, const Worker_ComponentData& Data);
	// This is called for AddComponentOps not in a critical section, which means they are not a part of the initial entity creation.
	void HandleIndividualAddComponent(const Worker_AddComponentOp& Op);
	void AttachDynamicSubobject(Worker_EntityId EntityId, const FClassInfo& Info);

	void ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* TargetObject, USpatialActorChannel* Channel, bool bIsHandover);

	void RegisterListeningEntityIfReady(Worker_EntityId EntityId, Schema_Object* Object);

	bool ApplyRPC(const FPendingRPCParams& Params);
	bool ApplyRPC(UObject* TargetObject, UFunction* Function, const SpatialGDK::RPCPayload& Payload, const FString& SenderWorkerId);	

	void ReceiveCommandResponse(const Worker_CommandResponseOp& Op);

	bool IsReceivedEntityTornOff(Worker_EntityId EntityId);

	void QueueIncomingRepUpdates(FChannelObjectPair ChannelObjectPair, const FObjectReferencesMap& ObjectReferencesMap, const TSet<FUnrealObjectRef>& UnresolvedRefs);

	void QueueIncomingRPC(FPendingRPCParamsPtr Params);

	void ResolvePendingOperations_Internal(UObject* Object, const FUnrealObjectRef& ObjectRef);
	void ResolveIncomingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef);

	void ResolveIncomingRPCs();

	void ResolveObjectReferences(FRepLayout& RepLayout, UObject* ReplicatedObject, FObjectReferencesMap& ObjectReferencesMap, uint8* RESTRICT StoredData, uint8* RESTRICT Data, int32 MaxAbsOffset, TArray<UProperty*>& RepNotifies, bool& bOutSomeObjectsWereMapped, bool& bOutStillHasUnresolved);

	void ProcessQueuedResolvedObjects();
	void ProcessQueuedActorRPCsOnEntityCreation(AActor* Actor, SpatialGDK::RPCsOnEntityCreation& QueuedRPCs);
	void UpdateShadowData(Worker_EntityId EntityId);
	TWeakObjectPtr<USpatialActorChannel> PopPendingActorRequest(Worker_RequestId RequestId);

	AActor* FindSingletonActor(UClass* SingletonClass);

	void OnHeartbeatComponentUpdate(const Worker_ComponentUpdateOp& Op);

public:
	TMap<FUnrealObjectRef, TSet<FChannelObjectPair>> IncomingRefsMap;

	TMap<TPair<Worker_EntityId_Key, Worker_ComponentId>, TSharedRef<FPendingSubobjectAttachment>> PendingEntitySubobjectDelegations;

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

	FTimerManager* TimerManager;

	// TODO: Figure out how to remove entries when Channel/Actor gets deleted - UNR:100
	TMap<FChannelObjectPair, FObjectReferencesMap> UnresolvedRefsMap;
	TArray<TPair<UObject*, FUnrealObjectRef>> ResolvedObjectQueue;

	TMap<FUnrealObjectRef, FIncomingRPCArray> IncomingRPCMap;
	FRPCContainer IncomingRPCs;

	bool bInCriticalSection;
	TArray<Worker_EntityId> PendingAddEntities;
	TArray<Worker_AuthorityChangeOp> PendingAuthorityChanges;
	TArray<PendingAddComponentWrapper> PendingAddComponents;
	TArray<Worker_RemoveComponentOp> QueuedRemoveComponentOps;

	TMap<Worker_RequestId, TWeakObjectPtr<USpatialActorChannel>> PendingActorRequests;
	FReliableRPCMap PendingReliableRPCs;

	TMap<Worker_RequestId, EntityQueryDelegate> EntityQueryDelegates;
	TMap<Worker_RequestId, ReserveEntityIDsDelegate> ReserveEntityIDsDelegates;
	TMap<Worker_RequestId, CreateEntityDelegate> CreateEntityDelegates;

	// This will map PlayerController entities to the corresponding SpatialNetConnection
	// for PlayerControllers that this server has authority over. This is used for player
	// lifecycle logic (Heartbeat component updates, disconnection logic).
	TMap<Worker_EntityId_Key, TWeakObjectPtr<USpatialNetConnection>> AuthorityPlayerControllerConnectionMap;

	TMap<TPair<Worker_EntityId_Key, Worker_ComponentId>, PendingAddComponentWrapper> PendingDynamicSubobjectComponents;
};
