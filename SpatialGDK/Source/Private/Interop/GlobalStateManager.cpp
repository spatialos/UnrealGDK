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
	GlobalStateManagerEntityId = SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID;
}

void UGlobalStateManager::ApplyData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	SingletonNameToEntityId = GetStringToEntityMapFromSchema(ComponentObject, 1);
}

void UGlobalStateManager::ApplyDeploymentMapURLData(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
	
	// Set the Deployment Map URL.
	if (Schema_GetObjectCount(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL_ID) == 1)
	{
		SetDeploymentMapURL(GetStringFromSchema(ComponentObject, 1));
	}

	// Set the AcceptingPlayers state.
	if (Schema_GetBoolCount(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID) == 1)
	{
		bool bDataAcceptingPlayers = !!Schema_GetBool(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID);
		ApplyAcceptingPlayersUpdate(bDataAcceptingPlayers);
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

void UGlobalStateManager::ApplyDeploymentMapUpdate(const Worker_ComponentUpdate& Update)
{
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	if (Schema_GetObjectCount(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL_ID) == 1)
	{
		SetDeploymentMapURL(GetStringFromSchema(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL_ID));
	}

	if (Schema_GetBoolCount(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID) == 1)
	{
		bool bUpdateAcceptingPlayers = !!Schema_GetBool(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID);
		ApplyAcceptingPlayersUpdate(bUpdateAcceptingPlayers);
	}
}

void UGlobalStateManager::ApplyAcceptingPlayersUpdate(bool bAcceptingPlayersUpdate)
{
	if (bAcceptingPlayersUpdate != bAcceptingPlayers)
	{
		UE_LOG(LogGlobalStateManager, Log, TEXT("GlobalStateManager Update - AcceptingPlayers: %s"), bAcceptingPlayersUpdate ? TEXT("true") : TEXT("false"));
		bAcceptingPlayers = bAcceptingPlayersUpdate;

		// Tell the SpatialNetDriver that AcceptingPlayers has changed.
		NetDriver->OnAcceptingPlayersChanged(bAcceptingPlayersUpdate);
	}
}

void UGlobalStateManager::LinkExistingSingletonActor(const UClass* SingletonActorClass)
{
	const Worker_EntityId* SingletonEntityIdPtr = SingletonNameToEntityId.Find(SingletonActorClass->GetPathName());
	if (SingletonEntityIdPtr == nullptr)
	{
		// No entry in SingletonNameToEntityId for this singleton class type
		UE_LOG(LogGlobalStateManager, Log, TEXT("LinkExistingSingletonActor %s failed to find entry"), *SingletonActorClass->GetName());
		return;
	}

	const Worker_EntityId SingletonEntityId = *SingletonEntityIdPtr;
	if (SingletonEntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		// Singleton Entity hasn't been created yet
		UE_LOG(LogGlobalStateManager, Log, TEXT("LinkExistingSingletonActor %s entity id is invalid"), *SingletonActorClass->GetName());
		return;
	}

	auto* ActorChannelPair = NetDriver->SingletonActorChannels.Find(SingletonActorClass);
	if (ActorChannelPair == nullptr)
	{
		// Dynamically spawn singleton actor if we have queued up data - ala USpatialReceiver::ReceiveActor - JIRA: 735

		// No local actor has registered itself as replicatible on this worker
		UE_LOG(LogGlobalStateManager, Warning, TEXT("LinkExistingSingletonActor no actor registered"), *SingletonActorClass->GetName());
		return;
	}

	AActor*& SingletonActor = ActorChannelPair->Key;
	USpatialActorChannel*& Channel = ActorChannelPair->Value;

	if (Channel != nullptr)
	{
		// Channel has already been setup
		UE_LOG(LogGlobalStateManager, Warning, TEXT("UGlobalStateManager::LinkExistingSingletonActor channel already setup"), *SingletonActorClass->GetName());
		return;
	}

	// If we have previously queued up data for this entity, apply it - JIRA: 734

	USpatialNetConnection* Connection = Cast<USpatialNetConnection>(NetDriver->ClientConnections[0]);
	Channel = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, 1));

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

void UGlobalStateManager::LinkExistingSingletonActors()
{
	// Client receive Singleton Actors via the normal Unreal replicated actor flow
	if (!NetDriver->IsServer())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("0x%x UGlobalStateManager::LinkExistingSingletonActors %d"), this, SingletonNameToEntityId.Num());

	for (const auto& Pair : SingletonNameToEntityId)
	{
		UClass* SingletonActorClass = LoadObject<UClass>(nullptr, *Pair.Key);
		if (SingletonActorClass == nullptr)
		{
			UE_LOG(LogGlobalStateManager, Error, TEXT("Failed to find Singleton Actor Class."));
			continue;
		}

		LinkExistingSingletonActor(SingletonActorClass);
	}
}

