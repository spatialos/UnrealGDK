// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "improbable/c_worker.h"
#include "improbable/c_schema.h"

#include "SchemaHelpers.h"

struct PendingAddComponentWrapper
{
	PendingAddComponentWrapper() = default;
	PendingAddComponentWrapper(Worker_EntityId InEntityId, Worker_ComponentId InComponentId, std::unique_ptr<ComponentData> InData)
		: EntityId(InEntityId), ComponentId(InComponentId), Data(std::move(InData)) {}

	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;
	std::unique_ptr<ComponentData> Data;
};

struct CAPIPipelineBlock
{
	void EnterCriticalSection();
	void LeaveCriticalSection();

	void AddEntity(Worker_AddEntityOp& Op);
	void AddComponent(Worker_AddComponentOp& Op);

	void RemoveEntity(Worker_RemoveEntityOp& Op);
	void RemoveComponent(Worker_RemoveComponentOp& Op);

	void CreateActor(Worker_EntityId EntityId);
	void RemoveActor(Worker_EntityId EntityId);

	void CleanupDeletedEntity(Worker_EntityId EntityId);

	UClass* GetNativeEntityClass(MetadataData* MetadataComponent);

	AActor* SpawnNewEntity(PositionData* PositionComponent, UClass* ActorClass, bool bDeferred);

	class UWorld* World;
	class USpatialNetDriver* NetDriver;

	class UDTBManager* DTBManager;

	bool bInCriticalSection;
	TArray<Worker_EntityId> PendingAddEntities;
	TArray<PendingAddComponentWrapper> PendingAddComponents;

	TArray<Worker_EntityId> PendingRemoveEntities;
};
