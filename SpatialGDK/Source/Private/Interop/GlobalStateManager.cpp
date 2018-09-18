// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GlobalStateManager.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialSender.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/EntityRegistry.h"

using namespace improbable;

void UGlobalStateManager::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	View = InNetDriver->View;
	Sender = InNetDriver->Sender;
}

void UGlobalStateManager::ApplyData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	SingletonNameToEntityId = Schema_GetStringToEntityMap(ComponentObject, 1);
	StablyNamedPathToEntityId = Schema_GetStringToEntityMap(ComponentObject, 2);
}

void UGlobalStateManager::ApplyUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (Schema_GetObjectCount(ComponentObject, 1) == 1)
	{
		SingletonNameToEntityId = Schema_GetStringToEntityMap(ComponentObject, 1);
	}

	if (Schema_GetObjectCount(ComponentObject, 2) == 1)
	{
		StablyNamedPathToEntityId = Schema_GetStringToEntityMap(ComponentObject, 2);
	}
}

void UGlobalStateManager::LinkExistingSingletonActors()
{
	if (!NetDriver->IsServer())
	{
		return;
	}

	for (const auto& Pair : SingletonNameToEntityId)
	{
		Worker_EntityId SingletonEntityId = Pair.Value;

		// Singleton Entity hasn't been created yet
		if (SingletonEntityId == SpatialConstants::INVALID_ENTITY_ID)
		{
			continue;
		}

		AActor* SingletonActor = nullptr;
		USpatialActorChannel* Channel = nullptr;
		GetSingletonActorAndChannel(Pair.Key, SingletonActor, Channel);

		// Singleton wasn't found or channel is already set up
		if (Channel == nullptr || Channel->Actor != nullptr)
		{
			continue;
		}

		// Add to entity registry
		// This indirectly causes SetChannelActor to not create a new entity for this actor
		NetDriver->GetEntityRegistry()->AddToRegistry(SingletonEntityId, SingletonActor);

		Channel->SetChannelActor(SingletonActor);


		UnrealMetadata* UnrealMetadata = View->GetUnrealMetadata(SingletonEntityId);
		if (UnrealMetadata == nullptr)
		{
			// Don't have entity checked out
			continue;
		}

		// Since the entity already exists, we have to handle setting up the PackageMap properly for this Actor
		NetDriver->PackageMap->ResolveEntityActor(SingletonActor, SingletonEntityId, UnrealMetadata->SubobjectNameToOffset);
		UE_LOG(LogTemp, Log, TEXT("Linked Singleton Actor %s with id %d"), *SingletonActor->GetClass()->GetName(), SingletonEntityId);
	}
}

void UGlobalStateManager::ExecuteInitialSingletonActorReplication()
{
	for (const auto& pair : SingletonNameToEntityId)
	{
		Worker_EntityId SingletonEntityId = pair.Value;

		// Entity has already been created on another server
		if (SingletonEntityId != SpatialConstants::INVALID_ENTITY_ID)
		{
			continue;
		}

		AActor* SingletonActor = nullptr;
		USpatialActorChannel* Channel = nullptr;
		GetSingletonActorAndChannel(pair.Key, SingletonActor, Channel);

		// Class couldn't be found
		if (Channel == nullptr)
		{
			continue;
		}

		// Set entity id of channel from the GlobalStateManager.
		// If the id was 0, SetChannelActor will create the entity.
		// If the id is not 0, it will start replicating to that entity.
		Channel->SetChannelActor(SingletonActor);

		UE_LOG(LogTemp, Log, TEXT("Started replication of Singleton Actor %s"), *SingletonActor->GetClass()->GetName());
	}
}

void UGlobalStateManager::UpdateSingletonEntityId(const FString& ClassName, const Worker_EntityId SingletonEntityId)
{
	SingletonNameToEntityId[ClassName] = SingletonEntityId;

	Worker_ComponentUpdate Update = {};
	Update.component_id = SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID);
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	Schema_AddStringToEntityMap(UpdateObject, 1, SingletonNameToEntityId);

	NetDriver->Connection->SendComponentUpdate(SpatialConstants::GLOBAL_STATE_MANAGER, &Update);
}

void UGlobalStateManager::GetSingletonActorAndChannel(FString ClassName, AActor*& OutActor, USpatialActorChannel*& OutChannel)
{
	OutActor = nullptr;
	OutChannel = nullptr;

	UClass* SingletonActorClass = LoadObject<UClass>(nullptr, *ClassName);

	if (SingletonActorClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find Singleton Actor Class."));
		return;
	}

	if (TPair<AActor*, USpatialActorChannel*>* Pair = NetDriver->SingletonActorChannels.Find(SingletonActorClass))
	{
		OutActor = Pair->Key;
		OutChannel = Pair->Value;
		return;
	}

	// Class doesn't exist in our map, have to find actor and create channel
	// Get Singleton Actor in world
	TArray<AActor*> SingletonActorList;
	UGameplayStatics::GetAllActorsOfClass(NetDriver->GetWorld(), SingletonActorClass, SingletonActorList);

	if (SingletonActorList.Num() > 1)
	{
		UE_LOG(LogTemp, Error, TEXT("More than one Singleton Actor exists of type %s"), *ClassName);
		return;
	}

	OutActor = SingletonActorList[0];

	USpatialNetConnection* Connection = Cast<USpatialNetConnection>(NetDriver->ClientConnections[0]);

	OutChannel = (USpatialActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1);
	NetDriver->SingletonActorChannels.Add(SingletonActorClass, TPair<AActor*, USpatialActorChannel*>(OutActor, OutChannel));
}

bool UGlobalStateManager::IsSingletonEntity(Worker_EntityId EntityId)
{
	for (const auto& Pair : SingletonNameToEntityId)
	{
		if (Pair.Value == EntityId)
		{
			return true;
		}
	}
	return false;
}
