#pragma once

#include "ComponentIdentifier.h"
#include "EntityId.h"
#include "EntityPipelineBlock.h"

#include "SpatialInteropPipelineBlock.generated.h"

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
class USpatialActorChannel;

UCLASS(BlueprintType)
class NUF_API USpatialInteropPipelineBlock : public UEntityPipelineBlock
{
	GENERATED_BODY()

public:
	void Init(UEntityRegistry* Registry);

	void AddEntity(const worker::AddEntityOp& AddEntityOp) override;
	void RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp) override;

	void AddComponent(UAddComponentOpWrapperBase* AddComponentOp) override;
	void RemoveComponent(const worker::ComponentId ComponentId,
		const worker::RemoveComponentOp& RemoveComponentOp) override;

	void ChangeAuthority(const worker::ComponentId ComponentId,
		const worker::AuthorityChangeOp& AuthChangeOp) override;

private:
	UPROPERTY()
		UEntityRegistry* EntityRegistry;

	TArray<FEntityId> EntitiesToSpawn;
	TArray<FEntityId> EntitiesToRemove;
	// Maps ComponentId to USpatialOsComponent* class name
	UPROPERTY()
		TMap<int, UClass*> KnownComponents;
	UPROPERTY()
		TMap<FComponentIdentifier, UAddComponentOpWrapperBase*> ComponentsToAdd;
	UPROPERTY()
		TMap<FComponentIdentifier, USpatialOsComponent*> PendingUComponents;

	TSet<FComponentIdentifier> ComponentsToRemove;
	TMap<FComponentIdentifier, worker::AuthorityChangeOp> ComponentAuthorities;

	void ProcessOps(const TWeakPtr<worker::View>& InView,
		const TWeakPtr<worker::Connection>& InConnection, UWorld* World,
		UCallbackDispatcher* InCallbackDispatcher) override;

	void AddEntities(UWorld* World, const TWeakPtr<worker::Connection>& InConnection);
	void AddComponents(const TWeakPtr<worker::View>& InView,
		const TWeakPtr<worker::Connection>& InConnection,
		UCallbackDispatcher* InCallbackDispatcher);
	void RemoveComponents(UCallbackDispatcher* InCallbackDispatcher);
	void RemoveEntities(UWorld* World);

	AActor* SpawnNewEntity(UPositionAddComponentOp* PositionComponent, UWorld* World, UClass* ClassToSpawn);
	
	UClass* GetNativeEntityClass(UMetadataAddComponentOp * MetadataComponent);
	UClass* GetRegisteredEntityClass(UMetadataAddComponentOp * MetadataComponent);
	
	void SetupComponentInterests(AActor* Actor, const FEntityId& EntityId,
		const TWeakPtr<worker::Connection>& Connection);
};