USpatialActorChannel* UGlobalStateManager::AddSingleton(AActor* SingletonActor)
{
	UE_LOG(LogTemp, Warning, TEXT("0x%x UGlobalStateManager::AddSingleton %s"), this, *SingletonActor->GetName());
	check(SingletonActor->GetIsReplicated());

	UClass* SingletonActorClass = SingletonActor->GetClass();

	auto& ActorChannelPair = NetDriver->SingletonActorChannels.FindOrAdd(SingletonActorClass);
	USpatialActorChannel*& Channel = ActorChannelPair.Value;
	check(ActorChannelPair.Key == nullptr || ActorChannelPair.Key == SingletonActor);
	ActorChannelPair.Key = SingletonActor;

	// Just return the channel if it's already been setup
	if (Channel != nullptr)
	{
		return Channel;
	}

	bool bHasGSMAuthority = NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::GLOBAL_STATE_MANAGER_COMPONENT_ID);
	if (bHasGSMAuthority)
	{
		// We have control over the GSM, so can safely setup a new channel and let it allocate an entity id
		USpatialNetConnection* Connection = Cast<USpatialNetConnection>(NetDriver->ClientConnections[0]);
		Channel = Cast<USpatialActorChannel>(Connection->CreateChannel(CHTYPE_Actor, 1));

		SingletonActor->Role = ROLE_Authority;
		SingletonActor->RemoteRole = ROLE_SimulatedProxy;

		// If entity id already exists for this singleton, set the actor to it
		// Otherwise SetChannelActor will issue a new entity id request
		if (const Worker_EntityId* SingletonEntityId = SingletonNameToEntityId.Find(SingletonActorClass->GetPathName()))
		{
			NetDriver->GetEntityRegistry()->AddToRegistry(*SingletonEntityId, SingletonActor);
		}

		Channel->SetChannelActor(SingletonActor);
		UE_LOG(LogGlobalStateManager, Log, TEXT("0x%x Started replication of Singleton Actor %s"), this, *SingletonActor->GetClass()->GetName());
	}
	else
	{
		// We don't have control over the GSM, but we may have received the entityid for this singleton already
		LinkExistingSingletonActor(SingletonActorClass);
	}

	return Channel;
}

void UGlobalStateManager::ExecuteInitialSingletonActorReplication()
{
	UE_LOG(LogTemp, Warning, TEXT("0x%x UGlobalStateManager::ExecuteInitialSingletonActorReplication %d"), this, NetDriver->SingletonActorChannels.Num());

	for (auto& ClassToActorChannel : NetDriver->SingletonActorChannels)
	{
		auto& ActorChannelPair = ClassToActorChannel.Value;
		AddSingleton(ActorChannelPair.Key);
	}
}

void UGlobalStateManager::UpdateSingletonEntityId(const FString& ClassName, const Worker_EntityId SingletonEntityId)
{
	Worker_EntityId& EntityId = SingletonNameToEntityId.FindOrAdd(ClassName);
	EntityId = SingletonEntityId;

	if (!NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID))
	{
		UE_LOG(LogGlobalStateManager, Warning, TEXT("0x%x UGlobalStateManager::UpdateSingletonEntityId: no authority over the GSM! Update will not be sent. Singleton class: %s, entity: %lld"), this, *ClassName, SingletonEntityId);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("0x%x UGlobalStateManager::UpdateSingletonEntityId %d"), this, SingletonNameToEntityId.Num());
	Worker_ComponentUpdate Update = {};
	Update.component_id = SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::SINGLETON_MANAGER_COMPONENT_ID);
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	AddStringToEntityMapToSchema(UpdateObject, 1, SingletonNameToEntityId);

	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
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

void UGlobalStateManager::SetAcceptingPlayers(bool bInAcceptingPlayers)
{
	if (!NetDriver->StaticComponentView->HasAuthority(GlobalStateManagerEntityId, SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID))
	{
		UE_LOG(LogGlobalStateManager, Warning, TEXT("Tried to set AcceptingPlayers on the GSM but this worker does not have authority."));
		return;
	}

	// Send the component update that we can now accept players.
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting accepting players to '%s'"), bInAcceptingPlayers ? TEXT("true") : TEXT("false"));
	Worker_ComponentUpdate Update = {};
	Update.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;
	Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID);
	Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);

	// Set the map URL on the GSM.
	AddStringToSchema(UpdateObject, SpatialConstants::GLOBAL_STATE_MANAGER_MAP_URL_ID, NetDriver->GetWorld()->URL.Map);

	// Set the AcceptingPlayers state on the GSM
	Schema_AddBool(UpdateObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID, uint8_t(bInAcceptingPlayers));

	// Component updates are short circuited so we set the updated state here and then send the component update.
	bAcceptingPlayers = bInAcceptingPlayers;
	NetDriver->Connection->SendComponentUpdate(GlobalStateManagerEntityId, &Update);
}

void UGlobalStateManager::AuthorityChanged(bool bWorkerAuthority, Worker_EntityId CurrentEntityID)
{
	UE_LOG(LogGlobalStateManager, Log, TEXT("Authority over the GSM has changed. This worker %s authority."),  bWorkerAuthority ? TEXT("now has") : TEXT ("does not have"));

	if (bWorkerAuthority)
	{
		// Make sure we update our known entity id for the GSM when we receive authority.
		GlobalStateManagerEntityId = CurrentEntityID;
		SetAcceptingPlayers(true);
	}
}

