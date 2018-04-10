#pragma once

#include "ComponentIdentifier.h"
#include "EntityId.h"
#include "EntityPipelineBlock.h"
#include "ComponentId.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialInteropPipelineBlock.generated.h"

namespace worker
{
	struct AddEntityOp;
	struct RemoveEntityOp;
	struct RemoveComponentOp;
}

namespace improbable
{
	class MetadataData;
	class PositionData;
}

class UAddComponentOpWrapperBase;
class UMetadataAddComponentOp;
class UPositionAddComponentOp;
class UCallbackDispatcher;
class UEntityRegistry;
class UEntityPipeline;
class USpatialOsComponent;
class USpatialActorChannel;
class USpatialNetDriver;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSInteropPipelineBlock, Log, All);

struct FPendingAddComponentWrapper
{
	FComponentIdentifier EntityComponent;
	AddComponentOpWrapperBase* AddComponentOp;
};

UCLASS(BlueprintType)
class SPATIALGDK_API USpatialInteropPipelineBlock : public UEntityPipelineBlock
{
	GENERATED_BODY()

public:
	void Init(UEntityRegistry* Registry, USpatialNetDriver* Driver, UWorld* LoadedWorld);

	void AddEntity(const worker::AddEntityOp& AddEntityOp) override;
	void RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp) override;

	void AddComponent(UAddComponentOpWrapperBase* AddComponentOp) override;
	void RemoveComponent(const worker::ComponentId ComponentId, const worker::RemoveComponentOp& RemoveComponentOp) override;

	void ChangeAuthority(const worker::ComponentId ComponentId, const worker::AuthorityChangeOp& AuthChangeOp) override;

	void EnterCriticalSection() override;
	void LeaveCriticalSection() override;

private:
	bool bInCriticalSection;

	UPROPERTY()
	TArray<FEntityId> PendingAddEntities;

	UPROPERTY()
	TArray<FPendingAddComponentWrapper> PendingAddComponents;

	// TMap<FComponentIdentifier, worker::AuthorityChangeOp> PendingAuthorityChanges;

	UPROPERTY()
	TArray<FComponentIdentifier> PendingRemoveComponents;

	UPROPERTY()
	TArray<FEntityId> PendingRemoveEntities;

	// UPROPERTY()
	// UEntityRegistry* EntityRegistry;

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UWorld* World;

	// Maps ComponentId to USpatialOsComponent* class name
	UPROPERTY()
	TMap<FComponentId, UClass*> KnownComponents;

private:
	void AddEntityImpl(const FEntityId& EntityId);
	void InitialiseNewComponentImpl(const FComponentIdentifier& ComponentIdentifier, UAddComponentOpWrapperBase* AddComponentOp);
	void DisableComponentImpl(const FComponentIdentifier& ComponentIdentifier);
	void RemoveEntityImpl(const FEntityId& EntityId);

	// Stub.
	void ProcessOps(const TWeakPtr<SpatialOSView>& InView,
		const TWeakPtr<SpatialOSConnection>& InConnection, UWorld* World) override;

private:
	AActor* GetOrCreateActor(TSharedPtr<worker::Connection> LockedConnection, TSharedPtr<worker::View> LockedView, const FEntityId& EntityId);
	AActor* SpawnNewEntity(improbable::PositionData* PositionComponent, UClass* ClassToSpawn);
	
	UClass* GetNativeEntityClass(improbable::MetadataData* MetadataComponent);
	UClass* GetRegisteredEntityClass(improbable::MetadataData* MetadataComponent);
	
//	void SetupComponentInterests(AActor* Actor, const FEntityId& EntityId, const TWeakPtr<worker::Connection>& Connection);

	template <typename AddOpType, typename Metaclass>
	typename Metaclass::Data* GetPendingComponentData(const FEntityId& EntityId)
	{
		const auto ComponentId = Metaclass::ComponentId;
		for (FPendingAddComponentWrapper& PendingAddComponent : PendingAddComponents)
		{
			if (PendingAddComponent.EntityComponent == FComponentIdentifier{EntityId.ToSpatialEntityId(), ComponentId})
			{
				return PendingAddComponent.AddComponentOp->IsValidLowLevel() ? Cast<AddOpType>(*PendingAddComponent.AddComponentOp)->Data.data() : nullptr;
			}
		}
		return nullptr;
	}

	template <typename Metaclass>
	typename Metaclass::Data* GetComponentDataFromView(TSharedPtr<worker::View> LockedView, const FEntityId& EntityId)
	{
		auto EntityIterator = LockedView->Entities.find(EntityId.ToSpatialEntityId());
		if (EntityIterator == LockedView->Entities.end())
		{
			return nullptr;
		}
		return EntityIterator->second.Get<Metaclass>().data();
	}
};
