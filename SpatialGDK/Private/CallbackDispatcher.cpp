// // Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// // ===========
// // DO NOT EDIT - this file is automatically regenerated.
// // =========== 


// #include "CallbackDispatcher.h"
// #include "ComponentUpdateOpWrapperBase.h"
// #include "UnrealLevelComponentUpdate.h"
// #include "UnrealLevelPlaceholderComponentUpdate.h"
// #include "PlayerControlClientComponentUpdate.h"
// #include "PlayerSpawnerComponentUpdate.h"
// #include "UnrealMetadataComponentUpdate.h"
// #include "UnrealCharacterSingleClientRepDataComponentUpdate.h"
// #include "UnrealCharacterMultiClientRepDataComponentUpdate.h"
// #include "UnrealCharacterMigratableDataComponentUpdate.h"
// #include "UnrealCharacterClientRPCsComponentUpdate.h"
// #include "UnrealCharacterServerRPCsComponentUpdate.h"
// #include "UnrealPlayerControllerSingleClientRepDataComponentUpdate.h"
// #include "UnrealPlayerControllerMultiClientRepDataComponentUpdate.h"
// #include "UnrealPlayerControllerMigratableDataComponentUpdate.h"
// #include "UnrealPlayerControllerClientRPCsComponentUpdate.h"
// #include "UnrealPlayerControllerServerRPCsComponentUpdate.h"
// #include "UnrealPlayerStateSingleClientRepDataComponentUpdate.h"
// #include "UnrealPlayerStateMultiClientRepDataComponentUpdate.h"
// #include "UnrealPlayerStateMigratableDataComponentUpdate.h"
// #include "UnrealPlayerStateClientRPCsComponentUpdate.h"
// #include "UnrealPlayerStateServerRPCsComponentUpdate.h"
// #include "UnrealWheeledVehicleSingleClientRepDataComponentUpdate.h"
// #include "UnrealWheeledVehicleMultiClientRepDataComponentUpdate.h"
// #include "UnrealWheeledVehicleMigratableDataComponentUpdate.h"
// #include "UnrealWheeledVehicleClientRPCsComponentUpdate.h"
// #include "UnrealWheeledVehicleServerRPCsComponentUpdate.h"
// #include "EntityAclComponentUpdate.h"
// #include "MetadataComponentUpdate.h"
// #include "PositionComponentUpdate.h"
// #include "PersistenceComponentUpdate.h"
// #include "ScopedViewCallbacks.h"
// #include "SpatialOSViewTypes.h"


// DECLARE_LOG_CATEGORY_EXTERN(LogCallbackDispatcher, Log, All);
// DEFINE_LOG_CATEGORY(LogCallbackDispatcher);

// UCallbackDispatcher::UCallbackDispatcher() : bInitialised(false)
// {
// }

// void UCallbackDispatcher::Init(const TWeakPtr<SpatialOSView>& InView)
// {
// 	checkf(!bInitialised, TEXT("Attempting to call Init more than once!"));
// 	Callbacks.Init(InView);

