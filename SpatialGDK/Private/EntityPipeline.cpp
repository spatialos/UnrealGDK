// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "EntityPipeline.h"
#include "EntityPipelineBlock.h"
#include "CoreMinimal.h"

#include "SpatialGDKCommon.h"

#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"
//#include "CallbackDispatcher.h"
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

/**
*
*/

DECLARE_LOG_CATEGORY_EXTERN(LogEntityPipeline, Log, All);
DEFINE_LOG_CATEGORY(LogEntityPipeline);

UEntityPipeline::UEntityPipeline()
: FirstBlock(nullptr)
, LastBlock(nullptr)
//, CallbackDispatcher(nullptr)
, bInitialised(false)
{
}

void UEntityPipeline::Init(const TWeakPtr<SpatialOSView>& InView)
{
	checkf(!bInitialised, TEXT("Attempting to call Init more than once!"));
	checkf(FirstBlock, TEXT("Trying to bind callbacks but no blocks have been added!"));

	Callbacks.Init(InView);
	// CallbackDispatcher = InCallbackDispatcher;
	// CallbackDispatcher->OnAuthorityChangeOpReceived.AddUObject(this, &UEntityPipeline::OnAuthorityChange);

	auto LockedView = InView.Pin();
	if (LockedView.IsValid())
	{
		Callbacks.Add(LockedView->OnAddEntity(
			std::bind(&UEntityPipeline::OnAddEntity, this, std::placeholders::_1)));

		Callbacks.Add(LockedView->OnRemoveEntity(
			std::bind(&UEntityPipeline::OnRemoveEntity, this, std::placeholders::_1)));

		Callbacks.Add(LockedView->OnCriticalSection(
			std::bind(&UEntityPipeline::OnCriticalSection, this, std::placeholders::_1)));

		// BindFunc Binder{Callbacks, LockedView, this};
		// worker::ForEachComponent(improbable::unreal::Components{}, Binder);

		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealLevel>(
			std::bind(&UEntityPipeline::AddUnrealLevelComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealLevelPlaceholder>(
			std::bind(&UEntityPipeline::AddUnrealLevelPlaceholderComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::PlayerControlClient>(
			std::bind(&UEntityPipeline::AddPlayerControlClientComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::PlayerSpawner>(
			std::bind(&UEntityPipeline::AddPlayerSpawnerComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealMetadata>(
			std::bind(&UEntityPipeline::AddUnrealMetadataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealCharacterSingleClientRepData>(
			std::bind(&UEntityPipeline::AddUnrealCharacterSingleClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealCharacterMultiClientRepData>(
			std::bind(&UEntityPipeline::AddUnrealCharacterMultiClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealCharacterMigratableData>(
			std::bind(&UEntityPipeline::AddUnrealCharacterMigratableDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealCharacterClientRPCs>(
			std::bind(&UEntityPipeline::AddUnrealCharacterClientRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealCharacterServerRPCs>(
			std::bind(&UEntityPipeline::AddUnrealCharacterServerRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealPlayerControllerSingleClientRepData>(
			std::bind(&UEntityPipeline::AddUnrealPlayerControllerSingleClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealPlayerControllerMultiClientRepData>(
			std::bind(&UEntityPipeline::AddUnrealPlayerControllerMultiClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealPlayerControllerMigratableData>(
			std::bind(&UEntityPipeline::AddUnrealPlayerControllerMigratableDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealPlayerControllerClientRPCs>(
			std::bind(&UEntityPipeline::AddUnrealPlayerControllerClientRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealPlayerControllerServerRPCs>(
			std::bind(&UEntityPipeline::AddUnrealPlayerControllerServerRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealPlayerStateSingleClientRepData>(
			std::bind(&UEntityPipeline::AddUnrealPlayerStateSingleClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealPlayerStateMultiClientRepData>(
			std::bind(&UEntityPipeline::AddUnrealPlayerStateMultiClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealPlayerStateMigratableData>(
			std::bind(&UEntityPipeline::AddUnrealPlayerStateMigratableDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealPlayerStateClientRPCs>(
			std::bind(&UEntityPipeline::AddUnrealPlayerStateClientRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealPlayerStateServerRPCs>(
			std::bind(&UEntityPipeline::AddUnrealPlayerStateServerRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealWheeledVehicleSingleClientRepData>(
			std::bind(&UEntityPipeline::AddUnrealWheeledVehicleSingleClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealWheeledVehicleMultiClientRepData>(
			std::bind(&UEntityPipeline::AddUnrealWheeledVehicleMultiClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealWheeledVehicleMigratableData>(
			std::bind(&UEntityPipeline::AddUnrealWheeledVehicleMigratableDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealWheeledVehicleClientRPCs>(
			std::bind(&UEntityPipeline::AddUnrealWheeledVehicleClientRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::unreal::UnrealWheeledVehicleServerRPCs>(
			std::bind(&UEntityPipeline::AddUnrealWheeledVehicleServerRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::EntityAcl>(
			std::bind(&UEntityPipeline::AddEntityAclComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::Metadata>(
			std::bind(&UEntityPipeline::AddMetadataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::Position>(
			std::bind(&UEntityPipeline::AddPositionComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnAddComponent<improbable::Persistence>(
			std::bind(&UEntityPipeline::AddPersistenceComponentOp, this, std::placeholders::_1)));

		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealLevel>(
			std::bind(&UEntityPipeline::RemoveUnrealLevelComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealLevelPlaceholder>(
			std::bind(&UEntityPipeline::RemoveUnrealLevelPlaceholderComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::PlayerControlClient>(
			std::bind(&UEntityPipeline::RemovePlayerControlClientComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::PlayerSpawner>(
			std::bind(&UEntityPipeline::RemovePlayerSpawnerComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealMetadata>(
			std::bind(&UEntityPipeline::RemoveUnrealMetadataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealCharacterSingleClientRepData>(
			std::bind(&UEntityPipeline::RemoveUnrealCharacterSingleClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealCharacterMultiClientRepData>(
			std::bind(&UEntityPipeline::RemoveUnrealCharacterMultiClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealCharacterMigratableData>(
			std::bind(&UEntityPipeline::RemoveUnrealCharacterMigratableDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealCharacterClientRPCs>(
			std::bind(&UEntityPipeline::RemoveUnrealCharacterClientRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealCharacterServerRPCs>(
			std::bind(&UEntityPipeline::RemoveUnrealCharacterServerRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealPlayerControllerSingleClientRepData>(
			std::bind(&UEntityPipeline::RemoveUnrealPlayerControllerSingleClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealPlayerControllerMultiClientRepData>(
			std::bind(&UEntityPipeline::RemoveUnrealPlayerControllerMultiClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealPlayerControllerMigratableData>(
			std::bind(&UEntityPipeline::RemoveUnrealPlayerControllerMigratableDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealPlayerControllerClientRPCs>(
			std::bind(&UEntityPipeline::RemoveUnrealPlayerControllerClientRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealPlayerControllerServerRPCs>(
			std::bind(&UEntityPipeline::RemoveUnrealPlayerControllerServerRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealPlayerStateSingleClientRepData>(
			std::bind(&UEntityPipeline::RemoveUnrealPlayerStateSingleClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealPlayerStateMultiClientRepData>(
			std::bind(&UEntityPipeline::RemoveUnrealPlayerStateMultiClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealPlayerStateMigratableData>(
			std::bind(&UEntityPipeline::RemoveUnrealPlayerStateMigratableDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealPlayerStateClientRPCs>(
			std::bind(&UEntityPipeline::RemoveUnrealPlayerStateClientRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealPlayerStateServerRPCs>(
			std::bind(&UEntityPipeline::RemoveUnrealPlayerStateServerRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealWheeledVehicleSingleClientRepData>(
			std::bind(&UEntityPipeline::RemoveUnrealWheeledVehicleSingleClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealWheeledVehicleMultiClientRepData>(
			std::bind(&UEntityPipeline::RemoveUnrealWheeledVehicleMultiClientRepDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealWheeledVehicleMigratableData>(
			std::bind(&UEntityPipeline::RemoveUnrealWheeledVehicleMigratableDataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealWheeledVehicleClientRPCs>(
			std::bind(&UEntityPipeline::RemoveUnrealWheeledVehicleClientRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::unreal::UnrealWheeledVehicleServerRPCs>(
			std::bind(&UEntityPipeline::RemoveUnrealWheeledVehicleServerRPCsComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::EntityAcl>(
			std::bind(&UEntityPipeline::RemoveEntityAclComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::Metadata>(
			std::bind(&UEntityPipeline::RemoveMetadataComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::Position>(
			std::bind(&UEntityPipeline::RemovePositionComponentOp, this, std::placeholders::_1)));
		Callbacks.Add(LockedView->OnRemoveComponent<improbable::Persistence>(
			std::bind(&UEntityPipeline::RemovePersistenceComponentOp, this, std::placeholders::_1)));

	}
	bInitialised = true;
}

void UEntityPipeline::DeregisterAllCallbacks()
{
	// if (CallbackDispatcher != nullptr)
	// {
	// 	CallbackDispatcher->Reset();
	// 	CallbackDispatcher = nullptr;
	// }
	Callbacks.Reset();
	bInitialised = false;
}

void UEntityPipeline::AddBlock(UEntityPipelineBlock* NewBlock)
{
    checkf(!bInitialised, TEXT("Cannot add blocks after the pipeline has started"));

    if(FirstBlock == nullptr)
	{
		FirstBlock = NewBlock;
	}
	if(LastBlock != nullptr)
	{
		LastBlock->NextBlock = NewBlock;
	}
	LastBlock = NewBlock;
}

void UEntityPipeline::ProcessOps(const TWeakPtr<SpatialOSView>& InView, const TWeakPtr<SpatialOSConnection>& InConnection, UWorld* World)
{
	auto Block = FirstBlock;

	while (Block != nullptr)
	{
		Block->ProcessOps(InView, InConnection, World);
		Block = Block->NextBlock;
	}
}

void UEntityPipeline::AddUnrealLevelComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealLevel>& Op) {
	UUnrealLevelAddComponentOp* NewOp = NewObject<UUnrealLevelAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealLevel::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealLevelComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealLevel::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealLevelPlaceholderComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealLevelPlaceholder>& Op) {
	UUnrealLevelPlaceholderAddComponentOp* NewOp = NewObject<UUnrealLevelPlaceholderAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealLevelPlaceholder::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealLevelPlaceholderComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealLevelPlaceholder::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddPlayerControlClientComponentOp(const worker::AddComponentOp<improbable::unreal::PlayerControlClient>& Op) {
	UPlayerControlClientAddComponentOp* NewOp = NewObject<UPlayerControlClientAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::PlayerControlClient::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemovePlayerControlClientComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::PlayerControlClient::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddPlayerSpawnerComponentOp(const worker::AddComponentOp<improbable::unreal::PlayerSpawner>& Op) {
	UPlayerSpawnerAddComponentOp* NewOp = NewObject<UPlayerSpawnerAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::PlayerSpawner::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemovePlayerSpawnerComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::PlayerSpawner::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealMetadataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealMetadata>& Op) {
	UUnrealMetadataAddComponentOp* NewOp = NewObject<UUnrealMetadataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealMetadata::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealMetadataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealMetadata::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealCharacterSingleClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealCharacterSingleClientRepData>& Op) {
	UUnrealCharacterSingleClientRepDataAddComponentOp* NewOp = NewObject<UUnrealCharacterSingleClientRepDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealCharacterSingleClientRepData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealCharacterSingleClientRepDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealCharacterSingleClientRepData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealCharacterMultiClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealCharacterMultiClientRepData>& Op) {
	UUnrealCharacterMultiClientRepDataAddComponentOp* NewOp = NewObject<UUnrealCharacterMultiClientRepDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealCharacterMultiClientRepData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealCharacterMultiClientRepDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealCharacterMultiClientRepData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealCharacterMigratableDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealCharacterMigratableData>& Op) {
	UUnrealCharacterMigratableDataAddComponentOp* NewOp = NewObject<UUnrealCharacterMigratableDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealCharacterMigratableData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealCharacterMigratableDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealCharacterMigratableData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealCharacterClientRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealCharacterClientRPCs>& Op) {
	UUnrealCharacterClientRPCsAddComponentOp* NewOp = NewObject<UUnrealCharacterClientRPCsAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealCharacterClientRPCs::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealCharacterClientRPCsComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealCharacterClientRPCs::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealCharacterServerRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealCharacterServerRPCs>& Op) {
	UUnrealCharacterServerRPCsAddComponentOp* NewOp = NewObject<UUnrealCharacterServerRPCsAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealCharacterServerRPCs::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealCharacterServerRPCsComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealCharacterServerRPCs::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealPlayerControllerSingleClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerSingleClientRepData>& Op) {
	UUnrealPlayerControllerSingleClientRepDataAddComponentOp* NewOp = NewObject<UUnrealPlayerControllerSingleClientRepDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealPlayerControllerSingleClientRepData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealPlayerControllerSingleClientRepDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerControllerSingleClientRepData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealPlayerControllerMultiClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerMultiClientRepData>& Op) {
	UUnrealPlayerControllerMultiClientRepDataAddComponentOp* NewOp = NewObject<UUnrealPlayerControllerMultiClientRepDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealPlayerControllerMultiClientRepData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealPlayerControllerMultiClientRepDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerControllerMultiClientRepData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealPlayerControllerMigratableDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerMigratableData>& Op) {
	UUnrealPlayerControllerMigratableDataAddComponentOp* NewOp = NewObject<UUnrealPlayerControllerMigratableDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealPlayerControllerMigratableData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealPlayerControllerMigratableDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerControllerMigratableData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealPlayerControllerClientRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerClientRPCs>& Op) {
	UUnrealPlayerControllerClientRPCsAddComponentOp* NewOp = NewObject<UUnrealPlayerControllerClientRPCsAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealPlayerControllerClientRPCs::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealPlayerControllerClientRPCsComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerControllerClientRPCs::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealPlayerControllerServerRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerServerRPCs>& Op) {
	UUnrealPlayerControllerServerRPCsAddComponentOp* NewOp = NewObject<UUnrealPlayerControllerServerRPCsAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealPlayerControllerServerRPCs::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealPlayerControllerServerRPCsComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerControllerServerRPCs::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealPlayerStateSingleClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerStateSingleClientRepData>& Op) {
	UUnrealPlayerStateSingleClientRepDataAddComponentOp* NewOp = NewObject<UUnrealPlayerStateSingleClientRepDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealPlayerStateSingleClientRepData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealPlayerStateSingleClientRepDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerStateSingleClientRepData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealPlayerStateMultiClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerStateMultiClientRepData>& Op) {
	UUnrealPlayerStateMultiClientRepDataAddComponentOp* NewOp = NewObject<UUnrealPlayerStateMultiClientRepDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealPlayerStateMultiClientRepData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealPlayerStateMultiClientRepDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerStateMultiClientRepData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealPlayerStateMigratableDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerStateMigratableData>& Op) {
	UUnrealPlayerStateMigratableDataAddComponentOp* NewOp = NewObject<UUnrealPlayerStateMigratableDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealPlayerStateMigratableData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealPlayerStateMigratableDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerStateMigratableData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealPlayerStateClientRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerStateClientRPCs>& Op) {
	UUnrealPlayerStateClientRPCsAddComponentOp* NewOp = NewObject<UUnrealPlayerStateClientRPCsAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealPlayerStateClientRPCs::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealPlayerStateClientRPCsComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerStateClientRPCs::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealPlayerStateServerRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealPlayerStateServerRPCs>& Op) {
	UUnrealPlayerStateServerRPCsAddComponentOp* NewOp = NewObject<UUnrealPlayerStateServerRPCsAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealPlayerStateServerRPCs::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealPlayerStateServerRPCsComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerStateServerRPCs::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealWheeledVehicleSingleClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealWheeledVehicleSingleClientRepData>& Op) {
	UUnrealWheeledVehicleSingleClientRepDataAddComponentOp* NewOp = NewObject<UUnrealWheeledVehicleSingleClientRepDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealWheeledVehicleSingleClientRepData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealWheeledVehicleSingleClientRepDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealWheeledVehicleSingleClientRepData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealWheeledVehicleMultiClientRepDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealWheeledVehicleMultiClientRepData>& Op) {
	UUnrealWheeledVehicleMultiClientRepDataAddComponentOp* NewOp = NewObject<UUnrealWheeledVehicleMultiClientRepDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealWheeledVehicleMultiClientRepData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealWheeledVehicleMultiClientRepDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealWheeledVehicleMultiClientRepData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealWheeledVehicleMigratableDataComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealWheeledVehicleMigratableData>& Op) {
	UUnrealWheeledVehicleMigratableDataAddComponentOp* NewOp = NewObject<UUnrealWheeledVehicleMigratableDataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealWheeledVehicleMigratableData::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealWheeledVehicleMigratableDataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealWheeledVehicleMigratableData::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealWheeledVehicleClientRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealWheeledVehicleClientRPCs>& Op) {
	UUnrealWheeledVehicleClientRPCsAddComponentOp* NewOp = NewObject<UUnrealWheeledVehicleClientRPCsAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealWheeledVehicleClientRPCs::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealWheeledVehicleClientRPCsComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealWheeledVehicleClientRPCs::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddUnrealWheeledVehicleServerRPCsComponentOp(const worker::AddComponentOp<improbable::unreal::UnrealWheeledVehicleServerRPCs>& Op) {
	UUnrealWheeledVehicleServerRPCsAddComponentOp* NewOp = NewObject<UUnrealWheeledVehicleServerRPCsAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::unreal::UnrealWheeledVehicleServerRPCs::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveUnrealWheeledVehicleServerRPCsComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::unreal::UnrealWheeledVehicleServerRPCs::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddEntityAclComponentOp(const worker::AddComponentOp<improbable::EntityAcl>& Op) {
	UEntityAclAddComponentOp* NewOp = NewObject<UEntityAclAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::EntityAcl::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveEntityAclComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::EntityAcl::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddMetadataComponentOp(const worker::AddComponentOp<improbable::Metadata>& Op) {
	UMetadataAddComponentOp* NewOp = NewObject<UMetadataAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::Metadata::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemoveMetadataComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::Metadata::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddPositionComponentOp(const worker::AddComponentOp<improbable::Position>& Op) {
	UPositionAddComponentOp* NewOp = NewObject<UPositionAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::Position::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemovePositionComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::Position::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
void UEntityPipeline::AddPersistenceComponentOp(const worker::AddComponentOp<improbable::Persistence>& Op) {
	UPersistenceAddComponentOp* NewOp = NewObject<UPersistenceAddComponentOp>();
	if (NewOp)
	{
		NewOp->Data = Op.Data;
		NewOp->ComponentId = improbable::Persistence::ComponentId;
		NewOp->EntityId = Op.EntityId;

		FirstBlock->AddComponent(NewOp);
	}
}

void UEntityPipeline::RemovePersistenceComponentOp(const worker::RemoveComponentOp& Op) {
	const worker::ComponentId ComponentId = improbable::Persistence::ComponentId;
	FirstBlock->RemoveComponent(ComponentId, Op);
}
