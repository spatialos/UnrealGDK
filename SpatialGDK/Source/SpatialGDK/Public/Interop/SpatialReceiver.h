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

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialReceiver.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialReceiver, Log, All);

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

using FIncomingRPCArray = TArray<TSharedPtr<FPendingIncomingRPC>>;

DECLARE_DELEGATE_OneParam(EntityQueryDelegate, Worker_EntityQueryResponseOp&);
DECLARE_DELEGATE_OneParam(ReserveEntityIDsDelegate, Worker_ReserveEntityIdsResponseOp&);
DECLARE_DELEGATE_OneParam(HeartbeatDelegate, Worker_ComponentUpdateOp&);

UCLASS()
class USpatialReceiver : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver, FTimerManager* InTimerManager);

	// Dispatcher Calls
	void OnCriticalSection(bool InCriticalSection);
	void OnAddEntity(Worker_AddEntityOp& Op);
	void OnAddComponent(Worker_AddComponentOp& Op);
	void OnRemoveEntity(Worker_RemoveEntityOp& Op);
	void OnAuthorityChange(Worker_AuthorityChangeOp& Op);

	void OnComponentUpdate(Worker_ComponentUpdateOp& Op);
	void HandleUnreliableRPC(Worker_ComponentUpdateOp& Op);
	void OnCommandRequest(Worker_CommandRequestOp& Op);
	void OnCommandResponse(Worker_CommandResponseOp& Op);

	void OnReserveEntityIdsResponse(Worker_ReserveEntityIdsResponseOp& Op);
	void OnCreateEntityResponse(Worker_CreateEntityResponseOp& Op);

	void AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel);
	void AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC);

	void AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate);
	void AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate);

	void AddHeartbeatDelegate(Worker_EntityId EntityId, HeartbeatDelegate Delegate);

	void OnEntityQueryResponse(Worker_EntityQueryResponseOp& Op);

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

	static FTransform GetRelativeSpawnTransform(UClass* ActorClass, FTransform SpawnTransform);

	void QueryForStartupActor(AActor* Actor, Worker_EntityId EntityId);

	void HandlePlayerLifecycleAuthority(Worker_AuthorityChangeOp& Op, class APlayerController* PlayerController);
	void HandleActorAuthority(Worker_AuthorityChangeOp& Op);

	void ApplyComponentData(Worker_EntityId EntityId, Worker_ComponentData& Data, USpatialActorChannel* Channel);
	void ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* TargetObject, USpatialActorChannel* Channel, bool bIsHandover);

	void ApplyRPC(UObject* TargetObject, UFunction* Function, SpatialGDK::RPCPayload& Payload, const FString& SenderWorkerId);

	void ReceiveCommandResponse(Worker_CommandResponseOp& Op);

	void QueueIncomingRepUpdates(FChannelObjectPair ChannelObjectPair, const FObjectReferencesMap& ObjectReferencesMap, const TSet<FUnrealObjectRef>& UnresolvedRefs);
	void QueueIncomingRPC(const TSet<FUnrealObjectRef>& UnresolvedRefs, UObject* TargetObject, UFunction* Function, SpatialGDK::RPCPayload& Payload, const FString& SenderWorkerId);

	void ResolvePendingOperations_Internal(UObject* Object, const FUnrealObjectRef& ObjectRef);
	void ResolveIncomingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef);
	void ResolveIncomingRPCs(UObject* Object, const FUnrealObjectRef& ObjectRef);
	void ResolveObjectReferences(FRepLayout& RepLayout, UObject* ReplicatedObject, FObjectReferencesMap& ObjectReferencesMap, uint8* RESTRICT StoredData, uint8* RESTRICT Data, int32 MaxAbsOffset, TArray<UProperty*>& RepNotifies, bool& bOutSomeObjectsWereMapped, bool& bOutStillHasUnresolved);

	void ProcessQueuedResolvedObjects();
	void ProcessQueuedActorRPCsOnEntityCreation(AActor* Actor, SpatialGDK::RPCsOnEntityCreation& QueuedRPCs);
	void UpdateShadowData(Worker_EntityId EntityId);
	TWeakObjectPtr<USpatialActorChannel> PopPendingActorRequest(Worker_RequestId RequestId);

	AActor* FindSingletonActor(UClass* SingletonClass);

public:
	TMap<FUnrealObjectRef, TSet<FChannelObjectPair>> IncomingRefsMap;

private:
	template <typename T>
	friend T* GetComponentData(USpatialReceiver& Receiver, Worker_EntityId EntityId);

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

	bool bInCriticalSection;
	TArray<Worker_EntityId> PendingAddEntities;
	TArray<Worker_AuthorityChangeOp> PendingAuthorityChanges;
	TArray<PendingAddComponentWrapper> PendingAddComponents;

	TMap<Worker_RequestId, TWeakObjectPtr<USpatialActorChannel>> PendingActorRequests;
	FReliableRPCMap PendingReliableRPCs;

	TMap<Worker_RequestId, EntityQueryDelegate> EntityQueryDelegates;
	TMap<Worker_RequestId, ReserveEntityIDsDelegate> ReserveEntityIDsDelegates;

	TMap<Worker_EntityId_Key, HeartbeatDelegate> HeartbeatDelegates;
};
