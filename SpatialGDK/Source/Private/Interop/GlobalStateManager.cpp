// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GlobalStateManager.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialSender.h"
#include "Interop/SpatialReceiver.h"
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
	Receiver = InNetDriver->Receiver;
	GlobalStateManagerEntityId = SpatialConstants::INTIAL_GLOBAL_STATE_MANAGER;

	FindDeploymentMapURL(); // TODO: This needs a complete overhaul and is in the wrong place.
}

void UGlobalStateManager::ApplyData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	SingletonNameToEntityId = GetStringToEntityMapFromSchema(ComponentObject, 1);
}

void UGlobalStateManager::ApplyMapData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	bAcceptingPlayers = Schema_GetBool(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID);

	if (bAcceptingPlayers)
	{
		UE_LOG(LogGlobalStateManager, Error, TEXT("GlobalStateManager ApplyData - Accepting new players"));
		AcceptingPlayersChanged.ExecuteIfBound(bAcceptingPlayers);
	}
}

void UGlobalStateManager::ApplyUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (Schema_GetObjectCount(ComponentObject, 1) > 0)
	{
		SingletonNameToEntityId = GetStringToEntityMapFromSchema(ComponentObject, 1);
	}
}

void UGlobalStateManager::ApplyMapUpdate(const Worker_ComponentUpdate& Update)
{
	UE_LOG(LogGlobalStateManager, Error, TEXT("GlobalStateManager Update"));

	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (Schema_GetObjectCount(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID) == 1)
	{
		UE_LOG(LogGlobalStateManager, Error, TEXT("GlobalStateManager Update - Accepting new players"));
		bAcceptingPlayers = Schema_GetBool(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID);
		AcceptingPlayersChanged.ExecuteIfBound(bAcceptingPlayers);
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

		improbable::UnrealMetadata* UnrealMetadata = StaticComponentView->GetComponentData<improbable::UnrealMetadata>(SingletonEntityId);
		if (UnrealMetadata == nullptr)
		{
			// Don't have entity checked out
			continue;
		}

		// Since the entity already exists, we have to handle setting up the PackageMap properly for this Actor
		NetDriver->PackageMap->ResolveEntityActor(SingletonActor, SingletonEntityId, UnrealMetadata->SubobjectNameToOffset);
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
		if (Channel->Actor)
		{
			// Channel already has an Actor
			UE_LOG(LogGlobalStateManager, Error, TEXT("Singleton Actor already has a channel: %s"), *SingletonActor->GetClass()->GetName());
			continue;
		}

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

	Worker_ComponentUpdate Update = {};
	Update.component_id = SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID);
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	AddStringToEntityMapToSchema(UpdateObject, 1, SingletonNameToEntityId);

	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

// if we are the GSM then we can toggle if we are accepting players or not.
void UGlobalStateManager::ToggleAcceptingPlayers(bool AcceptingPlayers)
{
	Worker_ComponentUpdate Update = {};
	Update.component_id = SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL;
	Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL);
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	uint8_t AcceptingPlayersInt = uint8_t(AcceptingPlayers);

	Schema_AddBool(UpdateObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID, AcceptingPlayersInt);

	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
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

void UGlobalStateManager::FindDeploymentMapURL()
{
	Worker_EntityIdConstraint GSMConstraintEID{ Worker_EntityId{ GlobalStateManagerEntityId } };

	Worker_Constraint GSMConstraint;
	GSMConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_ENTITY_ID;
	GSMConstraint.entity_id_constraint = GSMConstraintEID;

	Worker_EntityQuery MapQuery{};
	MapQuery.constraint = GSMConstraint;
	MapQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&MapQuery);

	EntityQueryDelegate MapQueryDelegate;
	MapQueryDelegate.BindLambda([this, RequestID](Worker_EntityQueryResponseOp& Op)
	{
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("Could not find GSM via entity query: %s"), Op.message);
		}

		if (Op.result_count == 0)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("Found no GSM"));
		}

		if (Op.result_count == 1)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("Found GSM!!"));

			// Extract the map
			UE_LOG(LogGlobalStateManager, Error, TEXT("Found %u components"), Op.results->component_count);
			for (uint32_t i = 0; i < Op.results->component_count; i++)
			{
				Worker_ComponentData Data = Op.results[0].components[i];
				if (Data.component_id == SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL)
				{
					Schema_Object* SchemaObject = Schema_GetComponentDataFields(Data.schema_type);
					FString MapURL = GetStringFromSchema(SchemaObject, 1);
					this->SetDeploymentMapURL(MapURL);
				}
			}
		}
	});

	Receiver->AddEntityQueryDelegate(RequestID, MapQueryDelegate);
}

