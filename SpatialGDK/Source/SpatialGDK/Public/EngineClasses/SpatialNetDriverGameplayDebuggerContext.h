// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Schema/GameplayDebuggerComponent.h"
#include "Schema/Interest.h"
#include "SpatialCommonTypes.h"
#include "UObject/WeakInterfacePtr.h"

#include "SpatialNetDriverGameplayDebuggerContext.generated.h"

class AGameplayDebuggerCategoryReplicator;
class UGameplayDebuggerLBStrategy;
class USpatialNetDriver;

namespace SpatialGDK
{
	class FSubView;
	struct ComponentChange;
} // namespace SpatialGDK

UCLASS()
class SPATIALGDK_API USpatialNetDriverGameplayDebuggerContext : public UObject
{
	GENERATED_BODY()

public:
	static void Enable(const SpatialGDK::FSubView& InSubView, USpatialNetDriver& NetDriver);
	static void Disable(USpatialNetDriver& NetDriver);

	void Init(const SpatialGDK::FSubView& InSubView, USpatialNetDriver& InNetDriver);
	void Cleanup();
	void Reset();

	TOptional<VirtualWorkerId> GetActorDelegatedWorkerId(const AActor& Actor);

	void AdvanceView();
	void TickServer();

	SpatialGDK::QueryConstraint ComputeAdditionalEntityQueryConstraint() const;

	void ClearNeedEntityInterestUpdate();

	UPROPERTY()
	UGameplayDebuggerLBStrategy* LBStrategy = nullptr;

protected:
	void TrackEntity(Worker_EntityId EntityId);
	void UntrackEntity(Worker_EntityId EntityId);
	void OnComponentChange(Worker_EntityId EntityId, const SpatialGDK::ComponentChange& Change);
	void ApplyComponentUpdate(Worker_EntityId EntityId, Schema_ComponentUpdate* Update);
	bool NeedEntityInterestUpdate() const;
	void OnServerWorkerIdChange(AGameplayDebuggerCategoryReplicator* InCategoryReplicator, FString InServerWorkerId);

	struct FEntityData
	{
		SpatialGDK::GameplayDebuggerComponent Component;
		FString CurrentWorkerId;
		FDelegateHandle Handle;
	};

	USpatialNetDriver* NetDriver = nullptr;
	const SpatialGDK::FSubView* SubView = nullptr;
	TMap<Worker_EntityId_Key, FEntityData> TrackedEntities;
	TSet<Worker_EntityId_Key> ComponentsAdded;
	TArray<Worker_EntityId_Key> ActorsAdded;
	TMap<FString, uint32> PhysicalToVirtualWorkerIdMap;
	bool bNeedToUpdateInterest = false;
};
