// // Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// // ===========
// // DO NOT EDIT - this file is automatically regenerated.
// // =========== 

// #pragma once

// #include "UObject/Class.h"
// #include "SpatialOSViewTypes.h"
// #include "SpatialOSWorkerTypes.h"
// #include "ComponentIdentifier.h"
// #include "ComponentUpdateQueue.h"
// #include "improbable/unreal/level_data.h"
// #include "improbable/unreal/player.h"
// #include "improbable/unreal/spawner.h"
// #include "improbable/unreal/unreal_metadata.h"
// #include "improbable/unreal/generated/UnrealCharacter.h"
// #include "improbable/unreal/generated/UnrealPlayerController.h"
// #include "improbable/unreal/generated/UnrealPlayerState.h"
// #include "improbable/unreal/generated/UnrealWheeledVehicle.h"
// #include "improbable/standard_library.h"
// #include "ScopedViewCallbacks.h"
// #include "SpatialOS.h"
// #include "CallbackDispatcher.generated.h"

// class UComponentUpdateOpWrapperBase;

// DECLARE_EVENT_TwoParams( UCallbackDispatcher, FAuthorityChangeOpReceivedEvent, worker::ComponentId, const worker::AuthorityChangeOp&);

// UCLASS()
// class SPATIALGDK_API UCallbackDispatcher : public UObject
// {
// 	GENERATED_BODY()

// public:
// 	UCallbackDispatcher();

// 	/**
// 	* Initialise the UCallbackDispatcher. Calling Init() more than once results in an error.
// 	*/
// 	void Init(const TWeakPtr<SpatialOSView>& InView);

// 	/**
// 	* Reset the UCallbackDispatcher to its initial state. Init() may be called again after this method is called.
// 	*/
// 	void Reset();

// 	void AddComponentUpdateCallback(worker::EntityId EntityId, worker::ComponentId ComponentId, TFunction<void(UComponentUpdateOpWrapperBase&)> Callback);
// 	void AddAuthorityChangeCallback(worker::EntityId EntityId, worker::ComponentId ComponentId, TFunction<void(const worker::AuthorityChangeOp&)> Callback);

// 	void RemoveComponentUpdateCallback(worker::EntityId EntityId, worker::ComponentId ComponentId);
// 	void RemoveAuthorityChangeCallback(worker::EntityId EntityId, worker::ComponentId ComponentId);

// 	FAuthorityChangeOpReceivedEvent OnAuthorityChangeOpReceived;

// 	void OnUnrealLevelComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealLevel>& Op);
// 	void OnUnrealLevelAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealLevelPlaceholderComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealLevelPlaceholder>& Op);
// 	void OnUnrealLevelPlaceholderAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnPlayerControlClientComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::PlayerControlClient>& Op);
// 	void OnPlayerControlClientAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnPlayerSpawnerComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::PlayerSpawner>& Op);
// 	void OnPlayerSpawnerAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealMetadataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealMetadata>& Op);
// 	void OnUnrealMetadataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealCharacterSingleClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterSingleClientRepData>& Op);
// 	void OnUnrealCharacterSingleClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealCharacterMultiClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterMultiClientRepData>& Op);
// 	void OnUnrealCharacterMultiClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealCharacterMigratableDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterMigratableData>& Op);
// 	void OnUnrealCharacterMigratableDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealCharacterClientRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterClientRPCs>& Op);
// 	void OnUnrealCharacterClientRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealCharacterServerRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterServerRPCs>& Op);
// 	void OnUnrealCharacterServerRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealPlayerControllerSingleClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerSingleClientRepData>& Op);
// 	void OnUnrealPlayerControllerSingleClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealPlayerControllerMultiClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerMultiClientRepData>& Op);
// 	void OnUnrealPlayerControllerMultiClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealPlayerControllerMigratableDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerMigratableData>& Op);
// 	void OnUnrealPlayerControllerMigratableDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealPlayerControllerClientRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerClientRPCs>& Op);
// 	void OnUnrealPlayerControllerClientRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealPlayerControllerServerRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerServerRPCs>& Op);
// 	void OnUnrealPlayerControllerServerRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealPlayerStateSingleClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateSingleClientRepData>& Op);
// 	void OnUnrealPlayerStateSingleClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealPlayerStateMultiClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateMultiClientRepData>& Op);
// 	void OnUnrealPlayerStateMultiClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealPlayerStateMigratableDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateMigratableData>& Op);
// 	void OnUnrealPlayerStateMigratableDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealPlayerStateClientRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateClientRPCs>& Op);
// 	void OnUnrealPlayerStateClientRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealPlayerStateServerRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateServerRPCs>& Op);
// 	void OnUnrealPlayerStateServerRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealWheeledVehicleSingleClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealWheeledVehicleSingleClientRepData>& Op);
// 	void OnUnrealWheeledVehicleSingleClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealWheeledVehicleMultiClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealWheeledVehicleMultiClientRepData>& Op);
// 	void OnUnrealWheeledVehicleMultiClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealWheeledVehicleMigratableDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealWheeledVehicleMigratableData>& Op);
// 	void OnUnrealWheeledVehicleMigratableDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealWheeledVehicleClientRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealWheeledVehicleClientRPCs>& Op);
// 	void OnUnrealWheeledVehicleClientRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnUnrealWheeledVehicleServerRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealWheeledVehicleServerRPCs>& Op);
// 	void OnUnrealWheeledVehicleServerRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnEntityAclComponentUpdateOp(const worker::ComponentUpdateOp<improbable::EntityAcl>& Op);
// 	void OnEntityAclAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnMetadataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::Metadata>& Op);
// 	void OnMetadataAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnPositionComponentUpdateOp(const worker::ComponentUpdateOp<improbable::Position>& Op);
// 	void OnPositionAuthorityChangeOp(const worker::AuthorityChangeOp& Op);

// 	void OnPersistenceComponentUpdateOp(const worker::ComponentUpdateOp<improbable::Persistence>& Op);
// 	void OnPersistenceAuthorityChangeOp(const worker::AuthorityChangeOp& Op);


// private:
// 	TMap<FComponentIdentifier, TFunction<void(UComponentUpdateOpWrapperBase&)>> ComponentUpdateCallbacks;
// 	TMap<FComponentIdentifier, TFunction<void(const worker::AuthorityChangeOp&)>> AuthorityChangeCallbacks;
	
// 	UPROPERTY()
// 	TMap<FComponentIdentifier, FComponentUpdateQueue> QueuedUpdates;
	
// 	void HandleUpdate(UComponentUpdateOpWrapperBase* Update);
// 	void EnqueueUpdate(const FComponentIdentifier& Id, UComponentUpdateOpWrapperBase* Update);
// 	void DispatchQueuedUpdates(const FComponentIdentifier& Id);

// 	improbable::unreal::callbacks::FScopedViewCallbacks Callbacks;
// 	bool bInitialised;
// };