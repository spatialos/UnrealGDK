// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ComponentIdentifier.h"
#include "EntityId.h"
#include "EntityPipelineBlock.h"
#include "SpatialShadowActor.h"

#include "SpatialShadowActorPipelineBlock.generated.h"

namespace worker {
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
class USpatialNetDriver;

UCLASS()
class NUF_API USpatialShadowActorPipelineBlock : public UEntityPipelineBlock {
	GENERATED_BODY()

public:
	void Init(UEntityRegistry* Registry);

	void AddEntity(const worker::AddEntityOp& AddEntityOp) override;
	void RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp) override;

	void AddComponent(UAddComponentOpWrapperBase* AddComponentOp) override;
	void RemoveComponent(const worker::ComponentId ComponentId, const worker::RemoveComponentOp& RemoveComponentOp) override;

	void ChangeAuthority(const worker::ComponentId ComponentId, const worker::AuthorityChangeOp& AuthChangeOp) override;

	ASpatialShadowActor* GetShadowActor(const FEntityId& EntityId) const;

	void ReplicateShadowActorChanges(float DeltaTime);

	UPROPERTY()
	USpatialNetDriver* NetDriver;

private:
	UPROPERTY()
	UEntityRegistry* EntityRegistry;

	UPROPERTY()
	TMap<FEntityId, ASpatialShadowActor*> ShadowActors;

	TArray<FEntityId> PendingAddEntity;
	TArray<FEntityId> PendingRemoveEntity;
	// Maps ComponentId to USpatialOsComponent* class name
	UPROPERTY()
		TMap<int, UClass*> KnownComponents;
	UPROPERTY()
		TMap<FComponentIdentifier, UAddComponentOpWrapperBase*> PendingAddComponentMap;
	UPROPERTY()
		TMap<FComponentIdentifier, USpatialOsComponent*> PendingUComponents;
	TSet<FComponentIdentifier> PendingRemoveComponent;
	TMap<FComponentIdentifier, worker::AuthorityChangeOp> PendingAuthorityChange;

private:
	void ProcessOps(const TWeakPtr<worker::View>& InView,
		const TWeakPtr<worker::Connection>& InConnection, UWorld* World,
		UCallbackDispatcher* InCallbackDispatcher) override;

	void AddEntities(
		UWorld* World,
		const TWeakPtr<worker::View>& InView,
		const TWeakPtr<worker::Connection>& InConnection,
		UCallbackDispatcher* InCallbackDispatcher);

	void RemoveEntities(UWorld* World);

	UAddComponentOpWrapperBase* GetPendingAddComponent(const FEntityId& EntityId, const worker::ComponentId& ComponentId);

	ASpatialShadowActor* TrySpawnShadowActor(
		const FEntityId& EntityId,
		UClass* EntityClass,
		UAddComponentOpWrapperBase* ReplicatedDataComponent,
		UAddComponentOpWrapperBase* CompleteDataComponent,
		UWorld* World,
		const TWeakPtr<worker::View>& InView,
		const TWeakPtr<worker::Connection>& InConnection,
		UCallbackDispatcher* InCallbackDispatcher,
		AActor* PairedEntity);
};
