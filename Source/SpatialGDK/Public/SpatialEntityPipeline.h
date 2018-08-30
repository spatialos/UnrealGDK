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

struct PendingAddComponentWrapper
{
	PendingAddComponentWrapper() = default;
	PendingAddComponentWrapper(Worker_EntityId InEntityId, Worker_ComponentId InComponentId, std::shared_ptr<Component>& InData)
		: EntityId(InEntityId), ComponentId(InComponentId), Data(InData) {}

	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;
	std::shared_ptr<Component> Data;
};

UCLASS()
class USpatialEntityPipeline : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* NetDriver);

	// Dispatcher Interface
	void OnCriticalSection(bool InCriticalSection);
	void OnAddEntity(Worker_AddEntityOp& Op);
	void OnAddComponent(Worker_AddComponentOp& Op);
	void OnRemoveEntity(Worker_RemoveEntityOp& Op);
	void OnReserveEntityIdResponse(Worker_ReserveEntityIdResponseOp& Op);
	void OnCreateEntityIdResponse(Worker_CreateEntityResponseOp& Op);

private:
	void EnterCriticalSection();
	void LeaveCriticalSection();

	void CreateActor(Worker_EntityId EntityId);
	AActor* SpawnNewEntity(Position* PositionComponent, UClass* ActorClass, bool bDeferred);
	void RemoveActor(Worker_EntityId EntityId);

	void CleanupDeletedEntity(Worker_EntityId EntityId);

	UClass* GetNativeEntityClass(Metadata* MetadataComponent);

	void ApplyComponentData(Worker_EntityId EntityId, Worker_ComponentData& Data, USpatialActorChannel* Channel, USpatialPackageMapClient* PackageMap);
	UObject* GetTargetObjectFromChannelAndClass(USpatialActorChannel* Channel, UClass* Class);

	void ApplyComponentUpdate(const Worker_ComponentUpdate& ComponentUpdate, UObject* TargetObject, USpatialActorChannel* Channel, EReplicatedPropertyGroup PropertyGroup, bool bAutonomousProxy);

private:

	class USpatialNetDriver* NetDriver;
	UWorld* World;

	bool bInCriticalSection;
	TArray<Worker_EntityId> PendingAddEntities;
	TArray<PendingAddComponentWrapper> PendingAddComponents;
	TArray<Worker_EntityId> PendingRemoveEntities;

	USpatialTypebindingManager* TypebindingManager;
};