// 	auto LockedView = InView.Pin();
// 	if(LockedView.IsValid())
// 	{
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealLevel>(std::bind(
// 			&UCallbackDispatcher::OnUnrealLevelComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealLevel>(std::bind(
// 			&UCallbackDispatcher::OnUnrealLevelAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealLevelPlaceholder>(std::bind(
// 			&UCallbackDispatcher::OnUnrealLevelPlaceholderComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealLevelPlaceholder>(std::bind(
// 			&UCallbackDispatcher::OnUnrealLevelPlaceholderAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::PlayerControlClient>(std::bind(
// 			&UCallbackDispatcher::OnPlayerControlClientComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::PlayerControlClient>(std::bind(
// 			&UCallbackDispatcher::OnPlayerControlClientAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::PlayerSpawner>(std::bind(
// 			&UCallbackDispatcher::OnPlayerSpawnerComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::PlayerSpawner>(std::bind(
// 			&UCallbackDispatcher::OnPlayerSpawnerAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealMetadata>(std::bind(
// 			&UCallbackDispatcher::OnUnrealMetadataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealMetadata>(std::bind(
// 			&UCallbackDispatcher::OnUnrealMetadataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealCharacterSingleClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealCharacterSingleClientRepDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealCharacterSingleClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealCharacterSingleClientRepDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealCharacterMultiClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealCharacterMultiClientRepDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealCharacterMultiClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealCharacterMultiClientRepDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealCharacterMigratableData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealCharacterMigratableDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealCharacterMigratableData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealCharacterMigratableDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealCharacterClientRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealCharacterClientRPCsComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealCharacterClientRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealCharacterClientRPCsAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealCharacterServerRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealCharacterServerRPCsComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealCharacterServerRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealCharacterServerRPCsAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealPlayerControllerSingleClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerControllerSingleClientRepDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealPlayerControllerSingleClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerControllerSingleClientRepDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealPlayerControllerMultiClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerControllerMultiClientRepDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealPlayerControllerMultiClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerControllerMultiClientRepDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealPlayerControllerMigratableData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerControllerMigratableDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealPlayerControllerMigratableData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerControllerMigratableDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealPlayerControllerClientRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerControllerClientRPCsComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealPlayerControllerClientRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerControllerClientRPCsAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealPlayerControllerServerRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerControllerServerRPCsComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealPlayerControllerServerRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerControllerServerRPCsAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealPlayerStateSingleClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerStateSingleClientRepDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealPlayerStateSingleClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerStateSingleClientRepDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealPlayerStateMultiClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerStateMultiClientRepDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealPlayerStateMultiClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerStateMultiClientRepDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealPlayerStateMigratableData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerStateMigratableDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealPlayerStateMigratableData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerStateMigratableDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealPlayerStateClientRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerStateClientRPCsComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealPlayerStateClientRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerStateClientRPCsAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealPlayerStateServerRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerStateServerRPCsComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealPlayerStateServerRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealPlayerStateServerRPCsAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealWheeledVehicleSingleClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealWheeledVehicleSingleClientRepDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealWheeledVehicleSingleClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealWheeledVehicleSingleClientRepDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealWheeledVehicleMultiClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealWheeledVehicleMultiClientRepDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealWheeledVehicleMultiClientRepData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealWheeledVehicleMultiClientRepDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealWheeledVehicleMigratableData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealWheeledVehicleMigratableDataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealWheeledVehicleMigratableData>(std::bind(
// 			&UCallbackDispatcher::OnUnrealWheeledVehicleMigratableDataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealWheeledVehicleClientRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealWheeledVehicleClientRPCsComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealWheeledVehicleClientRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealWheeledVehicleClientRPCsAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::unreal::UnrealWheeledVehicleServerRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealWheeledVehicleServerRPCsComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::unreal::UnrealWheeledVehicleServerRPCs>(std::bind(
// 			&UCallbackDispatcher::OnUnrealWheeledVehicleServerRPCsAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::EntityAcl>(std::bind(
// 			&UCallbackDispatcher::OnEntityAclComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::EntityAcl>(std::bind(
// 			&UCallbackDispatcher::OnEntityAclAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::Metadata>(std::bind(
// 			&UCallbackDispatcher::OnMetadataComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::Metadata>(std::bind(
// 			&UCallbackDispatcher::OnMetadataAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::Position>(std::bind(
// 			&UCallbackDispatcher::OnPositionComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::Position>(std::bind(
// 			&UCallbackDispatcher::OnPositionAuthorityChangeOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnComponentUpdate<improbable::Persistence>(std::bind(
// 			&UCallbackDispatcher::OnPersistenceComponentUpdateOp, this, std::placeholders::_1)));
// 		Callbacks.Add(LockedView->OnAuthorityChange<improbable::Persistence>(std::bind(
// 			&UCallbackDispatcher::OnPersistenceAuthorityChangeOp, this, std::placeholders::_1)));
// 	}
// 	bInitialised = true;
// }

// void UCallbackDispatcher::Reset()
// {
// 	ComponentUpdateCallbacks.Reset();
// 	AuthorityChangeCallbacks.Reset();
// 	QueuedUpdates.Reset();
// 	Callbacks.Reset();
// 	bInitialised = false;
// }

// void UCallbackDispatcher::AddComponentUpdateCallback(worker::EntityId EntityId, worker::ComponentId ComponentId, TFunction<void(UComponentUpdateOpWrapperBase&)> Callback)
// {
// 	FComponentIdentifier Id{ EntityId, ComponentId };
// 	ComponentUpdateCallbacks.Emplace(Id, Callback);
// 	DispatchQueuedUpdates(Id);
// }

// void UCallbackDispatcher::AddAuthorityChangeCallback(worker::EntityId EntityId, worker::ComponentId ComponentId, TFunction<void(const worker::AuthorityChangeOp&)> Callback)
// {
// 	AuthorityChangeCallbacks.Emplace(FComponentIdentifier{EntityId, ComponentId}, Callback);
// }

// void UCallbackDispatcher::RemoveComponentUpdateCallback(worker::EntityId EntityId, worker::ComponentId ComponentId)
// {
// 	ComponentUpdateCallbacks.Remove(FComponentIdentifier{EntityId, ComponentId});
// }

// void UCallbackDispatcher::RemoveAuthorityChangeCallback(worker::EntityId EntityId, worker::ComponentId ComponentId)
// {
// 	AuthorityChangeCallbacks.Remove(FComponentIdentifier{EntityId, ComponentId});
// }

// void UCallbackDispatcher::HandleUpdate(UComponentUpdateOpWrapperBase* Update)
// {
// 	FComponentIdentifier Id{ Update->EntityId, Update->ComponentId };

// 	auto Callback = ComponentUpdateCallbacks.Find(Id);
// 	if (Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(*Update);
// 	}
// 	else
// 	{
// 		EnqueueUpdate(Id, Update);
// 	}
// }

// void UCallbackDispatcher::EnqueueUpdate(const FComponentIdentifier& Id, UComponentUpdateOpWrapperBase* Update)
// {
// 	FComponentUpdateQueue* Queue = QueuedUpdates.Find(Id);
// 	if (!Queue)
// 	{
// 		Queue = &QueuedUpdates.Emplace(Id, FComponentUpdateQueue());
// 	}
// 	Queue->AddToQueue(Update);
// }

// void UCallbackDispatcher::DispatchQueuedUpdates(const FComponentIdentifier& Id)
// {
// 	FComponentUpdateQueue* PendingUpdateQueue = QueuedUpdates.Find(Id);
// 	if (PendingUpdateQueue)
// 	{
// 		auto Callback = ComponentUpdateCallbacks.Find(Id);
// 		if (Callback != nullptr && (*Callback) != nullptr)
// 		{
// 			for (auto QueuedUpdate : PendingUpdateQueue->GetQueue())
// 			{
// 				(*Callback)(*QueuedUpdate);
// 			}
// 		}
// 		QueuedUpdates.Remove(Id);
// 	}
// }

// void UCallbackDispatcher::OnUnrealLevelComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealLevel>& Op) {
// 	UUnrealLevelComponentUpdate* WrappedUpdate = NewObject<UUnrealLevelComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealLevel::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealLevelAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealLevel::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealLevelPlaceholderComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealLevelPlaceholder>& Op) {
// 	UUnrealLevelPlaceholderComponentUpdate* WrappedUpdate = NewObject<UUnrealLevelPlaceholderComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealLevelPlaceholder::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealLevelPlaceholderAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealLevelPlaceholder::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnPlayerControlClientComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::PlayerControlClient>& Op) {
// 	UPlayerControlClientComponentUpdate* WrappedUpdate = NewObject<UPlayerControlClientComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::PlayerControlClient::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnPlayerControlClientAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::PlayerControlClient::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnPlayerSpawnerComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::PlayerSpawner>& Op) {
// 	UPlayerSpawnerComponentUpdate* WrappedUpdate = NewObject<UPlayerSpawnerComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::PlayerSpawner::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnPlayerSpawnerAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::PlayerSpawner::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealMetadataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealMetadata>& Op) {
// 	UUnrealMetadataComponentUpdate* WrappedUpdate = NewObject<UUnrealMetadataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealMetadata::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealMetadataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealMetadata::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealCharacterSingleClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterSingleClientRepData>& Op) {
// 	UUnrealCharacterSingleClientRepDataComponentUpdate* WrappedUpdate = NewObject<UUnrealCharacterSingleClientRepDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealCharacterSingleClientRepData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealCharacterSingleClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealCharacterSingleClientRepData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealCharacterMultiClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterMultiClientRepData>& Op) {
// 	UUnrealCharacterMultiClientRepDataComponentUpdate* WrappedUpdate = NewObject<UUnrealCharacterMultiClientRepDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealCharacterMultiClientRepData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealCharacterMultiClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealCharacterMultiClientRepData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealCharacterMigratableDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterMigratableData>& Op) {
// 	UUnrealCharacterMigratableDataComponentUpdate* WrappedUpdate = NewObject<UUnrealCharacterMigratableDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealCharacterMigratableData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealCharacterMigratableDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealCharacterMigratableData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealCharacterClientRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterClientRPCs>& Op) {
// 	UUnrealCharacterClientRPCsComponentUpdate* WrappedUpdate = NewObject<UUnrealCharacterClientRPCsComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealCharacterClientRPCs::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealCharacterClientRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealCharacterClientRPCs::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealCharacterServerRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterServerRPCs>& Op) {
// 	UUnrealCharacterServerRPCsComponentUpdate* WrappedUpdate = NewObject<UUnrealCharacterServerRPCsComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealCharacterServerRPCs::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealCharacterServerRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealCharacterServerRPCs::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealPlayerControllerSingleClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerSingleClientRepData>& Op) {
// 	UUnrealPlayerControllerSingleClientRepDataComponentUpdate* WrappedUpdate = NewObject<UUnrealPlayerControllerSingleClientRepDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealPlayerControllerSingleClientRepData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealPlayerControllerSingleClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerControllerSingleClientRepData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealPlayerControllerMultiClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerMultiClientRepData>& Op) {
// 	UUnrealPlayerControllerMultiClientRepDataComponentUpdate* WrappedUpdate = NewObject<UUnrealPlayerControllerMultiClientRepDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealPlayerControllerMultiClientRepData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealPlayerControllerMultiClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerControllerMultiClientRepData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealPlayerControllerMigratableDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerMigratableData>& Op) {
// 	UUnrealPlayerControllerMigratableDataComponentUpdate* WrappedUpdate = NewObject<UUnrealPlayerControllerMigratableDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealPlayerControllerMigratableData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealPlayerControllerMigratableDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerControllerMigratableData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealPlayerControllerClientRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerClientRPCs>& Op) {
// 	UUnrealPlayerControllerClientRPCsComponentUpdate* WrappedUpdate = NewObject<UUnrealPlayerControllerClientRPCsComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealPlayerControllerClientRPCs::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealPlayerControllerClientRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerControllerClientRPCs::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealPlayerControllerServerRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerServerRPCs>& Op) {
// 	UUnrealPlayerControllerServerRPCsComponentUpdate* WrappedUpdate = NewObject<UUnrealPlayerControllerServerRPCsComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealPlayerControllerServerRPCs::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealPlayerControllerServerRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerControllerServerRPCs::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealPlayerStateSingleClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateSingleClientRepData>& Op) {
// 	UUnrealPlayerStateSingleClientRepDataComponentUpdate* WrappedUpdate = NewObject<UUnrealPlayerStateSingleClientRepDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealPlayerStateSingleClientRepData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealPlayerStateSingleClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerStateSingleClientRepData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealPlayerStateMultiClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateMultiClientRepData>& Op) {
// 	UUnrealPlayerStateMultiClientRepDataComponentUpdate* WrappedUpdate = NewObject<UUnrealPlayerStateMultiClientRepDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealPlayerStateMultiClientRepData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealPlayerStateMultiClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerStateMultiClientRepData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealPlayerStateMigratableDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateMigratableData>& Op) {
// 	UUnrealPlayerStateMigratableDataComponentUpdate* WrappedUpdate = NewObject<UUnrealPlayerStateMigratableDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealPlayerStateMigratableData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealPlayerStateMigratableDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerStateMigratableData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealPlayerStateClientRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateClientRPCs>& Op) {
// 	UUnrealPlayerStateClientRPCsComponentUpdate* WrappedUpdate = NewObject<UUnrealPlayerStateClientRPCsComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealPlayerStateClientRPCs::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealPlayerStateClientRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerStateClientRPCs::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealPlayerStateServerRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateServerRPCs>& Op) {
// 	UUnrealPlayerStateServerRPCsComponentUpdate* WrappedUpdate = NewObject<UUnrealPlayerStateServerRPCsComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealPlayerStateServerRPCs::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealPlayerStateServerRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealPlayerStateServerRPCs::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealWheeledVehicleSingleClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealWheeledVehicleSingleClientRepData>& Op) {
// 	UUnrealWheeledVehicleSingleClientRepDataComponentUpdate* WrappedUpdate = NewObject<UUnrealWheeledVehicleSingleClientRepDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealWheeledVehicleSingleClientRepData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealWheeledVehicleSingleClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealWheeledVehicleSingleClientRepData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealWheeledVehicleMultiClientRepDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealWheeledVehicleMultiClientRepData>& Op) {
// 	UUnrealWheeledVehicleMultiClientRepDataComponentUpdate* WrappedUpdate = NewObject<UUnrealWheeledVehicleMultiClientRepDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealWheeledVehicleMultiClientRepData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealWheeledVehicleMultiClientRepDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealWheeledVehicleMultiClientRepData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealWheeledVehicleMigratableDataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealWheeledVehicleMigratableData>& Op) {
// 	UUnrealWheeledVehicleMigratableDataComponentUpdate* WrappedUpdate = NewObject<UUnrealWheeledVehicleMigratableDataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealWheeledVehicleMigratableData::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealWheeledVehicleMigratableDataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealWheeledVehicleMigratableData::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealWheeledVehicleClientRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealWheeledVehicleClientRPCs>& Op) {
// 	UUnrealWheeledVehicleClientRPCsComponentUpdate* WrappedUpdate = NewObject<UUnrealWheeledVehicleClientRPCsComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealWheeledVehicleClientRPCs::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealWheeledVehicleClientRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealWheeledVehicleClientRPCs::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnUnrealWheeledVehicleServerRPCsComponentUpdateOp(const worker::ComponentUpdateOp<improbable::unreal::UnrealWheeledVehicleServerRPCs>& Op) {
// 	UUnrealWheeledVehicleServerRPCsComponentUpdate* WrappedUpdate = NewObject<UUnrealWheeledVehicleServerRPCsComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::unreal::UnrealWheeledVehicleServerRPCs::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnUnrealWheeledVehicleServerRPCsAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::unreal::UnrealWheeledVehicleServerRPCs::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnEntityAclComponentUpdateOp(const worker::ComponentUpdateOp<improbable::EntityAcl>& Op) {
// 	UEntityAclComponentUpdate* WrappedUpdate = NewObject<UEntityAclComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::EntityAcl::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnEntityAclAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::EntityAcl::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnMetadataComponentUpdateOp(const worker::ComponentUpdateOp<improbable::Metadata>& Op) {
// 	UMetadataComponentUpdate* WrappedUpdate = NewObject<UMetadataComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::Metadata::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnMetadataAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::Metadata::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnPositionComponentUpdateOp(const worker::ComponentUpdateOp<improbable::Position>& Op) {
// 	UPositionComponentUpdate* WrappedUpdate = NewObject<UPositionComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::Position::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnPositionAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::Position::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

// void UCallbackDispatcher::OnPersistenceComponentUpdateOp(const worker::ComponentUpdateOp<improbable::Persistence>& Op) {
// 	UPersistenceComponentUpdate* WrappedUpdate = NewObject<UPersistenceComponentUpdate>();
// 	if (WrappedUpdate)
// 	{
// 		WrappedUpdate->InitInternal(Op.Update);
// 		WrappedUpdate->ComponentId = improbable::Persistence::ComponentId;
// 		WrappedUpdate->EntityId = Op.EntityId;

// 		HandleUpdate(WrappedUpdate);
// 	}
// }

// void UCallbackDispatcher::OnPersistenceAuthorityChangeOp(const worker::AuthorityChangeOp& Op) {
// 	const worker::ComponentId ComponentId = improbable::Persistence::ComponentId;

// 	auto Callback = AuthorityChangeCallbacks.Find(FComponentIdentifier{Op.EntityId, ComponentId});
// 	if(Callback != nullptr && (*Callback) != nullptr)
// 	{
// 		(*Callback)(Op);
// 	}

// 	OnAuthorityChangeOpReceived.Broadcast(ComponentId, Op);
// }

