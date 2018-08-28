//// Fill out your copyright notice in the Description page of Project Settings.
//
//#include "SpatialActorSpawner.h"
//
//#include "EngineMinimal.h"
//#include "EntityRegistry.h"
//#include "GameFramework/PlayerController.h"
//#include "SpatialActorChannel.h"
//#include "SpatialConstants.h"
//#include "SpatialNetConnection.h"
//#include "SpatialNetDriver.h"
//#include "SpatialPackageMapClient.h"
//#include "SpatialGDK/Generated/SpatialComponents.h"
//#include "improbable/view.h"
//#include "improbable/worker.h"
//#include <improbable/unreal/gdk/unreal_metadata.h>
//#include "GameFramework/CharacterMovementComponent.h"
//
//void USpatialActorSpawner::Init(USpatialNetDriver* NetDriver, UEntityRegistry* EntityRegistry)
//{
//	this->NetDriver = NetDriver;
//	this->Connection = NetDriver->Connection;
//	this->View = NetDriver->View;
//	this->EntityRegistry = EntityRegistry;
//	World = NetDriver->GetWorld();
//}
//
//template <typename T>
//void USpatialActorSpawner::Accept()
//{
//	View->OnAddComponent<T>([this](const worker::AddComponentOp<T>& op) {
//		if(this->inCriticalSection)
//		{
//			TSharedPtr<worker::detail::ComponentStorage<T>> Component = MakeShared<worker::detail::ComponentStorage<T>>(op.Data);
//			FAddComponent AddComponentWrapper{T::ComponentId, Component};
//
//			if(PendingAddComponentOps.Find(op.EntityId) == nullptr)
//			{
//				PendingAddComponentOps.Add(op.EntityId, TArray<FAddComponent>());
//			}
//
//			PendingAddComponentOps[op.EntityId].Add(AddComponentWrapper);
//		}
//	});
//}
//
//void USpatialActorSpawner::RegisterCallbacks()
//{
//	View->OnAddEntity([this](const worker::AddEntityOp& op) {
//		AddEntity(op);
//	});
//
//	View->OnRemoveEntity([this](const worker::RemoveEntityOp& op) {
//		RemoveEntity(op);
//	});
//
//	View->OnCriticalSection([this](const worker::CriticalSectionOp op) {
//		HitCriticalSection(op);
//	});
//
//	ForEachComponent(improbable::unreal::Components{}, *this);
//}
//
//void USpatialActorSpawner::AddEntity(const worker::AddEntityOp& op)
//{
//	if(inCriticalSection)
//	{
//		PendingAddEntityOps.Add(op);
//		return;
//	}
//
//	CreateActor(op.EntityId);
//}
//
//void USpatialActorSpawner::RemoveEntity(const worker::RemoveEntityOp& op)
//{
//	if(inCriticalSection)
//	{
//		PendingRemoveEntityOps.Add(op);
//		return;
//	}
//}
//
//void USpatialActorSpawner::HitCriticalSection(const worker::CriticalSectionOp& op)
//{
//	if(!inCriticalSection)
//	{
//		inCriticalSection = true;
//	}
//	else
//	{
//		inCriticalSection = false;
//
//		for(const worker::AddEntityOp& AddOp : PendingAddEntityOps)
//		{
//			AddEntity(AddOp);
//		}
//		PendingAddEntityOps.Empty();
//
//		for(const worker::RemoveEntityOp& RemoveOp : PendingRemoveEntityOps)
//		{
//			RemoveEntity(RemoveOp);
//		}
//		PendingRemoveEntityOps.Empty();
//
//		NetDriver->GetSpatialInterop()->OnLeaveCriticalSection();
//	}
//}
//
//void USpatialActorSpawner::CreateActor(const worker::EntityId& EntityId)
//{
//	improbable::PositionData* PositionComponent = GetComponentDataFromView<improbable::Position>(EntityId);
//	improbable::MetadataData* MetadataComponent = GetComponentDataFromView<improbable::Metadata>(EntityId);
//
//	if (!PositionComponent || !MetadataComponent)
//	{
//		return;
//	}
//
//	AActor* EntityActor = EntityRegistry->GetActorFromEntityId(EntityId);
//	//UE_LOG(LogSpatialGDKActorSpawner, Log, TEXT("Checked out entity with entity ID %lld"), EntityId.ToSpatialEntityId());
//
//	// There are 2 main options when we get here with regards to how this entity was created:
//	// 1) A SpawnActor() call (through interop) on this worker, which means it already has an actor associated with it.
//	//	  This usually happens on the Unreal server only (as servers are the only workers which can spawn actors).
//	// 2) A SpawnActor() call that was initiated from a different worker, which means we need to find and spawn the corresponding "native" actor that corresponds to it.
//	//	  This can happen on either the client (for all actors) or server (for actors which were spawned by a different server worker, or are transitioning).
//
//	if (EntityActor)
//	{
//		UClass* ActorClass = GetNativeEntityClass(MetadataComponent);
//		USpatialInterop* Interop = NetDriver->GetSpatialInterop();
//		check(Interop);
//
//		// Option 1
//		//UE_LOG(LogSpatialGDKActorSpawner, Log, TEXT("Entity for core actor %s has been checked out on the worker which spawned it."), *EntityActor->GetName());
//
//		improbable::unreal::UnrealMetadataData* UnrealMetadataComponent = GetComponentDataFromView<improbable::unreal::UnrealMetadata>(EntityId);
//		check(UnrealMetadataComponent);
//
//		USpatialPackageMapClient* PackageMap = Cast<USpatialPackageMapClient>(NetDriver->GetSpatialOSNetConnection()->PackageMap);
//		check(PackageMap);
//
//		FNetworkGUID NetGUID = PackageMap->ResolveEntityActor(EntityActor, EntityId, UnrealMetadataComponent->subobject_name_to_offset());
//		//UE_LOG(LogSpatialGDKActorSpawner, Log, TEXT("Received create entity response op for %d"), EntityId.ToSpatialEntityId());
//
//		// actor channel/entity mapping should be registered by this point
//		check(NetDriver->GetSpatialInterop()->GetActorChannelByEntityId(EntityId));
//	}
//	else
//	{
//		if (UClass* ActorClass = GetNativeEntityClass(MetadataComponent)) 
//		{
//			USpatialInterop* Interop = NetDriver->GetSpatialInterop();
//			check(Interop);
//
//			// Initial Singleton Actor replication is handled with USpatialInterop::LinkExistingSingletonActors
//			if (NetDriver->IsServer() && Interop->IsSingletonClass(ActorClass))
//			{
//				return;
//			}
//
//			UNetConnection* Connection = nullptr;
//			improbable::unreal::UnrealMetadataData* UnrealMetadataComponent = GetComponentDataFromView<improbable::unreal::UnrealMetadata>(EntityId);
//			check(UnrealMetadataComponent);
//			bool bDoingDeferredSpawn = false;
//
//			// If we're checking out a player controller, spawn it via "USpatialNetDriver::AcceptNewPlayer"
//			if (NetDriver->IsServer() && ActorClass->IsChildOf(APlayerController::StaticClass()))
//			{
//				checkf(!UnrealMetadataComponent->owner_worker_id().empty(), TEXT("A player controller entity must have an owner worker ID."));
//				FString URLString = FURL().ToString();
//				FString OwnerWorkerId = UTF8_TO_TCHAR(UnrealMetadataComponent->owner_worker_id().data()->c_str());
//				URLString += TEXT("?workerId=") + OwnerWorkerId;
//				Connection = NetDriver->AcceptNewPlayer(FURL(nullptr, *URLString, TRAVEL_Absolute), true);
//				check(Connection);
//				EntityActor = Connection->PlayerController;
//			}
//			else
//			{
//				// Either spawn the actor or get it from the level if it has a persistent name.
//				if (UnrealMetadataComponent->static_path().empty())
//				{
//					//UE_LOG(LogSpatialGDKInteropPipelineBlock, Log, TEXT("Spawning a native dynamic %s whilst checking out an entity."), *ActorClass->GetFullName());
//					EntityActor = SpawnActor(PositionComponent, ActorClass, true);
//					bDoingDeferredSpawn = true;
//				}
//				else
//				{
//					FString FullPath = UTF8_TO_TCHAR(UnrealMetadataComponent->static_path().data()->c_str());
//					//UE_LOG(LogSpatialGDKInteropPipelineBlock, Log, TEXT("Searching for a native static actor %s of class %s in the persistent level whilst checking out an entity."), *FullPath, *ActorClass->GetName());
//					EntityActor = FindObject<AActor>(GetWorld(), *FullPath);
//				}
//				check(EntityActor);
//
//				// Get the net connection for this actor.
//				if (NetDriver->IsServer())
//				{
//					// TODO(David): Currently, we just create an actor channel on the "catch-all" connection, then create a new actor channel once we check out the player controller
//					// and create a new connection. This is fine due to lazy actor channel creation in USpatialNetDriver::ServerReplicateActors. However, the "right" thing to do
//					// would be to make sure to create anything which depends on the PlayerController _after_ the PlayerController's connection is set up so we can use the right
//					// one here.
//					Connection = NetDriver->GetSpatialOSNetConnection();
//				}
//				else
//				{
//					Connection = NetDriver->GetSpatialOSNetConnection();
//				}
//			}
//
//			// Add to entity registry. 
//			EntityRegistry->AddToRegistry(EntityId, EntityActor);
//
//			// Set up actor channel.
//			auto PackageMap = Cast<USpatialPackageMapClient>(Connection->PackageMap);
//			auto Channel = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, NetDriver->IsServer()));
//			check(Channel);
//
//			if (bDoingDeferredSpawn)
//			{
//				auto InitialLocation = SpatialConstants::SpatialOSCoordinatesToLocation(PositionComponent->coords());
//				FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(InitialLocation, GetWorld()->OriginLocation);
//				EntityActor->FinishSpawning(FTransform(FRotator::ZeroRotator, SpawnLocation));
//			}
//
//			PackageMap->ResolveEntityActor(EntityActor, EntityId, UnrealMetadataComponent->subobject_name_to_offset());
//			Channel->SetChannelActor(EntityActor);
//
//			// Apply initial replicated properties.
//			// This was moved to after FinishingSpawning because components existing only in blueprints aren't added until spawning is complete
//			// Potentially we could split out the initial actor state and the initial component state
//			TArray<FAddComponent>& PendingAddComponents = PendingAddComponentOps[EntityId];
//			for(const FAddComponent& AddComponent : PendingAddComponents )
//			{
//				NetDriver->GetSpatialInterop()->ReceiveAddComponent(Channel, AddComponent);
//			}
//			PendingAddComponentOps.Remove(EntityId);
//
//			// Update interest on the entity's components after receiving initial component data (so Role and RemoteRole are properly set).
//			NetDriver->GetSpatialInterop()->SendComponentInterests(Channel, EntityId);
//
//			// This is a bit of a hack unfortunately, among the core classes only PlayerController implements this function and it requires
//			// a player index. For now we don't support split screen, so the number is always 0.
//			if (NetDriver->ServerConnection)
//			{
//				if (EntityActor->IsA(APlayerController::StaticClass()))
//				{
//					uint8 PlayerIndex = 0;
//					// FInBunch takes size in bits not bytes
//					FInBunch Bunch(NetDriver->ServerConnection, &PlayerIndex, sizeof(PlayerIndex) * 8);
//					EntityActor->OnActorChannelOpen(Bunch, NetDriver->ServerConnection);
//				}
//				else
//				{
//					FInBunch Bunch(NetDriver->ServerConnection);
//					EntityActor->OnActorChannelOpen(Bunch, NetDriver->ServerConnection);
//				}
//
//				// Call PostNetInit on client only.
//				EntityActor->PostNetInit();
//			}
//		}
//	}
//	return;
//}
//
//AActor* USpatialActorSpawner::SpawnActor(improbable::PositionData* PositionComponent, UClass* ActorClass, bool bDeferred)
//{
//	FVector InitialLocation = SpatialConstants::SpatialOSCoordinatesToLocation(PositionComponent->coords());
//	AActor* NewActor = nullptr;
//	if (ActorClass)
//	{
//		//bRemoteOwned needs to be public in source code. This might be a controversial change.
//		FActorSpawnParameters SpawnInfo;
//		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//		SpawnInfo.bRemoteOwned = !NetDriver->IsServer();
//		SpawnInfo.bNoFail = true;
//		// We defer the construction in the GDK pipeline to allow initialization of replicated properties first.
//		SpawnInfo.bDeferConstruction = bDeferred;
//
//		FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(InitialLocation, World->OriginLocation);
//
//		NewActor = World->SpawnActorAbsolute(ActorClass, FTransform(FRotator::ZeroRotator, SpawnLocation), SpawnInfo);
//		check(NewActor);
//	}
//
//	return NewActor;
//}
//
//void USpatialActorSpawner::CleanupDeletedActor(const worker::EntityId EntityId)
//{
//	EntityRegistry->RemoveFromRegistry(EntityId);
//	NetDriver->GetSpatialInterop()->RemoveActorChannel(EntityId);
//	auto* PackageMap = Cast<USpatialPackageMapClient>(NetDriver->GetSpatialOSNetConnection()->PackageMap);
//	PackageMap->RemoveEntityActor(EntityId);
//}
//
//UClass* USpatialActorSpawner::GetNativeEntityClass(improbable::MetadataData* MetadataComponent)
//{
//	FString Metadata = UTF8_TO_TCHAR(MetadataComponent->entity_type().c_str());
//	return FindObject<UClass>(ANY_PACKAGE, *Metadata);
//}
