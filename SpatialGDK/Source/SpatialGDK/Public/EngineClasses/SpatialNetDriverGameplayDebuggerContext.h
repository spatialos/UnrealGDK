// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "Schema/GameplayDebuggerComponent.h"
#include "Schema/Interest.h"
#include "SpatialCommonTypes.h"
#include "UObject/WeakInterfacePtr.h"
#endif // WITH_GAMEPLAY_DEBUGGER

#include "SpatialNetDriverGameplayDebuggerContext.generated.h"

class AGameplayDebuggerCategoryReplicator;
class UGameplayDebuggerLBStrategy;
class USpatialNetDriver;

namespace SpatialGDK
{
class FSubView;
struct ComponentChange;
} // namespace SpatialGDK

/*
 * A helper object that allocates a custom LB strategy to handle gameplay debugger
 * replicated actors, through a custom subview that tracks actor add/remove/auth changes.
 */

UCLASS()
class SPATIALGDK_API USpatialNetDriverGameplayDebuggerContext : public UObject
{
	GENERATED_BODY()

public:
	USpatialNetDriverGameplayDebuggerContext() = default;

#if WITH_GAMEPLAY_DEBUGGER
	virtual ~USpatialNetDriverGameplayDebuggerContext();

	static void Enable(const SpatialGDK::FSubView& InSubView, USpatialNetDriver& InNetDriver);
	static void Disable(USpatialNetDriver& NetDriver);

	void Init(const SpatialGDK::FSubView& InSubView, USpatialNetDriver& InNetDriver);
	void Reset();

	/** Given an actor, return the delegated worker ID if the actor is a gameplay
	 * debugger replicator actor tracked by this context */
	TOptional<VirtualWorkerId> GetActorDelegatedWorkerId(const AActor& InActor);

	void AdvanceView();
	void TickServer();
#endif // WITH_GAMEPLAY_DEBUGGER

	UPROPERTY()
	UGameplayDebuggerLBStrategy* LBStrategy = nullptr;

protected:
#if WITH_GAMEPLAY_DEBUGGER
	struct FEntityData
	{
		SpatialGDK::GameplayDebuggerComponent Component;
		TWeakObjectPtr<AGameplayDebuggerCategoryReplicator> ReplicatorWeakObjectPtr;
		FString CurrentWorkerId;
		FDelegateHandle Handle;
	};

	void TrackEntity(Worker_EntityId InEntityId);
	void UntrackEntity(Worker_EntityId InEntityId);
	void AddAuthority(Worker_EntityId InEntityId, FEntityData* InOptionalEntityData);
	void RemoveAuthority(Worker_EntityId InEntityId, FEntityData* InOptionalEntityData);
	void RegisterServerRequestCallback(AGameplayDebuggerCategoryReplicator& InReplicator, FEntityData& InEntityData);
	void UnregisterServerRequestCallback(AGameplayDebuggerCategoryReplicator& InReplicator, FEntityData& InEntityData);
	void OnServerRequest(AGameplayDebuggerCategoryReplicator* InCategoryReplicator, FString InServerWorkerId);

	USpatialNetDriver* NetDriver = nullptr;
	const SpatialGDK::FSubView* SubView = nullptr;
	TMap<Worker_EntityId_Key, FEntityData> TrackedEntities;
	TSet<Worker_EntityId_Key> ComponentsAdded;
	TSet<Worker_EntityId_Key> ComponentsUpdated;
	TArray<Worker_EntityId_Key> ActorsAdded;
	TMap<FString, uint32> PhysicalToVirtualWorkerIdMap;
#endif // WITH_GAMEPLAY_DEBUGGER
};
