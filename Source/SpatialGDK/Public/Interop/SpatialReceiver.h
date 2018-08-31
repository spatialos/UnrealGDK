// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialTypebindingManager.h"
#include "SpatialNetDriver.h"
#include "SpatialActorChannel.h"
#include "SpatialPackageMapClient.h"
#include "CoreTypes/StandardLibrary.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

#include <memory>

#include "SpatialReceiver.generated.h"

using FChannelObjectPair = TPair<USpatialActorChannel*, UObject*>;
using FUnresolvedObjectsMap = TMap<Schema_FieldId, TSet<const UObject*>>;
struct FObjectReferences;
using FObjectReferencesMap = TMap<int32, FObjectReferences>;

struct PendingAddComponentWrapper
{
	PendingAddComponentWrapper() = default;
	PendingAddComponentWrapper(Worker_EntityId InEntityId, Worker_ComponentId InComponentId, std::shared_ptr<Component>& InData)
		: EntityId(InEntityId), ComponentId(InComponentId), Data(InData) {}

	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;
	std::shared_ptr<Component> Data;
};

struct FObjectReferences
{
	FObjectReferences() = default;
	FObjectReferences(FObjectReferences&& Other)
		: UnresolvedRefs(MoveTemp(Other.UnresolvedRefs))
		, bSingleProp(Other.bSingleProp)
		, Buffer(MoveTemp(Other.Buffer))
		, NumBufferBits(Other.NumBufferBits)
		, Array(MoveTemp(Other.Array))
		, ParentIndex(Other.ParentIndex)
		, Property(Other.Property) {}

	// Single property constructor
	FObjectReferences(const UnrealObjectRef& InUnresolvedRef, int32 InParentIndex, UProperty* InProperty)
		: bSingleProp(true), ParentIndex(InParentIndex), Property(InProperty)
	{
		UnresolvedRefs.Add(InUnresolvedRef);
	}

	// Struct (memory stream) constructor
	FObjectReferences(const TArray<uint8>& InBuffer, int32 InNumBufferBits, const TSet<UnrealObjectRef>& InUnresolvedRefs, int32 InParentIndex, UProperty* InProperty)
		: UnresolvedRefs(InUnresolvedRefs), bSingleProp(false), Buffer(InBuffer), NumBufferBits(InNumBufferBits), ParentIndex(InParentIndex), Property(InProperty) {}

	// Array constructor
	FObjectReferences(FObjectReferencesMap* InArray, int32 InParentIndex, UProperty* InProperty)
		: bSingleProp(false), Array(InArray), ParentIndex(InParentIndex), Property(InProperty) {}

	TSet<UnrealObjectRef>				UnresolvedRefs;

	bool								bSingleProp;
	TArray<uint8>						Buffer;
	int32								NumBufferBits;

	TUniquePtr<FObjectReferencesMap>	Array;
	int32								ParentIndex;
	UProperty*							Property;
};

UCLASS()
class USpatialReceiver : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver);

	// Dispatcher Calls
	void OnCriticalSection(bool InCriticalSection);
	void OnAddEntity(Worker_AddEntityOp& Op);
	void OnAddComponent(Worker_AddComponentOp& Op);
	void OnRemoveEntity(Worker_RemoveEntityOp& Op);

	void OnComponentUpdate(Worker_ComponentUpdateOp& Op);
	void OnCommandRequest(Worker_CommandRequestOp& Op);

	void OnReserveEntityIdResponse(Worker_ReserveEntityIdResponseOp& Op);
	void OnCreateEntityIdResponse(Worker_CreateEntityResponseOp& Op);
	void AddPendingActorRequest(Worker_RequestId RequestId);

	void CleanupDeletedEntity(Worker_EntityId EntityId);

private:
	void EnterCriticalSection();
	void LeaveCriticalSection();

	void CreateActor(Worker_EntityId EntityId);
	void RemoveActor(Worker_EntityId EntityId);
	AActor* SpawnNewEntity(Position* PositionComponent, UClass* ActorClass, bool bDeferred);
	UClass* GetNativeEntityClass(Metadata* MetadataComponent);

	void ApplyComponentData(Worker_EntityId EntityId, Worker_ComponentData& Data, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap);
	void ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* TargetObject, USpatialActorChannel* Channel, EReplicatedPropertyGroup PropertyGroup, bool bAutonomousProxy);

	void ResolvePendingOperations(UObject* Object, const UnrealObjectRef& ObjectRef);
	void ResolvePendingOperations_Internal(UObject* Object, const UnrealObjectRef& ObjectRef);

	UObject* GetTargetObjectFromChannelAndClass(USpatialActorChannel* Channel, UClass* Class);

	USpatialActorChannel* PopPendingActorRequest(Worker_RequestId RequestId);

private:
	USpatialNetDriver* NetDriver;
	UWorld* World;
	USpatialView* View;
	USpatialTypebindingManager* TypebindingManager;

	// TODO: Figure out how to remove entries when Channel/Actor gets deleted
	TMap<UnrealObjectRef, TSet<FChannelObjectPair>> IncomingRefsMap;
	TMap<FChannelObjectPair, FObjectReferencesMap> UnresolvedRefsMap;
	TArray<TPair<UObject*, UnrealObjectRef>> ResolvedObjectQueue;

	bool bInCriticalSection;
	TArray<Worker_EntityId> PendingAddEntities;
	TArray<PendingAddComponentWrapper> PendingAddComponents;
	TArray<Worker_EntityId> PendingRemoveEntities;


	TMap<Worker_RequestId, USpatialActorChannel*> PendingActorRequests;
};
