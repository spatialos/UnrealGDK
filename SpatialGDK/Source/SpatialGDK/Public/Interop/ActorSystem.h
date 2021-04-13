// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "ClaimPartitionHandler.h"
#include "Schema/SpawnData.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/RepDataUtils.h"

#include "Interop/CreateEntityHandler.h"
#include "SpatialView/ViewCoordinator.h"

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
class USpatialNetDriver;

using FChannelsToUpdatePosition =
	TSet<TWeakObjectPtr<USpatialActorChannel>, TWeakObjectPtrKeyFuncs<TWeakObjectPtr<USpatialActorChannel>, false>>;

namespace SpatialGDK
{
class SpatialEventTracer;
class FSubView;

class ActorHandler;

struct EntityDelta;

struct ActorData
{
	SpawnData Spawn;
	UnrealMetadata Metadata;
};

class ActorSystem
{
public:
	ActorSystem(const FSubView& InActorSubView, const FSubView& InAuthoritySubView, const FSubView& InAutonomousSubView,
				const FSubView& InSimulatedSubView, const FSubView& InTombstoneSubView, USpatialNetDriver* InNetDriver,
				SpatialEventTracer* InEventTracer);

	void Advance();

	static FSubView& CreateActorSubView(USpatialNetDriver& NetDriver);
	static FSubView& CreateActorSubViewOnComponent(const Worker_ComponentId ComponentId, USpatialNetDriver& NetDriver);
	static FSubView& CreateActorAuthSubView(const FSubView& ActorSubView, USpatialNetDriver& NetDriver);
	static FSubView& CreateAuthoritySubView(const FSubView& ActorSubView, USpatialNetDriver& NetDriver);
	static FSubView& CreateAutonomousSubView(const FSubView& ActorSubView, USpatialNetDriver& NetDriver);
	static FSubView& CreateSimulatedSubView(const FSubView& ActorSubView, USpatialNetDriver& NetDriver);

	UnrealMetadata* GetUnrealMetadata(Worker_EntityId EntityId);

	void MoveMappedObjectToUnmapped(const FUnrealObjectRef& Ref);
	void CleanupRepStateMap(FSpatialObjectRepState& RepState);
	void ResolvePendingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef);
	void RetireWhenAuthoritative(Worker_EntityId EntityId, Worker_ComponentId ActorClassId, bool bIsNetStartup, bool bNeedsTearOff);
	void RemoveActor(Worker_EntityId EntityId);

	// Tombstones
	void CreateTombstoneEntity(AActor* Actor);
	void RetireEntity(Worker_EntityId EntityId, bool bIsNetStartupActor) const;

	// Updates
	void SendComponentUpdates(UObject* Object, const FClassInfo& Info, USpatialActorChannel* Channel, const FRepChangeState* RepChanges,
							  const FHandoverChangeState* HandoverChanges, uint32& OutBytesWritten);
	void SendActorTornOffUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const;
	void ProcessPositionUpdates();
	void RegisterChannelForPositionUpdate(USpatialActorChannel* Channel);
	void UpdateInterestComponent(AActor* Actor);
	void SendInterestBucketComponentChange(Worker_EntityId EntityId, Worker_ComponentId OldComponent,
										   Worker_ComponentId NewComponent) const;
	void SendAddComponentForSubobject(USpatialActorChannel* Channel, UObject* Subobject, const FClassInfo& SubobjectInfo,
									  uint32& OutBytesWritten);
	void SendRemoveComponentForClassInfo(Worker_EntityId EntityId, const FClassInfo& Info);

	// Creating entities for actor channels
	void SendCreateEntityRequest(USpatialActorChannel& ActorChannel, uint32& OutBytesWritten);
	void OnEntityCreated(const Worker_CreateEntityResponseOp& Op, FSpatialGDKSpanId CreateOpSpan);
	bool HasPendingOpsForChannel(const USpatialActorChannel& ActorChannel) const;

	static Worker_ComponentData CreateLevelComponentData(const AActor& Actor, const UWorld& NetDriverWorld,
														 const USpatialClassInfoManager& ClassInfoManager);

private:
	// Helper struct to manage FSpatialObjectRepState update cycle.
	// TODO: move into own class.
	struct RepStateUpdateHelper;

	struct DeferredRetire
	{
		Worker_EntityId EntityId;
		Worker_ComponentId ActorClassId;
		bool bIsNetStartupActor;
		bool bNeedsTearOff;
	};
	TArray<DeferredRetire> EntitiesToRetireOnAuthorityGain;

	// Map from references to replicated objects to properties using these references.
	// Useful to manage entities going in and out of interest, in order to recover references to actors.
	FObjectToRepStateMap ObjectRefToRepStateMap;

	void PopulateDataStore(Worker_EntityId EntityId);
	void HandleActorAuthority(Worker_EntityId EntityId, Worker_ComponentSetId ComponentSetId, Worker_Authority Authority);

