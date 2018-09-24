// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/GlobalStateManager.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/EntityRegistry.h"

DEFINE_LOG_CATEGORY(LogGlobalStateManager);

using namespace improbable;

void UGlobalStateManager::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Sender = InNetDriver->Sender;
	Receiver = InNetDriver->Receiver;
	TimerManager = InTimerManager;
	GlobalStateManagerEntityId = SpatialConstants::INTIAL_GLOBAL_STATE_MANAGER;
}

void UGlobalStateManager::ApplyData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	SingletonNameToEntityId = GetStringToEntityMapFromSchema(ComponentObject, 1);
}

void UGlobalStateManager::ApplyMapData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	
	// Set the Map URL.
	if(Schema_GetObjectCount(ComponentObject, 1) == 1)
	{
		SetDeploymentMapURL(GetStringFromSchema(ComponentObject, 1));
	}

	if(Schema_GetBoolCount(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID) == 1)
	{
		// Set the AcceptingPlayers state.
		bool bDataAcceptingPlayers = bool(Schema_GetBool(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID));

		if (bDataAcceptingPlayers)
		{
			if (bDataAcceptingPlayers != bAcceptingPlayers)
			{
				UE_LOG(LogGlobalStateManager, Log, TEXT("GlobalStateManager ApplyMapData - AcceptingPlayers: %s"), bDataAcceptingPlayers ? TEXT("true") : TEXT("false"));
				bAcceptingPlayers = bDataAcceptingPlayers;
				AcceptingPlayersChanged.ExecuteIfBound(bAcceptingPlayers);
			}
		}
		else
		{
			if (!NetDriver->IsServer())
			{
				// TODO: UNR-656 - TLDR: Hack to get around runtime not giving data on streaming queries unless you have write authority.
				// There is currently a bug in runtime which prevents clients from being able to have read access on the component via the streaming query.
				// This means that the clients never actually receive updates or data on the GSM. To get around this we are making timed entity queries to
				// find the state of the GSM and the accepting players. Remove this work-around when the runtime bug is fixed.
				UE_LOG(LogGlobalStateManager, Warning, TEXT("ApplyMapData - Not yet accepting new players, trying again..."));
				QueryGSM(true /*bWithRetry*/);
			}
		}
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
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (Schema_GetObjectCount(ComponentObject, 1) == 1)
	{
		SetDeploymentMapURL(GetStringFromSchema(ComponentObject, 1));
	}

	if (Schema_GetBoolCount(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID) == 1)
	{
		bool bUpdateAcceptingPlayers = bool(Schema_GetBool(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID));
		if (bUpdateAcceptingPlayers != bAcceptingPlayers)
		{
			UE_LOG(LogGlobalStateManager, Log, TEXT("GlobalStateManager Update - AcceptingPlayers: %s"), bUpdateAcceptingPlayers ? TEXT("true") : TEXT("false"));
			bAcceptingPlayers = bUpdateAcceptingPlayers;
			AcceptingPlayersChanged.ExecuteIfBound(bAcceptingPlayers);
		}
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

void UGlobalStateManager::ToggleAcceptingPlayers(bool bInAcceptingPlayers)
{
	if (!bHasLiveMapAuthority || !Cast<USpatialGameInstance>(NetDriver->GetWorld()->GetGameInstance())->bIsWorkerAuthorativeOverGSM)
	{
		UE_LOG(LogGlobalStateManager, Error, TEXT("Tried to toggle AcceptingPlayers but this worker is not authorative over the GSM"));
	}

	// GSM Constraint
	Worker_Constraint GSMConstraint;
	GSMConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	GSMConstraint.component_constraint.component_id = SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL;

	// GSM Query
	Worker_EntityQuery GSMQuery{};
	GSMQuery.constraint = GSMConstraint;
	GSMQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&GSMQuery);

	EntityQueryDelegate GSMQueryDelegate;
	GSMQueryDelegate.BindLambda([this, bInAcceptingPlayers](Worker_EntityQueryResponseOp& Op)
	{
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS || Op.result_count == 0)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("Tried to toggle AcceptingPlayers but the query for the GSM failed. Trying again in 3 seconds."));
			FTimerHandle RetryTimer;
			TimerManager->SetTimer(RetryTimer, [this, bInAcceptingPlayers]()
			{
				ToggleAcceptingPlayers(bInAcceptingPlayers);
			}, 3.f, false);
		}
		else if (Op.result_count == 1)
		{
			// We have the GSM from the query. Update our local GSM entity ID
			GlobalStateManagerEntityId = Op.results->entity_id;

			// Send the component update that we can now accept players.
			UE_LOG(LogGlobalStateManager, Log, TEXT("Toggling accepting players to '%s'"), bInAcceptingPlayers ? TEXT("true") : TEXT("false"));
			Worker_ComponentUpdate Update = {};
			Update.component_id = SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL;
			Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL);
			Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

			// Set the map URL on the GSM.
			AddStringToSchema(UpdateObject, 1, NetDriver->GetWorld()->URL.ToString());

			// Set the AcceptingPlayers state on the GSM
			Schema_AddBool(UpdateObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID, uint8_t(bInAcceptingPlayers));

			// Component updates are short circuited so we set the updated state here and then send the component update.
			bAcceptingPlayers = bInAcceptingPlayers;
			NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
		}
	});

	Receiver->AddEntityQueryDelegate(RequestID, GSMQueryDelegate);
}

