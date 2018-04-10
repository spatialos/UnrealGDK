// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
#include "EntityPipelineBlock.h"
#include "ScopedViewCallbacks.h"
// #include "UnrealLevelAddComponentOp.h"
// #include "UnrealLevelPlaceholderAddComponentOp.h"
// #include "PlayerControlClientAddComponentOp.h"
// #include "PlayerSpawnerAddComponentOp.h"
// #include "UnrealMetadataAddComponentOp.h"
// #include "UnrealCharacterSingleClientRepDataAddComponentOp.h"
// #include "UnrealCharacterMultiClientRepDataAddComponentOp.h"
// #include "UnrealCharacterMigratableDataAddComponentOp.h"
// #include "UnrealCharacterClientRPCsAddComponentOp.h"
// #include "UnrealCharacterServerRPCsAddComponentOp.h"
// #include "UnrealPlayerControllerSingleClientRepDataAddComponentOp.h"
// #include "UnrealPlayerControllerMultiClientRepDataAddComponentOp.h"
// #include "UnrealPlayerControllerMigratableDataAddComponentOp.h"
// #include "UnrealPlayerControllerClientRPCsAddComponentOp.h"
// #include "UnrealPlayerControllerServerRPCsAddComponentOp.h"
// #include "UnrealPlayerStateSingleClientRepDataAddComponentOp.h"
// #include "UnrealPlayerStateMultiClientRepDataAddComponentOp.h"
// #include "UnrealPlayerStateMigratableDataAddComponentOp.h"
// #include "UnrealPlayerStateClientRPCsAddComponentOp.h"
// #include "UnrealPlayerStateServerRPCsAddComponentOp.h"
// #include "UnrealWheeledVehicleSingleClientRepDataAddComponentOp.h"
// #include "UnrealWheeledVehicleMultiClientRepDataAddComponentOp.h"
// #include "UnrealWheeledVehicleMigratableDataAddComponentOp.h"
// #include "UnrealWheeledVehicleClientRPCsAddComponentOp.h"
// #include "UnrealWheeledVehicleServerRPCsAddComponentOp.h"
// #include "EntityAclAddComponentOp.h"
// #include "MetadataAddComponentOp.h"
// #include "PositionAddComponentOp.h"
// #include "PersistenceAddComponentOp.h"
#include "EntityPipeline.generated.h"

UCLASS()
class SPATIALGDK_API UEntityPipeline : public UObject
{
	GENERATED_BODY()

public:
	UEntityPipeline();

	/**
	* Initialise the UEntityPipeline. Calling Init() more than once results in an error.
	*/
	void Init(const TWeakPtr<SpatialOSView>& InView);

	/**
	* Deregister all callbacks. Init() may be called again after this method is called.
	*/
	void DeregisterAllCallbacks();

	void AddBlock(UEntityPipelineBlock* NewBlock);
	void ProcessOps(const TWeakPtr<SpatialOSView>& InView, const TWeakPtr<SpatialOSConnection>& InConnection, UWorld* World);

	void OnAddEntity(const worker::AddEntityOp& Op) { FirstBlock->AddEntity(Op); }
	void OnRemoveEntity(const worker::RemoveEntityOp& Op) { FirstBlock->RemoveEntity(Op); }
	void OnCriticalSection(const worker::CriticalSectionOp& Op) { if (Op.InCriticalSection) { FirstBlock->EnterCriticalSection(); } else { FirstBlock->LeaveCriticalSection(); } }
	void OnRemoveComponent(const worker::ComponentId ComponentId, const worker::RemoveComponentOp& Op) { FirstBlock->RemoveComponent(ComponentId, Op); }
	void OnAuthorityChange(const worker::ComponentId ComponentId, const worker::AuthorityChangeOp& Op) { FirstBlock->ChangeAuthority(ComponentId, Op); }

