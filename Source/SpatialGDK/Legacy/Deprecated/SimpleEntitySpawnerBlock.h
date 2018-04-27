// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ComponentAddOpQueueWrapper.h"
#include "ComponentIdentifier.h"
#include "EntityId.h"
#include "EntityPipelineBlock.h"
#include "SimpleEntitySpawnerBlock.generated.h"

namespace worker
{
struct AddEntityOp;
struct RemoveEntityOp;
struct RemoveComponentOp;
}

class UAddComponentOpWrapperBase;
class UMetadataAddComponentOp;
class UPositionAddComponentOp;
class UCallbackDispatcher;
class UEntityRegistry;
class UEntityPipeline;
class USpatialOsComponent;

UCLASS(BlueprintType)
class SPATIALGDK_API USimpleEntitySpawnerBlock : public UEntityPipelineBlock
{
	GENERATED_BODY()

  public:
	void Init(UEntityRegistry* Registry);

	void AddEntity(const worker::AddEntityOp& AddEntityOp) override;
	void RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp) override;

	void AddComponent(UAddComponentOpWrapperBase* AddComponentOp) override;
	void RemoveComponent(const worker::ComponentId ComponentId,
						 const worker::RemoveComponentOp& RemoveComponentOp) override;

	void ChangeAuthority(const worker::ComponentId ComponentId, const worker::AuthorityChangeOp& AuthChangeOp) override;

  private:
	enum class ESpatialOperationType
	{
		AddEntity,
		RemoveEntity,
		AddComponent,
		RemoveComponent
	};

	struct FSpatialOperation
	{
		ESpatialOperationType OperationType;
		FComponentIdentifier ComponentIdentifier;

		explicit FSpatialOperation(const worker::AddEntityOp& AddEntityOp);
		explicit FSpatialOperation(const worker::RemoveEntityOp& RemoveEntityOp);
		explicit FSpatialOperation(ESpatialOperationType OperationType, FComponentIdentifier ComponentIdentifer);
		explicit FSpatialOperation(UAddComponentOpWrapperBase* AddComponentOp);
		explicit FSpatialOperation(const worker::ComponentId ComponentId,
								   const worker::RemoveComponentOp& RemoveComponentOp);
		bool operator==(const FSpatialOperation& Other) const;
	};

	UPROPERTY()
	UEntityRegistry* EntityRegistry;

	// Maps ComponentId to USpatialOsComponent* class name
	UPROPERTY()
	TMap<int, UClass*> KnownComponents;

	UPROPERTY()
	TMap<FComponentIdentifier, FComponentAddOpQueueWrapper> ComponentsToAdd;

	TMap<FComponentIdentifier, worker::AuthorityChangeOp> ComponentAuthorities;

	TMap<FEntityId, TArray<FSpatialOperation>> QueuedOps;

	TArray<FEntityId> EmptyOpsQueues;

	void ProcessOps(const TWeakPtr<SpatialOSView>& InView, const TWeakPtr<SpatialOSConnection>& InConnection,
					UWorld* World, UCallbackDispatcher* InCallbackDispatcher) override;
	bool ProcessOp(const TWeakPtr<SpatialOSView>& InView, const TWeakPtr<SpatialOSConnection>& InConnection,
				   UWorld* World, UCallbackDispatcher* InCallbackDispatcher, FSpatialOperation SpatialOperation);

	bool ProcessAddEntityOp(UWorld* World, const TWeakPtr<SpatialOSConnection>& InConnection,
							const FSpatialOperation& SpatialOperation);
	bool ProcessAddComponentOp(const TWeakPtr<SpatialOSView>& InView, const TWeakPtr<SpatialOSConnection>& InConnection,
							   UCallbackDispatcher* InCallbackDispatcher, const FSpatialOperation& SpatialOperation);
	bool ProcessRemoveComponentOp(UCallbackDispatcher* InCallbackDispatcher, const FSpatialOperation& SpatialOperation);
	bool ProcessRemoveEntityOp(UWorld* World, const FSpatialOperation& SpatialOperation);

	void QueueOp(FSpatialOperation SpatialOperation);

	AActor* SpawnNewEntity(UMetadataAddComponentOp* MetadataComponent, UPositionAddComponentOp* PositionComponent,
						   UWorld* World);
	void SetupComponentInterests(AActor* Actor, const FEntityId& EntityId,
								 const TWeakPtr<SpatialOSConnection>& Connection);
};