// Queries for the GlobalStateManager in the deployment.
// bRetryUntilAcceptingPlayers will continue querying until the state of AcceptingPlayers is true, this is so clients know when to connect to the deployment.
void UGlobalStateManager::QueryGSM(bool bRetryUntilAcceptingPlayers)
{
	Worker_ComponentConstraint GSMComponentConstraint{};
	GSMComponentConstraint.component_id = SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID;

	Worker_Constraint GSMConstraint{};
	GSMConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	GSMConstraint.component_constraint = GSMComponentConstraint;

	Worker_EntityQuery GSMQuery{};
	GSMQuery.constraint = GSMConstraint;
	GSMQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&GSMQuery);

	EntityQueryDelegate GSMQueryDelegate;
	GSMQueryDelegate.BindLambda([this, bRetryUntilAcceptingPlayers](Worker_EntityQueryResponseOp& Op)
	{
		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogGlobalStateManager, Warning, TEXT("Could not find GSM via entity query: %s"), UTF8_TO_TCHAR(Op.message));
		}
		else if (Op.result_count == 0)
		{
			UE_LOG(LogGlobalStateManager, Log, TEXT("GSM entity query shows the GSM does not yet exist in the world."));
		}
		else
		{
			bool bNewAcceptingPlayers = GetAcceptingPlayersFromQueryResponse(Op);

			if (!bNewAcceptingPlayers && bRetryUntilAcceptingPlayers)
			{
				UE_LOG(LogGlobalStateManager, Log, TEXT("Not yet accepting new players. Will retry query for GSM."));
				RetryQueryGSM(bRetryUntilAcceptingPlayers);
			}
			else
			{
				ApplyDeploymentMapDataFromQueryResponse(Op);
			}

			return;
		}

		if (bRetryUntilAcceptingPlayers)
		{
			RetryQueryGSM(bRetryUntilAcceptingPlayers);
		}
	});

	Receiver->AddEntityQueryDelegate(RequestID, GSMQueryDelegate);
}

void UGlobalStateManager::ApplyDeploymentMapDataFromQueryResponse(Worker_EntityQueryResponseOp& Op)
{
	for (uint32_t i = 0; i < Op.results[0].component_count; i++)
	{
		Worker_ComponentData Data = Op.results[0].components[i];
		if (Data.component_id == SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID)
		{
			ApplyDeploymentMapURLData(Data);
		}
	}
}

bool UGlobalStateManager::GetAcceptingPlayersFromQueryResponse(Worker_EntityQueryResponseOp& Op)
{
	checkf(Op.result_count == 1, TEXT("There should never be more than one GSM"));

	// Iterate over each component on the GSM until we get the DeploymentMap component.
	for (uint32_t i = 0; i < Op.results[0].component_count; i++)
	{
		Worker_ComponentData Data = Op.results[0].components[i];
		if (Data.component_id == SpatialConstants::DEPLOYMENT_MAP_COMPONENT_ID)
		{
			Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

			if (Schema_GetBoolCount(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID) == 1)
			{
				bool bDataAcceptingPlayers = !!Schema_GetBool(ComponentObject, SpatialConstants::GLOBAL_STATE_MANAGER_ACCEPTING_PLAYERS_ID);
				return bDataAcceptingPlayers;
			}
		}
	}

	UE_LOG(LogGlobalStateManager, Warning, TEXT("Entity query response for the GSM did not contain an AcceptingPlayers state."));

	return false;
}

void UGlobalStateManager::RetryQueryGSM(bool bRetryUntilAcceptingPlayers)
{
	// TODO: UNR-656 - TLDR: Hack to get around runtime not giving data on streaming queries unless you have write authority.
	// There is currently a bug in runtime which prevents clients from being able to have read access on the component via the streaming query.
	// This means that the clients never actually receive updates or data on the GSM. To get around this we are making timed entity queries to
	// find the state of the GSM and the accepting players. Remove this work-around when the runtime bug is fixed.
	float RetryTimerDelay = SpatialConstants::ENTITY_QUERY_RETRY_WAIT_SECONDS;

	// In PIE we want to retry the entity query as soon as possible.
#if WITH_EDITOR
	RetryTimerDelay = 0.1f;
#endif

	UE_LOG(LogGlobalStateManager, Log, TEXT("Retrying query for GSM in %f seconds"), RetryTimerDelay);
	FTimerHandle RetryTimer;
	TimerManager->SetTimer(RetryTimer, [this, bRetryUntilAcceptingPlayers]()
	{
		QueryGSM(bRetryUntilAcceptingPlayers);
	}, RetryTimerDelay, false);
}

void UGlobalStateManager::SetDeploymentMapURL(const FString& MapURL)
{
	UE_LOG(LogGlobalStateManager, Log, TEXT("Setting DeploymentMapURL: %s"), *MapURL);
	DeploymentMapURL = MapURL;
}