void UGlobalStateManager::AuthorityChanged(bool bWorkerAuthority, Worker_EntityId CurrentEntityID)
{
	// Make sure the GameInstance knows that this worker is now authoritative over the GSM (used for server travel).
	// The GameInstance is the only persistent object during server travel.
	// bIsWorkerAuthorativeOverGSM exists to inform the worker that was previously authoritative over the GSM that it has the responsibility of loading the new snapshot.
	Cast<USpatialGameInstance>(NetDriver->GetWorld()->GetGameInstance())->bIsWorkerAuthorativeOverGSM = bWorkerAuthority;

	// Also update this instance of the GSM that it has current authority (used for accepting players toggle).
	// The instance of each GSM is destroyed on server travel and a new one is made, hence the need for 'live' authority.
	bHasLiveMapAuthority = bWorkerAuthority;

	// Make sure we update our known entity id for the GSM when we receive authority.
	GlobalStateManagerEntityId = CurrentEntityID;

	OnAuthorityChanged.ExecuteIfBound(bWorkerAuthority);
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

// Queries for the GlobalStateManager in the deployment.
// bWithRetry will continue querying until the state of AcceptingPlayers is true, this is so clients know when to connect to the deployment.
void UGlobalStateManager::QueryGSM(bool bWithRetry)
{
	Worker_ComponentConstraint GSMComponentConstraint{};
	GSMComponentConstraint.component_id = SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL;

	Worker_Constraint GSMConstraint;
	GSMConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	GSMConstraint.component_constraint = GSMComponentConstraint;

	Worker_EntityQuery GSMQuery{};
	GSMQuery.constraint = GSMConstraint;
	GSMQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&GSMQuery);

	EntityQueryDelegate GSMQueryDelegate;
	GSMQueryDelegate.BindLambda([this, bWithRetry](Worker_EntityQueryResponseOp& Op)
	{
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS || Op.result_count == 0)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("Could not find GSM via entity query: %s"), UTF8_TO_TCHAR(Op.message));
			if (bWithRetry)
			{
				UE_LOG(LogGlobalStateManager, Warning, TEXT("Retrying entity query for the GSM in 3 seconds"));
				FTimerHandle RetryTimer;
				TimerManager->SetTimer(RetryTimer, [this, bWithRetry]()
				{
					QueryGSM(bWithRetry);
				}, 3.f, false);
			}
		}

		if (Op.result_count == 1)
		{
			for (uint32_t i = 0; i < Op.results->component_count; i++)
			{
				Worker_ComponentData Data = Op.results[0].components[i];
				if (Data.component_id == SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL)
				{
					UE_LOG(LogGlobalStateManager, Log, TEXT("GlobalStateManager found via entity query. Applying MapData."));
					ApplyMapData(Data);
				}
			}
		}
	});

	Receiver->AddEntityQueryDelegate(RequestID, GSMQueryDelegate);
}

void UGlobalStateManager::AuthorityChanged(bool bWorkerAuthority)
{
	// Make sure the GameInstance knows that this worker is now authoritative over the GSM (used for server travel).
	// The GameInstance is the only persistent object during server travel.
	// bIsWorkerAuthorativeOverGSM exists to inform the worker that was previously authoritative over the GSM that it has the responsibility of loading the new snapshot.
	Cast<USpatialGameInstance>(NetDriver->GetWorld()->GetGameInstance())->bIsWorkerAuthorativeOverGSM = bWorkerAuthority;

	// Also update this instance of the GSM that it has current authority (used for accepting players toggle).
	// The instance of each GSM is destroyed on server travel and a new one is made, hence the need for 'live' authority.
	bHasLiveMapAuthority = bWorkerAuthority;

	OnAuthorityChanged.ExecuteIfBound(bWorkerAuthority);
}

void UGlobalStateManager::SetDeploymentMapURL(const FString& MapURL)
{
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting DeploymentMapURL: %s"), *MapURL);
	DeploymentMapURL = MapURL;
}
