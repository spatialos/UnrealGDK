// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "ClaimPartitionHandler.h"
#include "Schema/SpawnData.h"
#include "Schema/UnrealMetadata.h"
#include "Utils/RepDataUtils.h"

#include "Interop/ClientNetLoadActorHelper.h"
#include "Interop/CreateEntityHandler.h"

DECLARE_LOG_CATEGORY_EXTERN(LogActorSystem, Log, All);

class USpatialClassInfoManager;

struct FRepChangeState;
struct FPendingSubobjectAttachment;
class USpatialNetConnection;
class FSpatialObjectRepState;
class FRepLayout;
struct FClassInfo;
class USpatialNetDriver;

class SpatialActorChannel;

using FChannelsToUpdatePosition =
	TSet<TWeakObjectPtr<USpatialActorChannel>, TWeakObjectPtrKeyFuncs<TWeakObjectPtr<USpatialActorChannel>, false>>;

namespace SpatialGDK
{
class SpatialEventTracer;
class FSubView;

struct ActorData
{
	SpawnData Spawn;
	UnrealMetadata Metadata;
};

class ActorSystem
{
public:
	ActorSystem(const FSubView& InActorSubView, const FSubView& InAuthoritySubView, const FSubView& InOwnershipSubView,
				const FSubView& InSimulatedSubView, const FSubView& InTombstoneSubView, USpatialNetDriver* InNetDriver,
				SpatialEventTracer* InEventTracer);

	void Advance();

	UnrealMetadata* GetUnrealMetadata(FSpatialEntityId EntityId);

	void MoveMappedObjectToUnmapped(const FUnrealObjectRef& Ref);
	void CleanupRepStateMap(FSpatialObjectRepState& RepState);
	void ResolvePendingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef);
	void RetireWhenAuthoritative(FSpatialEntityId EntityId, Worker_ComponentId ActorClassId, bool bIsNetStartup, bool bNeedsTearOff);
	void RemoveActor(FSpatialEntityId EntityId);

	// Tombstones
	void CreateTombstoneEntity(AActor* Actor);
	void RetireEntity(FSpatialEntityId EntityId, bool bIsNetStartupActor) const;

	// Updates
	void SendComponentUpdates(UObject* Object, const FClassInfo& Info, USpatialActorChannel* Channel, const FRepChangeState* RepChanges,
							  uint32& OutBytesWritten);
	void SendActorTornOffUpdate(FSpatialEntityId EntityId, Worker_ComponentId ComponentId) const;
	void ProcessPositionUpdates();
	void RegisterChannelForPositionUpdate(USpatialActorChannel* Channel);
	void UpdateInterestComponent(AActor* Actor);
	void SendInterestBucketComponentChange(FSpatialEntityId EntityId, Worker_ComponentId OldComponent,
										   Worker_ComponentId NewComponent) const;
	void SendAddComponentForSubobject(USpatialActorChannel* Channel, UObject* Subobject, const FClassInfo& SubobjectInfo,
									  uint32& OutBytesWritten);
	void SendRemoveComponentForClassInfo(FSpatialEntityId EntityId, const FClassInfo& Info);

	// Creating entities for actor channels
	void SendCreateEntityRequest(USpatialActorChannel& ActorChannel, uint32& OutBytesWritten);
	void OnEntityCreated(const Worker_CreateEntityResponseOp& Op, FSpatialGDKSpanId CreateOpSpan);
	bool HasPendingOpsForChannel(const USpatialActorChannel& ActorChannel) const;

	static Worker_ComponentData CreateLevelComponentData(const AActor& Actor, const UWorld& NetDriverWorld,
														 const USpatialClassInfoManager& ClassInfoManager);

	void DestroySubObject(const FSpatialEntityId EntityId, UObject& Object, const FUnrealObjectRef& ObjectRef) const;

private:
	// Helper struct to manage FSpatialObjectRepState update cycle.
	// TODO: move into own class.
	struct RepStateUpdateHelper;

	struct DeferredRetire
	{
		FSpatialEntityId EntityId;
		Worker_ComponentId ActorClassId;
		bool bIsNetStartupActor;
		bool bNeedsTearOff;
	};
	TArray<DeferredRetire> EntitiesToRetireOnAuthorityGain;

	// Map from references to replicated objects to properties using these references.
	// Useful to manage entities going in and out of interest, in order to recover references to actors.
	FObjectToRepStateMap ObjectRefToRepStateMap;

	void PopulateDataStore(FSpatialEntityId EntityId);

	struct FEntitySubViewUpdate;

	void ProcessUpdates(const FEntitySubViewUpdate& SubViewUpdate);
	void ProcessAdds(const FEntitySubViewUpdate& SubViewUpdate);
	void ProcessRemoves(const FEntitySubViewUpdate& SubViewUpdate);

	void ApplyComponentAdd(FSpatialEntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data);

	void AuthorityLost(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId);
	void AuthorityGained(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId);
	void HandleActorAuthority(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId, Worker_Authority Authority);

