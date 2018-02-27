#pragma once

#include "ComponentIdentifier.h"
#include "EntityId.h"
#include "EntityPipelineBlock.h"
#include "ComponentId.h"

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
class USpatialNetDriver;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOSInteropPipelineBlock, Log, All);

UCLASS(BlueprintType)
class NUF_API USpatialInteropPipelineBlock : public UEntityPipelineBlock
{
	GENERATED_BODY()

public:
	void Init(UEntityRegistry* Registry, USpatialNetDriver* Driver);

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
	TSet<FEntityId> PendingAddEntities;

	UPROPERTY()
	TMap<FComponentIdentifier, UAddComponentOpWrapperBase*> PendingAddComponents;

	TMap<FComponentIdentifier, worker::AuthorityChangeOp> PendingAuthorityChanges;

	UPROPERTY()
	TSet<FComponentIdentifier> PendingRemoveComponents;

	UPROPERTY()
	TSet<FEntityId> PendingRemoveEntities;

	UPROPERTY()
	UEntityRegistry* EntityRegistry;

	UPROPERTY()
	USpatialNetDriver* NetDriver;

	// Maps ComponentId to USpatialOsComponent* class name
	UPROPERTY()
	TMap<FComponentId, UClass*> KnownComponents;

private:
	AActor* GetOrCreateActor(TSharedPtr<worker::Connection> LockedConnection, TSharedPtr<worker::View> LockedView, const FEntityId& EntityId);
	AActor* SpawnNewEntity(improbable::PositionData* PositionComponent, UWorld* World, UClass* ClassToSpawn);
	
	UClass* GetNativeEntityClass(improbable::MetadataData* MetadataComponent);
	UClass* GetRegisteredEntityClass(improbable::MetadataData* MetadataComponent);

	void ProcessOps(const TWeakPtr<SpatialOSView>& InView,
		const TWeakPtr<SpatialOSConnection>& InConnection, UWorld* World,
		::UCallbackDispatcher* CallbackDispatcher) override;
	
	void SetupComponentInterests(AActor* Actor, const FEntityId& EntityId, const TWeakPtr<worker::Connection>& Connection);

	template <typename AddOpType, typename Metaclass>
	typename Metaclass::Data* GetPendingComponentData(const FEntityId& EntityId)
	{
		const auto ComponentId = Metaclass::ComponentId;
		UAddComponentOpWrapperBase** BaseAddComponent = PendingAddComponents.Find(FComponentIdentifier{EntityId.ToSpatialEntityId(), ComponentId});
		if (BaseAddComponent && (*BaseAddComponent)->IsValidLowLevel())
		{
			return Cast<AddOpType>(*BaseAddComponent)->Data.data();
		}
		else
		{
			return nullptr;
		}
	}

	template <typename Metaclass>
	typename Metaclass::Data* GetComponentDataFromView(TSharedPtr<worker::View> LockedView, const FEntityId& EntityId)
	{
		auto EntityIterator = LockedView->Entities.find(EntityId.ToSpatialEntityId());
		if (EntityIterator != LockedView->Entities.end())
		{
			return EntityIterator->second.Get<Metaclass>().data();
		}
		else
		{
			return nullptr;
		}
	}
};
