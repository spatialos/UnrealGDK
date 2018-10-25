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

DEFINE_LOG_CATEGORY(LogGlobalStateManager);

using namespace improbable;

void UGlobalStateManager::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Sender = InNetDriver->Sender;
}

void UGlobalStateManager::ApplyData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	SingletonNameToEntityId = GetStringToEntityMapFromSchema(ComponentObject, 1);
}

void UGlobalStateManager::ApplyUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (Schema_GetObjectCount(ComponentObject, 1) > 0)
	{
		SingletonNameToEntityId = GetStringToEntityMapFromSchema(ComponentObject, 1);
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

		SingletonActor->Role = ROLE_SimulatedProxy;
		SingletonActor->RemoteRole = ROLE_Authority;

		// Add to entity registry
		// This indirectly causes SetChannelActor to not create a new entity for this actor
		NetDriver->GetEntityRegistry()->AddToRegistry(SingletonEntityId, SingletonActor);

		Channel->SetChannelActor(SingletonActor);

		// Since the entity already exists, we have to handle setting up the PackageMap properly for this Actor
		FClassInfo* Info = NetDriver->TypebindingManager->FindClassInfoByClass(SingletonActor->GetClass());
		check(Info);

		NetDriver->PackageMap->ResolveEntityActor(SingletonActor, SingletonEntityId, improbable::CreateOffsetMapFromActor(SingletonActor, Info));

		UE_LOG(LogGlobalStateManager, Log, TEXT("Linked Singleton Actor %s with id %d"), *SingletonActor->GetClass()->GetName(), SingletonEntityId);
	}
}

void UGlobalStateManager::ExecuteInitialSingletonActorReplication()
{
	for (const auto& Pair : SingletonNameToEntityId)
	{
		Worker_EntityId SingletonEntityId = Pair.Value;

		// Entity has already been created on another server
		if (SingletonEntityId != SpatialConstants::INVALID_ENTITY_ID)
		{
			continue;
		}

		AActor* SingletonActor = nullptr;
		USpatialActorChannel* Channel = nullptr;
		GetSingletonActorAndChannel(Pair.Key, SingletonActor, Channel);

		// Class couldn't be found
		if (Channel == nullptr)
		{
			continue;
		}

		SingletonActor->Role = ROLE_Authority;
		SingletonActor->RemoteRole = ROLE_SimulatedProxy;

		// Set entity id of channel from the GlobalStateManager.
		// If the id was 0, SetChannelActor will create the entity.
		// If the id is not 0, it will start replicating to that entity.
		Channel->SetChannelActor(SingletonActor);

		UE_LOG(LogGlobalStateManager, Log, TEXT("Started replication of Singleton Actor %s"), *SingletonActor->GetClass()->GetName());
	}
}

void UGlobalStateManager::UpdateSingletonEntityId(const FString& ClassName, const Worker_EntityId SingletonEntityId)
{
	SingletonNameToEntityId[ClassName] = SingletonEntityId;

	if (!NetDriver->StaticComponentView->HasAuthority(SpatialConstants::GLOBAL_STATE_MANAGER, SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID))
	{
		UE_LOG(LogGlobalStateManager, Warning, TEXT("UpdateSingletonEntityId: no authority over the GSM! Update will not be sent. Singleton class: %s, entity: %lld"), *ClassName, SingletonEntityId);
		return;
	}

	Worker_ComponentUpdate Update = {};
	Update.component_id = SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID);
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	AddStringToEntityMapToSchema(UpdateObject, 1, SingletonNameToEntityId);

	NetDriver->Connection->SendComponentUpdate(SpatialConstants::GLOBAL_STATE_MANAGER, &Update);
}

void UGlobalStateManager::GetSingletonActorAndChannel(FString ClassName, AActor*& OutActor, USpatialActorChannel*& OutChannel)
{
	OutActor = nullptr;
	OutChannel = nullptr;

	UClass* SingletonActorClass = LoadObject<UClass>(nullptr, *ClassName);

	if (SingletonActorClass == nullptr)
	{
		UE_LOG(LogGlobalStateManager, Error, TEXT("Failed to find Singleton Actor Class."));
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

	if (SingletonActorList.Num() == 0)
	{
		UE_LOG(LogGlobalStateManager, Error, TEXT("No Singletons of type %s exist!"), *ClassName);
		return;
	}

	if (SingletonActorList.Num() > 1)
	{
		UE_LOG(LogGlobalStateManager, Error, TEXT("More than one Singleton Actor exists of type %s"), *ClassName);
		return;
	}

	OutActor = SingletonActorList[0];

	USpatialNetConnection* Connection = Cast<USpatialNetConnection>(NetDriver->ClientConnections[0]);

	OutChannel = (USpatialActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1);
	if (OutChannel)
	{
		NetDriver->SingletonActorChannels.Add(SingletonActorClass, TPair<AActor*, USpatialActorChannel*>(OutActor, OutChannel));
	}
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