	void ComponentAdded(FSpatialEntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data);
	void ComponentUpdated(FSpatialEntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);
	void ComponentRemoved(FSpatialEntityId EntityId, Worker_ComponentId ComponentId) const;

	void EntityAdded(FSpatialEntityId EntityId);
	void EntityRemoved(FSpatialEntityId EntityId);
	void RefreshEntity(const FSpatialEntityId EntityId);
	void ApplyFullState(const FSpatialEntityId EntityId, USpatialActorChannel& EntityActorChannel, AActor& EntityActor);

	// Authority
	bool HasEntityBeenRequestedForDelete(FSpatialEntityId EntityId) const;
	void HandleEntityDeletedAuthority(FSpatialEntityId EntityId) const;
	void HandleDeferredEntityDeletion(const DeferredRetire& Retire) const;
	void UpdateShadowData(FSpatialEntityId EntityId) const;

	// Component add
	void HandleDormantComponentAdded(FSpatialEntityId EntityId) const;
	void HandleIndividualAddComponent(FSpatialEntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data);
	void AttachDynamicSubobject(AActor* Actor, FSpatialEntityId EntityId, const FClassInfo& Info);
	void ApplyComponentData(USpatialActorChannel& Channel, UObject& TargetObject, const Worker_ComponentId ComponentId,
							Schema_ComponentData* Data);

	void ResolveIncomingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef);
	void ResolveObjectReferences(FRepLayout& RepLayout, UObject* ReplicatedObject, FSpatialObjectRepState& RepState,
								 FObjectReferencesMap& ObjectReferencesMap, uint8* RESTRICT StoredData, uint8* RESTRICT Data,
								 int32 MaxAbsOffset, TArray<GDK_PROPERTY(Property) *>& RepNotifies, bool& bOutSomeObjectsWereMapped);

	// Component update
	USpatialActorChannel* GetOrRecreateChannelForDormantActor(AActor* Actor, FSpatialEntityId EntityID) const;
	void ApplyComponentUpdate(Worker_ComponentId ComponentId, Schema_ComponentUpdate* ComponentUpdate, UObject& TargetObject,
							  USpatialActorChannel& Channel);

	// Entity add
	void ReceiveActor(FSpatialEntityId EntityId);
	bool IsReceivedEntityTornOff(FSpatialEntityId EntityId) const;
	AActor* TryGetActor(const UnrealMetadata& Metadata) const;
	AActor* TryGetOrCreateActor(ActorData& ActorComponents, FSpatialEntityId EntityId);
	AActor* CreateActor(ActorData& ActorComponents, FSpatialEntityId EntityId);
	void ApplyComponentDataOnActorCreation(FSpatialEntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data,
										   USpatialActorChannel& Channel, TArray<ObjectPtrRefPair>& OutObjectsToResolve);

	USpatialActorChannel* SetUpActorChannel(AActor* Actor, FSpatialEntityId EntityId);
	USpatialActorChannel* TryRestoreActorChannelForStablyNamedActor(AActor* StablyNamedActor, FSpatialEntityId EntityId);

	// Entity remove
	void DestroyActor(AActor* Actor, FSpatialEntityId EntityId);
	static FString GetObjectNameFromRepState(const FSpatialObjectRepState& RepState);

	void CreateEntityWithRetries(FSpatialEntityId EntityId, FString EntityName, TArray<FWorkerComponentData> EntityComponents);
	static TArray<FWorkerComponentData> CopyEntityComponentData(const TArray<FWorkerComponentData>& EntityComponents);
	static void DeleteEntityComponentData(TArray<FWorkerComponentData>& EntityComponents);
	void AddTombstoneToEntity(FSpatialEntityId EntityId) const;

	// Updates
	void SendAddComponents(FSpatialEntityId EntityId, TArray<FWorkerComponentData> ComponentDatas) const;
	void SendRemoveComponents(FSpatialEntityId EntityId, TArray<Worker_ComponentId> ComponentIds) const;

	const FSubView* ActorSubView;
	const FSubView* AuthoritySubView;
	const FSubView* OwnershipSubView;
	const FSubView* SimulatedSubView;
	const FSubView* TombstoneSubView;

	USpatialNetDriver* NetDriver;
	SpatialEventTracer* EventTracer;
	FClientNetLoadActorHelper ClientNetLoadActorHelper;

	CreateEntityHandler CreateEntityHandler;
	ClaimPartitionHandler ClaimPartitionHandler;

	TSet<Worker_EntityId_Key> PresentEntities;

	TMap<Worker_RequestId_Key, TWeakObjectPtr<USpatialActorChannel>> CreateEntityRequestIdToActorChannel;

	TMap<Worker_EntityId_Key, TSet<Worker_ComponentId>> PendingDynamicSubobjectComponents;

	FChannelsToUpdatePosition ChannelsToUpdatePosition;

	// Deserialized state store for Actor relevant components.
	TMap<Worker_EntityId_Key, ActorData> ActorDataStore;
};

} // namespace SpatialGDK