	// void AddUnrealLevelComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealLevel>& Op);
	// void RemoveUnrealLevelComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealLevelPlaceholderComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealLevelPlaceholder>& Op);
	// void RemoveUnrealLevelPlaceholderComponentOp(const worker::RemoveComponentOp& Op);
	// void AddPlayerControlClientComponentOp(const worker::AddComponentOp<improbable::unreal::PlayerControlClient>& Op);
	// void RemovePlayerControlClientComponentOp(const worker::RemoveComponentOp& Op);
	// void AddPlayerSpawnerComponentOp(const worker::AddComponentOp<improbable::unreal::PlayerSpawner>& Op);
	// void RemovePlayerSpawnerComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealMetadataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealMetadata>& Op);
	// void RemoveUnrealMetadataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealCharacterSingleClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealCharacterSingleClientRepData>& Op);
	// void RemoveUnrealCharacterSingleClientRepDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealCharacterMultiClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealCharacterMultiClientRepData>& Op);
	// void RemoveUnrealCharacterMultiClientRepDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealCharacterMigratableDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealCharacterMigratableData>& Op);
	// void RemoveUnrealCharacterMigratableDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealCharacterClientRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealCharacterClientRPCs>& Op);
	// void RemoveUnrealCharacterClientRPCsComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealCharacterServerRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealCharacterServerRPCs>& Op);
	// void RemoveUnrealCharacterServerRPCsComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealPlayerControllerSingleClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerSingleClientRepData>& Op);
	// void RemoveUnrealPlayerControllerSingleClientRepDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealPlayerControllerMultiClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerMultiClientRepData>& Op);
	// void RemoveUnrealPlayerControllerMultiClientRepDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealPlayerControllerMigratableDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerMigratableData>& Op);
	// void RemoveUnrealPlayerControllerMigratableDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealPlayerControllerClientRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerClientRPCs>& Op);
	// void RemoveUnrealPlayerControllerClientRPCsComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealPlayerControllerServerRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerServerRPCs>& Op);
	// void RemoveUnrealPlayerControllerServerRPCsComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealPlayerStateSingleClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerStateSingleClientRepData>& Op);
	// void RemoveUnrealPlayerStateSingleClientRepDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealPlayerStateMultiClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerStateMultiClientRepData>& Op);
	// void RemoveUnrealPlayerStateMultiClientRepDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealPlayerStateMigratableDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerStateMigratableData>& Op);
	// void RemoveUnrealPlayerStateMigratableDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealPlayerStateClientRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerStateClientRPCs>& Op);
	// void RemoveUnrealPlayerStateClientRPCsComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealPlayerStateServerRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerStateServerRPCs>& Op);
	// void RemoveUnrealPlayerStateServerRPCsComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealWheeledVehicleSingleClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealWheeledVehicleSingleClientRepData>& Op);
	// void RemoveUnrealWheeledVehicleSingleClientRepDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealWheeledVehicleMultiClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealWheeledVehicleMultiClientRepData>& Op);
	// void RemoveUnrealWheeledVehicleMultiClientRepDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealWheeledVehicleMigratableDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealWheeledVehicleMigratableData>& Op);
	// void RemoveUnrealWheeledVehicleMigratableDataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealWheeledVehicleClientRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealWheeledVehicleClientRPCs>& Op);
	// void RemoveUnrealWheeledVehicleClientRPCsComponentOp(const worker::RemoveComponentOp& Op);
	// void AddUnrealWheeledVehicleServerRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealWheeledVehicleServerRPCs>& Op);
	// void RemoveUnrealWheeledVehicleServerRPCsComponentOp(const worker::RemoveComponentOp& Op);
	// void AddEntityAclComponentOp(const worker::AddComponentOp<improbable::EntityAcl>& Op);
	// void RemoveEntityAclComponentOp(const worker::RemoveComponentOp& Op);
	// void AddMetadataComponentOp(const worker::AddComponentOp<improbable::Metadata>& Op);
	// void RemoveMetadataComponentOp(const worker::RemoveComponentOp& Op);
	// void AddPositionComponentOp(const worker::AddComponentOp<improbable::Position>& Op);
	// void RemovePositionComponentOp(const worker::RemoveComponentOp& Op);
	// void AddPersistenceComponentOp(const worker::AddComponentOp<improbable::Persistence>& Op);
	// void RemovePersistenceComponentOp(const worker::RemoveComponentOp& Op);

private:
	UPROPERTY()
	UEntityPipelineBlock* FirstBlock;
	UPROPERTY()
	UEntityPipelineBlock* LastBlock;
	// UPROPERTY()
	// UCallbackDispatcher* CallbackDispatcher;

	bool bInitialised;
	improbable::unreal::callbacks::FScopedViewCallbacks Callbacks;
};