void UGlobalStateManager::SetDeploymentMapURL(FString MapURL)
{
	UE_LOG(LogGlobalStateManager, Error, TEXT("Setting DeploymentMapURL: %s"), *MapURL);
	DeploymentMapURL = MapURL;
}

void UGlobalStateManager::WorldWipe(const USpatialNetDriver::ServerTravelDelegate& Delegate)
{
	Worker_EntityIdConstraint GSMConstraintEID;
	GSMConstraintEID.entity_id = GlobalStateManagerEntityId;

	Worker_Constraint GSMConstraint;
	GSMConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_ENTITY_ID;
	GSMConstraint.entity_id_constraint = GSMConstraintEID;

	Worker_NotConstraint NotGSMConstraint;
	NotGSMConstraint.constraint = &GSMConstraint;

	Worker_Constraint WorldConstraint;
	WorldConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_NOT;
	WorldConstraint.not_constraint = NotGSMConstraint;

	Worker_EntityQuery WorldQuery{};
	WorldQuery.constraint = WorldConstraint;
	WorldQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&WorldQuery);

	EntityQueryDelegate WorldQueryDelegate;
	WorldQueryDelegate.BindLambda([this, RequestID, Delegate](Worker_EntityQueryResponseOp& Op)
	{
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("World query failed: %s"), Op.message);
		}

		if (Op.result_count == 0)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("No entities in world query"));
		}

		if (Op.result_count >= 1)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("Found some entities in the world query: %u"), Op.result_count);
			DeleteEntities(Op);
		}

		Delegate.ExecuteIfBound();
	});

	Receiver->AddEntityQueryDelegate(RequestID, WorldQueryDelegate);
}

void UGlobalStateManager::DeleteEntities(const Worker_EntityQueryResponseOp& Op)
{
	UE_LOG(LogGlobalStateManager, Error, TEXT("Deleting entities!"));

	for (uint32_t i = 0; i < Op.result_count; i++)
	{
		// Lets pray
		UE_LOG(LogGlobalStateManager, Error, TEXT("- Sending delete request for: %i"), Op.results[i].entity_id);
		NetDriver->Connection->SendDeleteEntityRequest(Op.results[i].entity_id);
	}

	// Also kill the GSM
	NetDriver->Connection->SendDeleteEntityRequest(GlobalStateManagerEntityId);
}

void DeepCopy(Schema_Object* source, Schema_Object* target) {
	auto length = Schema_GetWriteBufferLength(source);
	auto buffer = Schema_AllocateBuffer(target, length);
	Schema_WriteToBuffer(source, buffer);
	Schema_Clear(target);
	Schema_MergeFromBuffer(target, buffer, length);
}

Schema_ComponentData* Schema_DeepCopyComponentData(Schema_ComponentData* source) {
	auto* copy = Schema_CreateComponentData(Schema_GetComponentDataComponentId(source));
	DeepCopy(Schema_GetComponentDataFields(source), Schema_GetComponentDataFields(copy));
	return copy;
}