	// HACK: Make these public so they can be accessed from ActorHandler
public:
	enum class EActorSubViewType
	{
		Authority,
		Autonomous,
		Simulated,
	};

	struct FEntitySubViewUpdate
	{
		const TArray<EntityDelta>& EntityDeltas;
		EActorSubViewType SubViewType;
	};

	void ProcessUpdates(const FEntitySubViewUpdate& SubViewUpdate);
	void ProcessAdds(const FEntitySubViewUpdate& SubViewUpdate);
	void ProcessRemoves(const FEntitySubViewUpdate& SubViewUpdate);

	void ApplyComponentAdd(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data);

	void AuthorityLost(Worker_EntityId EntityId, Worker_ComponentSetId ComponentSetId);
	void AuthorityGained(Worker_EntityId EntityId, Worker_ComponentSetId ComponentSetId);

	void ComponentAdded(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data);
	void ComponentUpdated(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);
	void ComponentRemoved(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const;

	void EntityAdded(Worker_EntityId EntityId);
	void EntityRemoved(Worker_EntityId EntityId);

private:
	// Authority
	bool HasEntityBeenRequestedForDelete(Worker_EntityId EntityId) const;
	void HandleEntityDeletedAuthority(Worker_EntityId EntityId) const;
	void HandleDeferredEntityDeletion(const DeferredRetire& Retire) const;
	void UpdateShadowData(Worker_EntityId EntityId) const;

	// Component add
	void HandleDormantComponentAdded(Worker_EntityId EntityId) const;
	void HandleIndividualAddComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data);
	void AttachDynamicSubobject(AActor* Actor, Worker_EntityId EntityId, const FClassInfo& Info);
	void ApplyComponentData(USpatialActorChannel& Channel, UObject& TargetObject, const Worker_ComponentId ComponentId,
							Schema_ComponentData* Data);

	bool IsDynamicSubObject(AActor* Actor, uint32 SubObjectOffset);
	void ResolveIncomingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef);
	void ResolveObjectReferences(FRepLayout& RepLayout, UObject* ReplicatedObject, FSpatialObjectRepState& RepState,
								 FObjectReferencesMap& ObjectReferencesMap, uint8* RESTRICT StoredData, uint8* RESTRICT Data,
								 int32 MaxAbsOffset, TArray<GDK_PROPERTY(Property) *>& RepNotifies, bool& bOutSomeObjectsWereMapped);

	// Component update
	USpatialActorChannel* GetOrRecreateChannelForDormantActor(AActor* Actor, Worker_EntityId EntityID) const;
	void ApplyComponentUpdate(Worker_ComponentId ComponentId, Schema_ComponentUpdate* ComponentUpdate, UObject& TargetObject,
							  USpatialActorChannel& Channel, bool bIsHandover);

	// Entity add
	void ReceiveActor(Worker_EntityId EntityId);
	bool IsReceivedEntityTornOff(Worker_EntityId EntityId) const;
	AActor* TryGetActor(const UnrealMetadata& Metadata) const;
	AActor* TryGetOrCreateActor(ActorData& ActorComponents, Worker_EntityId EntityId);
	AActor* CreateActor(ActorData& ActorComponents, Worker_EntityId EntityId);
	void ApplyComponentDataOnActorCreation(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data,
										   USpatialActorChannel& Channel, TArray<ObjectPtrRefPair>& OutObjectsToResolve);

	USpatialActorChannel* SetUpActorChannel(AActor* Actor, Worker_EntityId EntityId);
	USpatialActorChannel* TryRestoreActorChannelForStablyNamedActor(AActor* StablyNamedActor, Worker_EntityId EntityId);

	// Entity remove
	void DestroyActor(AActor* Actor, Worker_EntityId EntityId);
	static FString GetObjectNameFromRepState(const FSpatialObjectRepState& RepState);

	void CreateEntityWithRetries(Worker_EntityId EntityId, FString EntityName, TArray<FWorkerComponentData> EntityComponents);
	static TArray<FWorkerComponentData> CopyEntityComponentData(const TArray<FWorkerComponentData>& EntityComponents);
	static void DeleteEntityComponentData(TArray<FWorkerComponentData>& EntityComponents);
	void AddTombstoneToEntity(Worker_EntityId EntityId) const;

	// Updates
	void SendAddComponents(Worker_EntityId EntityId, TArray<FWorkerComponentData> ComponentDatas) const;
	void SendRemoveComponents(Worker_EntityId EntityId, TArray<Worker_ComponentId> ComponentIds) const;

	const FSubView* ActorSubView;
	const FSubView* AuthoritySubView;
	const FSubView* AutonomousSubView;
	const FSubView* SimulatedSubView;
	const FSubView* TombstoneSubView;

	USpatialNetDriver* NetDriver;
	SpatialEventTracer* EventTracer;

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
