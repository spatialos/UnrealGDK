// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Schema/NetOwningClientWorker.h"
#include "Schema/SpawnData.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"

DECLARE_LOG_CATEGORY_EXTERN(LogActorSystem, Log, All);

struct FPendingSubobjectAttachment;
class USpatialNetConnection;
class FSpatialObjectRepState;
class FRepLayout;
struct FClassInfo;
class USpatialNetDriver;

class SpatialActorChannel;
class USpatialNetDriver;

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
	ActorSystem(const FSubView& InSubView, const FSubView& InTombstoneSubView, USpatialNetDriver* InNetDriver,
				FTimerManager* InTimerManager, SpatialEventTracer* InEventTracer);

	void Advance();

	UnrealMetadata* GetUnrealMetadata(Worker_EntityId EntityId);

	void MoveMappedObjectToUnmapped(const FUnrealObjectRef& Ref);
	void CleanupRepStateMap(FSpatialObjectRepState& RepState);
	void ResolvePendingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef);
	void RetireWhenAuthoritative(Worker_EntityId EntityId, Worker_ComponentId ActorClassId, bool bIsNetStartup, bool bNeedsTearOff);
	void RemoveActor(Worker_EntityId EntityId);

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
	void ApplyComponentAdd(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data);

	void AuthorityLost(Worker_EntityId EntityId, Worker_ComponentSetId ComponentSetId);
	void AuthorityGained(Worker_EntityId EntityId, Worker_ComponentSetId ComponentSetId);
	void HandleActorAuthority(Worker_EntityId EntityId, Worker_ComponentSetId ComponentSetId, Worker_Authority Authority);

	void ComponentAdded(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* Data);
	void ComponentUpdated(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);
	void ComponentRemoved(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const;

	void EntityAdded(Worker_EntityId EntityId);
	void EntityRemoved(Worker_EntityId EntityId);

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

	// Entity remove
	void DestroyActor(AActor* Actor, Worker_EntityId EntityId);
	static FString GetObjectNameFromRepState(const FSpatialObjectRepState& RepState);

	// Helper
	bool EntityHasComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const;

	const FSubView* SubView;
	const FSubView* TombstoneSubView;
	USpatialNetDriver* NetDriver;
	FTimerManager* TimerManager;
	SpatialEventTracer* EventTracer;

	TMap<Worker_EntityId_Key, TArray<Worker_ComponentId>> PendingDynamicSubobjectComponents;

	TArray<Worker_ComponentId> SemanticActorComponents = { SpatialConstants::SPAWN_DATA_COMPONENT_ID,
														   SpatialConstants::UNREAL_METADATA_COMPONENT_ID };
	// Deserialized state store for Actor relevant components.
	TMap<Worker_EntityId_Key, ActorData> ActorDataStore;
};

} // namespace SpatialGDK