void UGlobalStateManager::LoadSnapshot()
{
	// YOLO
	Worker_ComponentVtable DefaultVtable{};
	Worker_SnapshotParameters Parameters{};
	Parameters.default_component_vtable = &DefaultVtable;

	// TODO: Move the snapshot to a variable and get it from the `snapshot=` param in the URL.
	Worker_SnapshotInputStream* Snapshot = Worker_SnapshotInputStream_Create("E:\\Projects\\UnrealGDKStarterProjectPlugin\\spatial\\snapshots\\BestMap.snapshot", &Parameters);

	FString Error = Worker_SnapshotInputStream_GetError(Snapshot);
	if (!Error.IsEmpty())
	{
		UE_LOG(LogGlobalStateManager, Error, TEXT(" Fucking error reading snapshot mate: %s"), *Error);
		return;
	}

	TArray<TArray<Worker_ComponentData>> EntitiesToSpawn;

	// Get all of the entities from the snapshot.
	while (Snapshot && Worker_SnapshotInputStream_HasNext(Snapshot) > 0)
	{
		Error = Worker_SnapshotInputStream_GetError(Snapshot);
		if (!Error.IsEmpty())
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("- Error when reading the HasNext.........."));
			return;
		}

		const Worker_Entity* EntityToSpawn = Worker_SnapshotInputStream_ReadEntity(Snapshot);

		Error = Worker_SnapshotInputStream_GetError(Snapshot);
		if (Error.IsEmpty())
		{
			TArray<Worker_ComponentData> EntityComponents;
			for (uint32_t i = 0; i < EntityToSpawn->component_count; ++i) {
				// Entity component data must be deep copied so that it can be used for CreateEntityRequest.
				auto* CopySchemaData = Schema_DeepCopyComponentData(EntityToSpawn->components[i].schema_type);
				auto EntityComponentData = Worker_ComponentData{};
				EntityComponentData.component_id = Schema_GetComponentDataComponentId(CopySchemaData);
				EntityComponentData.schema_type = CopySchemaData;
				EntityComponents.Add(EntityComponentData);
			}

			// Josh - New flow. Bulk reserve the EntityIDs and then send.
			EntitiesToSpawn.Add(EntityComponents);
		}
		else
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT(" Fucking error reading entity mate: %s"), *Error);
			// Abort
			Worker_SnapshotInputStream_Destroy(Snapshot);
			return;
		}
	}

	// End it
	Worker_SnapshotInputStream_Destroy(Snapshot);

	// TODO: This needs retry logic in case of failures.
	// Set up reserve IDs delegate
	ReserveEntityIDsDelegate SpawnEntitiesDelegate;
	SpawnEntitiesDelegate.BindLambda([EntitiesToSpawn, this] (Worker_ReserveEntityIdsResponseOp& Op) { // Need to get the reserved IDs in here.
		UE_LOG(LogGlobalStateManager, Error, TEXT("Creating entities in snapshot, num of entities: %i"), Op.number_of_entity_ids);

		// Ensure we have the same number of reserved IDs as we have entities to spawn
		check(EntitiesToSpawn.Num() == Op.number_of_entity_ids);

		for (uint32_t i = 0; i < Op.number_of_entity_ids; i++)
		{
			// Get an entity to spawn and a reserved EntityID
			auto& EntityToSpawn = EntitiesToSpawn[i];
			Worker_EntityId EntityID = Op.first_entity_id + i;

			// Check if this is the GSM
			for (auto& ComponentData : EntityToSpawn)
			{
				if (ComponentData.component_id == SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID)
				{
					// Save the ID in this class
					GlobalStateManagerEntityId = EntityID;
				}
			}

			UE_LOG(LogGlobalStateManager, Warning, TEXT("- Sending entity create request for: %i"), EntityID);
			NetDriver->Connection->SendCreateEntityRequest(EntityToSpawn.Num(), EntityToSpawn.GetData(), &EntityID /**Reserved entity ID**/);

			// Once all the requests have been sent we can start accepting players.
			ToggleAcceptingPlayers(true);
		}
	});

	// Reserve the Entity IDs
	Worker_RequestId ReserveRequestID = NetDriver->Connection->SendReserveEntityIdsRequest(EntitiesToSpawn.Num());

	// Add the spawn delegate
	Receiver->AddReserveEntityIdsDelegate(ReserveRequestID, SpawnEntitiesDelegate);
}

void UGlobalStateManager::SetupGSMStreamingQuery()
{

}
