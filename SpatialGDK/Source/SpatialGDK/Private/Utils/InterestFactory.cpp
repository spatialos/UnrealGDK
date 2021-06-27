// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/InterestFactory.h"

#include "EngineClasses/Components/ActorInterestComponent.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/GDKPropertyMacros.h"
#include "Utils/Interest/NetCullDistanceInterest.h"

#include "Engine/Classes/GameFramework/Actor.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialReplicationGraph.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "ReplicationGraph.h"
#include "UObject/UObjectIterator.h"
#include "Utils/MetricsExport.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategoryReplicator.h"
#endif

DEFINE_LOG_CATEGORY(LogInterestFactory);

DECLARE_STATS_GROUP(TEXT("InterestFactory"), STATGROUP_SpatialInterestFactory, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("AddUserDefinedQueries"), STAT_InterestFactoryAddUserDefinedQueries, STATGROUP_SpatialInterestFactory);

namespace SpatialGDK
{
InterestFactory::InterestFactory(USpatialClassInfoManager* InClassInfoManager)
	: ClassInfoManager(InClassInfoManager)
{
	check(ClassInfoManager != nullptr);
	CreateAndCacheInterestState();
}

void InterestFactory::CreateAndCacheInterestState()
{
	ClientCheckoutRadiusConstraint = NetCullDistanceInterest::CreateCheckoutRadiusConstraints(ClassInfoManager);
	ClientNonAuthInterestResultType = CreateClientNonAuthInterestResultType();
	ClientAuthInterestResultType = CreateClientAuthInterestResultType();
	ServerNonAuthInterestResultType = CreateServerNonAuthInterestResultType();
	ServerAuthInterestResultType = CreateServerAuthInterestResultType();
}

SchemaResultType InterestFactory::CreateClientNonAuthInterestResultType()
{
	SchemaResultType ClientNonAuthResultType{};

	// Add the required unreal components
	ClientNonAuthResultType.ComponentIds.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST);

	// Add all data components- clients don't need to see handover or owner only components on other entities.
	ClientNonAuthResultType.ComponentSetsIds.Push(SpatialConstants::DATA_COMPONENT_SET_ID);

	// If Initial Only is disabled, add full interest in the Initial Only data
	if (!GetDefault<USpatialGDKSettings>()->bEnableInitialOnlyReplicationCondition)
	{
		ClientNonAuthResultType.ComponentSetsIds.Push(SpatialConstants::INITIAL_ONLY_COMPONENT_SET_ID);
	}

	return ClientNonAuthResultType;
}

SchemaResultType InterestFactory::CreateClientAuthInterestResultType()
{
	SchemaResultType ClientAuthResultType;

	// Add the required known components
	ClientAuthResultType.ComponentIds.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_AUTH_CLIENT_INTEREST);
	ClientAuthResultType.ComponentIds.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST);

	// Add all the generated unreal components
	ClientAuthResultType.ComponentSetsIds.Push(SpatialConstants::DATA_COMPONENT_SET_ID);
	ClientAuthResultType.ComponentSetsIds.Push(SpatialConstants::OWNER_ONLY_COMPONENT_SET_ID);

	// If Initial Only is disabled, add full interest in the Initial Only data
	if (!GetDefault<USpatialGDKSettings>()->bEnableInitialOnlyReplicationCondition)
	{
		ClientAuthResultType.ComponentSetsIds.Push(SpatialConstants::INITIAL_ONLY_COMPONENT_SET_ID);
	}

	return ClientAuthResultType;
}

SchemaResultType InterestFactory::CreateServerNonAuthInterestResultType()
{
	SchemaResultType ServerNonAuthResultType{};

	// Add the required unreal components
	ServerNonAuthResultType.ComponentIds.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_SERVER_INTEREST);

	// Add all data, owner only, handover and initial only components
	ServerNonAuthResultType.ComponentSetsIds.Push(SpatialConstants::DATA_COMPONENT_SET_ID);
	ServerNonAuthResultType.ComponentSetsIds.Push(SpatialConstants::OWNER_ONLY_COMPONENT_SET_ID);
	ServerNonAuthResultType.ComponentSetsIds.Push(SpatialConstants::HANDOVER_COMPONENT_SET_ID);
	ServerNonAuthResultType.ComponentSetsIds.Push(SpatialConstants::INITIAL_ONLY_COMPONENT_SET_ID);

	return ServerNonAuthResultType;
}

SchemaResultType InterestFactory::CreateServerAuthInterestResultType()
{
	SchemaResultType ServerAuthResultType{};
	// Just the components that we won't have already checked out through authority
	ServerAuthResultType.ComponentIds.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_AUTH_SERVER_INTEREST);
	return ServerAuthResultType;
}

Worker_ComponentData UnrealServerInterestFactory::CreateInterestData(AActor* InActor, const FClassInfo& InInfo,
																	 const Worker_EntityId InEntityId) const
{
	return CreateInterest(InActor, InInfo, InEntityId).CreateComponentData();
}

Worker_ComponentUpdate UnrealServerInterestFactory::CreateInterestUpdate(AActor* InActor, const FClassInfo& InInfo,
																		 const Worker_EntityId InEntityId) const
{
	return CreateInterest(InActor, InInfo, InEntityId).CreateInterestUpdate();
}

Interest InterestFactory::CreateServerWorkerInterest(const UAbstractLBStrategy* LBStrategy) const
{
	// Build the Interest component as we go by updating the component-> query list mappings.
	Interest ServerInterest;

	Query ServerQuery{};

	// Workers have interest in all system worker entities.
	ServerQuery = Query();
	ServerQuery.ResultComponentIds = { SpatialConstants::WORKER_COMPONENT_ID,
									   /* System component query tag */ SpatialConstants::SYSTEM_COMPONENT_ID };
	ServerQuery.Constraint.ComponentConstraint = SpatialConstants::WORKER_COMPONENT_ID;
	AddComponentQueryPairToInterestComponent(ServerInterest, SpatialConstants::SERVER_WORKER_ENTITY_AUTH_COMPONENT_SET_ID, ServerQuery);

	// And an interest in all server worker entities.
	ServerQuery = Query();
	ServerQuery.ResultComponentIds = { SpatialConstants::SERVER_WORKER_COMPONENT_ID, SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID };
	ServerQuery.Constraint.ComponentConstraint = SpatialConstants::SERVER_WORKER_COMPONENT_ID;
	AddComponentQueryPairToInterestComponent(ServerInterest, SpatialConstants::SERVER_WORKER_ENTITY_AUTH_COMPONENT_SET_ID, ServerQuery);

	// Ensure server worker receives core GDK snapshot entities.
	ServerQuery = Query();
	ServerQuery.ResultComponentIds = ServerNonAuthInterestResultType.ComponentIds;
	ServerQuery.ResultComponentSetIds = ServerNonAuthInterestResultType.ComponentSetsIds;
	ServerQuery.Constraint = CreateGDKSnapshotEntitiesConstraint();
	AddComponentQueryPairToInterestComponent(ServerInterest, SpatialConstants::SERVER_WORKER_ENTITY_AUTH_COMPONENT_SET_ID, ServerQuery);

	// SelfInterest for routing worker components.
	ServerQuery = Query();
	ServerQuery.ResultComponentIds = { SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID,
									   SpatialConstants::CROSS_SERVER_SENDER_ENDPOINT_COMPONENT_ID,
									   SpatialConstants::CROSS_SERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID,
									   SpatialConstants::CROSS_SERVER_RECEIVER_ENDPOINT_COMPONENT_ID };
	ServerQuery.Constraint.bSelfConstraint = true;
	AddComponentQueryPairToInterestComponent(ServerInterest, SpatialConstants::SERVER_WORKER_ENTITY_AUTH_COMPONENT_SET_ID, ServerQuery);

	return ServerInterest;
}

Interest InterestFactory::CreatePartitionInterest(const SpatialGDK::QueryConstraint& LoadBalancingConstraint, bool bDebug) const
{
	// Add load balancing query
	Interest PartitionInterest{};

	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	const Worker_ComponentId PartitionAuthComponentSet = Settings->bRunStrategyWorker
															 ? SpatialConstants::PARTITION_WORKER_AUTH_COMPONENT_SET_ID
															 : SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID;

	const Worker_ComponentId PartitionCompletenessTag = Settings->bRunStrategyWorker ? SpatialConstants::PARTITION_AUTH_TAG_COMPONENT_ID
																					 : SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID;

	// Add load balancing query
	Query PartitionQuery{};
	PartitionQuery.ResultComponentIds = ServerNonAuthInterestResultType.ComponentIds;
	PartitionQuery.ResultComponentSetIds = ServerNonAuthInterestResultType.ComponentSetsIds;
	PartitionQuery.Constraint = LoadBalancingConstraint;
	AddComponentQueryPairToInterestComponent(PartitionInterest, PartitionAuthComponentSet, PartitionQuery);

	// Ensure server worker receives AlwaysRelevant entities.
	PartitionQuery = Query();
	PartitionQuery.ResultComponentIds = ServerNonAuthInterestResultType.ComponentIds;
	PartitionQuery.ResultComponentSetIds = ServerNonAuthInterestResultType.ComponentSetsIds;
	PartitionQuery.Constraint = CreateServerAlwaysRelevantConstraint();
	AddComponentQueryPairToInterestComponent(PartitionInterest, PartitionAuthComponentSet, PartitionQuery);

	// Add a self query for completeness
	PartitionQuery = Query();
	PartitionQuery.ResultComponentIds = { PartitionCompletenessTag };
	PartitionQuery.Constraint.bSelfConstraint = true;
	AddComponentQueryPairToInterestComponent(PartitionInterest, PartitionAuthComponentSet, PartitionQuery);

	// Query to know about all the actors tagged with a debug component
	if (bDebug)
	{
		PartitionQuery = Query();
		PartitionQuery.ResultComponentIds = { SpatialConstants::GDK_DEBUG_COMPONENT_ID };
		PartitionQuery.Constraint.ComponentConstraint = SpatialConstants::GDK_DEBUG_COMPONENT_ID;
		AddComponentQueryPairToInterestComponent(PartitionInterest, PartitionAuthComponentSet, PartitionQuery);

		PartitionQuery = Query();
		PartitionQuery.ResultComponentIds = { SpatialConstants::GDK_DEBUG_TAG_COMPONENT_ID };
		PartitionQuery.Constraint.ComponentConstraint = SpatialConstants::GDK_DEBUG_TAG_COMPONENT_ID;
		AddComponentQueryPairToInterestComponent(PartitionInterest, PartitionAuthComponentSet, PartitionQuery);
	}

#if WITH_GAMEPLAY_DEBUGGER
	// Query to know about all the actors tagged with a gameplay debugger component
	PartitionQuery = Query();
	PartitionQuery.ResultComponentIds = { SpatialConstants::GDK_GAMEPLAY_DEBUGGER_COMPONENT_ID };
	PartitionQuery.Constraint.ComponentConstraint = SpatialConstants::GDK_GAMEPLAY_DEBUGGER_COMPONENT_ID;
	AddComponentQueryPairToInterestComponent(PartitionInterest, SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, PartitionQuery);
#endif

	return PartitionInterest;
}

Interest InterestFactory::CreateRoutingWorkerInterest()
{
	Interest ServerInterest;
	Query ServerQuery;

	ServerQuery.ResultComponentIds = {
		SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID,
		SpatialConstants::CROSS_SERVER_RECEIVER_ENDPOINT_COMPONENT_ID,
		SpatialConstants::CROSS_SERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID,
		SpatialConstants::CROSS_SERVER_SENDER_ENDPOINT_COMPONENT_ID,
		SpatialConstants::CROSS_SERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID,
	};
	ServerQuery.Constraint.ComponentConstraint = SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID;

	AddComponentQueryPairToInterestComponent(ServerInterest, SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, ServerQuery);

	return ServerInterest;
}

Interest UnrealServerInterestFactory::CreateInterest(AActor* InActor, const FClassInfo& InInfo, const Worker_EntityId InEntityId) const
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	// The interest is built progressively by adding the different component query pairs to build the full map.
	Interest ResultInterest;

	if (InActor->IsA(APlayerController::StaticClass()))
	{
		// Put the "main" client interest queries on the player controller
		AddClientPlayerControllerActorInterest(ResultInterest, InActor, InInfo);
	}

#if WITH_GAMEPLAY_DEBUGGER
	if (AGameplayDebuggerCategoryReplicator* Replicator = Cast<AGameplayDebuggerCategoryReplicator>(InActor))
	{
		// Put special server interest on the replicator for the auth server to ensure player controller visibility
		AddServerGameplayDebuggerCategoryReplicatorActorInterest(ResultInterest, *Replicator);
	}
#endif

	// Clients need to see owner only and server RPC components on entities they have authority over
	AddClientSelfInterest(ResultInterest);

	// Every actor needs a self query for the server to the client RPC endpoint
	AddServerSelfInterest(ResultInterest);

	// Ensure the auth server has interest in an actor's ownership chain
	AddServerActorOwnerInterest(ResultInterest, InActor, InEntityId);

	// Add interest in AlwaysInterested UProperties
	AddAlwaysInterestedInterest(ResultInterest, InActor, InInfo);

	return ResultInterest;
}

void InterestFactory::AddClientPlayerControllerActorInterest(Interest& OutInterest, const AActor* InActor, const FClassInfo& InInfo) const
{
	if (GetDefault<USpatialGDKSettings>()->bUseClientEntityInterestQueries)
	{
		AddClientInterestEntityIdQuery(OutInterest, InActor);
		return;
	}

	const QueryConstraint LevelConstraint = CreateLevelConstraints(InActor);
	AddClientAlwaysRelevantQuery(OutInterest, InActor, InInfo, LevelConstraint);
	AddUserDefinedQueries(OutInterest, InActor, LevelConstraint);

	// Either add the NCD interest because there are no user interest queries, or because the user interest specified we should.
	if (ShouldAddNetCullDistanceInterest(InActor))
	{
		AddNetCullDistanceQueries(OutInterest, LevelConstraint);
	}
}

#if WITH_GAMEPLAY_DEBUGGER
void InterestFactory::AddServerGameplayDebuggerCategoryReplicatorActorInterest(Interest& OutInterest,
																			   const AGameplayDebuggerCategoryReplicator& Replicator) const
{
	APlayerController* PlayerController = Replicator.GetReplicationOwner();
	if (PlayerController == nullptr)
	{
		UE_LOG(LogInterestFactory, Warning,
			   TEXT("Gameplay debugger category replicator actor %s doesn't have a player controller set as its replication owner. An "
					"interest query for the authoritative server could not be added and using the gameplay debugger might fail."),
			   *Replicator.GetName());
		return;
	}

	const UNetDriver* NetDriver = PlayerController->GetNetDriver();
	if (NetDriver == nullptr)
	{
		UE_LOG(LogInterestFactory, Warning,
			   TEXT("Player controller actor %s doesn't have an associated net driver. An interest query for the authoritative server "
					"could not be added and using the gameplay debugger might fail."),
			   *PlayerController->GetName());
		return;
	}

	// Add a query for the authoritative server to see the player controller
	Query PlayerControllerQuery;
	PlayerControllerQuery.Constraint.EntityIdConstraint = NetDriver->GetActorEntityId(*PlayerController);
	PlayerControllerQuery.ResultComponentIds = ServerNonAuthInterestResultType.ComponentIds;
	PlayerControllerQuery.ResultComponentSetIds = ServerNonAuthInterestResultType.ComponentSetsIds;
	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, PlayerControllerQuery);
}
#endif

void InterestFactory::AddClientInterestEntityIdQuery(Interest& OutInterest, const AActor* InActor) const
{
	const APlayerController* PlayerController = Cast<APlayerController>(InActor);
	if (!ensure(GetDefault<USpatialGDKSettings>()->bUseClientEntityInterestQueries)
		|| !ensureMsgf(PlayerController != nullptr, TEXT("Only PCs can use client entity interest: %s"), *GetNameSafe(InActor)))
	{
		return;
	}

	Query RepGraphEntityIdQuery = Query();
	RepGraphEntityIdQuery.ResultComponentIds = ClientNonAuthInterestResultType.ComponentIds;
	RepGraphEntityIdQuery.ResultComponentSetIds = ClientNonAuthInterestResultType.ComponentSetsIds;

	// If and when the Runtime has a better interest changing API, we can ask for a diff here instead.
	TArray<Worker_EntityId> ClientInterestedEntities = GetClientInterestedEntityIds(PlayerController);

	// MAKE DEBUGGING EASIER, REVERT ME
#if WITH_EDITOR
	ClientInterestedEntities.Sort();
#endif

	UMetricsExport* MetricsExport =
		Cast<UMetricsExport>(InActor->GetWorld()->GetGameState()->GetComponentByClass(UMetricsExport::StaticClass()));
	if (MetricsExport != nullptr)
	{
		USpatialNetConnection* NetConnection = Cast<USpatialNetConnection>(PlayerController->NetConnection);
		const FString ClientIdentifier = FString::Printf(TEXT("PC-%lld"), NetConnection->GetPlayerControllerEntityId());
		MetricsExport->WriteMetricsToProtocolBuffer(ClientIdentifier, TEXT("total_interested_entities"), ClientInterestedEntities.Num());
	}

	for (Worker_EntityId EntityId : ClientInterestedEntities)
	{
		QueryConstraint Constraint;
		Constraint.EntityIdConstraint = EntityId;
		RepGraphEntityIdQuery.Constraint.OrConstraint.Add(Constraint);
	}

	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, RepGraphEntityIdQuery);
}

TArray<Worker_EntityId> InterestFactory::GetClientInterestedEntityIds(const APlayerController* InPlayerController) const
{
	TArray<Worker_EntityId> InterestedEntityIdList{};

	UNetConnection* NetConnection = InPlayerController->NetConnection;
	USpatialReplicationGraph* RepGraph = Cast<USpatialReplicationGraph>(NetConnection->Driver->GetReplicationDriver());
	if (RepGraph == nullptr)
	{
		UE_LOG(LogInterestFactory, Error, TEXT("Rep graph was nullptr or not a USpatialReplicationGraph subclass."));
		return TArray<Worker_EntityId>();
	}

	// If and when the Runtime has a better interest changing API, we can ask for a diff here instead.
	TArray<AActor*> ClientInterestedActors = RepGraph->GatherClientInterestedActors(NetConnection);

	UNetReplicationGraphConnection* ConnectionDriver =
		Cast<UNetReplicationGraphConnection>(NetConnection->GetReplicationConnectionDriver());
	UMetricsExport* MetricsExport =
		Cast<UMetricsExport>(InPlayerController->GetWorld()->GetGameState()->GetComponentByClass(UMetricsExport::StaticClass()));
	if (MetricsExport != nullptr)
	{
		USpatialNetConnection* SpatialNetConnection = Cast<USpatialNetConnection>(InPlayerController->NetConnection);
		const FString ClientIdentifier = FString::Printf(TEXT("PC-%lld"), SpatialNetConnection->GetPlayerControllerEntityId());
		MetricsExport->WriteMetricsToProtocolBuffer(ClientIdentifier, TEXT("interested_spatialized_actors"),
													ConnectionDriver->InterestedSpatializedActors.Num());
		MetricsExport->WriteMetricsToProtocolBuffer(ClientIdentifier, TEXT("noninterested_spatialized_actors"),
													ConnectionDriver->NoninterestedSpatializedActors.Num());
	}

	InterestedEntityIdList.Reserve(ClientInterestedActors.Num());

	for (const AActor* Actor : ClientInterestedActors)
	{
		const Worker_EntityId EntityId = PackageMap->GetEntityIdFromObject(Actor);

		// We should be assigning entity IDs to replicated Actors as soon as they are created.
		// The only exception is startup Actors that are authoritative on another server worker.
		// In this case, we need to wait for the other server to replicate this Actor, and the Runtime
		// to send us the entity data.
		if (EntityId == SpatialConstants::INVALID_ENTITY_ID && !(Actor->bNetStartup && !Actor->HasAuthority()))
		{
			UE_LOG(LogInterestFactory, Error, TEXT("Frame %u. Failed getting entity ID for %s when creating client entity interest"),
				   RepGraph->GetReplicationGraphFrame(), *GetNameSafe(Actor));
			continue;
		}

		InterestedEntityIdList.Emplace(EntityId);
	}

	UE_LOG(LogInterestFactory, Verbose, TEXT("Frame %u. Sending %i interest entities for %s"), RepGraph->GetReplicationGraphFrame(),
		   InterestedEntityIdList.Num(), *InPlayerController->GetName());

	return InterestedEntityIdList;
}

void InterestFactory::AddClientSelfInterest(Interest& OutInterest) const
{
	Query NewQuery;
	// Just an entity ID constraint is fine, as clients should not become authoritative over entities outside their loaded levels
	NewQuery.Constraint.bSelfConstraint = true;
	NewQuery.ResultComponentIds = ClientAuthInterestResultType.ComponentIds;
	NewQuery.ResultComponentSetIds = ClientAuthInterestResultType.ComponentSetsIds;

	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, NewQuery);
}

void InterestFactory::AddServerSelfInterest(Interest& OutInterest) const
{
	// Add a query for components all servers need to read client data
	Query ClientQuery;
	ClientQuery.Constraint.bSelfConstraint = true;
	ClientQuery.ResultComponentIds = ServerAuthInterestResultType.ComponentIds;
	ClientQuery.ResultComponentSetIds = ServerAuthInterestResultType.ComponentSetsIds;
	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, ClientQuery);

	// Add a query for the load balancing worker (whoever is delegated the auth delegation component) to read the authority intent
	Query LoadBalanceQuery;
	LoadBalanceQuery.Constraint.bSelfConstraint = true;
	LoadBalanceQuery.ResultComponentIds = { SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID,
											SpatialConstants::AUTHORITY_INTENTV2_COMPONENT_ID,
											SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID,
											SpatialConstants::LB_TAG_COMPONENT_ID };
	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, LoadBalanceQuery);
}

void InterestFactory::AddClientAlwaysRelevantQuery(Interest& OutInterest, const AActor* InActor, const FClassInfo& InInfo,
												   const QueryConstraint& LevelConstraint) const
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	QueryConstraint AlwaysRelevantConstraint = CreateClientAlwaysRelevantConstraint();

	// Add the level constraint here as all client queries need to make sure they don't check out anything outside their loaded levels.
	QueryConstraint AlwaysRelevantAndLevelConstraint;
	AlwaysRelevantAndLevelConstraint.AndConstraint.Add(AlwaysRelevantConstraint);
	AlwaysRelevantAndLevelConstraint.AndConstraint.Add(LevelConstraint);

	Query ClientSystemQuery;
	ClientSystemQuery.Constraint = AlwaysRelevantAndLevelConstraint;
	ClientSystemQuery.ResultComponentIds = ClientNonAuthInterestResultType.ComponentIds;
	ClientSystemQuery.ResultComponentSetIds = ClientNonAuthInterestResultType.ComponentSetsIds;

	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, ClientSystemQuery);
}

void UnrealServerInterestFactory::AddAlwaysInterestedInterest(Interest& OutInterest, const AActor* InActor, const FClassInfo& InInfo) const
{
	QueryConstraint AlwaysInterestedConstraint = CreateAlwaysInterestedConstraint(InActor, InInfo);

	if (AlwaysInterestedConstraint.IsValid())
	{
		{
			const QueryConstraint LevelConstraint = CreateLevelConstraints(InActor);

			QueryConstraint AlwaysInterestAndLevelConstraint;
			AlwaysInterestAndLevelConstraint.AndConstraint.Add(AlwaysInterestedConstraint);
			AlwaysInterestAndLevelConstraint.AndConstraint.Add(LevelConstraint);

			Query ClientAlwaysInterestedQuery;
			ClientAlwaysInterestedQuery.Constraint = AlwaysInterestAndLevelConstraint;
			ClientAlwaysInterestedQuery.ResultComponentIds = ClientNonAuthInterestResultType.ComponentIds;
			ClientAlwaysInterestedQuery.ResultComponentSetIds = ClientNonAuthInterestResultType.ComponentSetsIds;

			AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID,
													 ClientAlwaysInterestedQuery);
		}

		{
			Query ServerAlwaysInterestedQuery;
			ServerAlwaysInterestedQuery.Constraint = AlwaysInterestedConstraint;
			ServerAlwaysInterestedQuery.ResultComponentIds = ServerNonAuthInterestResultType.ComponentIds;
			ServerAlwaysInterestedQuery.ResultComponentSetIds = ServerNonAuthInterestResultType.ComponentSetsIds;

			AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID,
													 ServerAlwaysInterestedQuery);
		}
	}
}

void InterestFactory::AddUserDefinedQueries(Interest& OutInterest, const AActor* InActor, const QueryConstraint& LevelConstraint) const
{
	SCOPE_CYCLE_COUNTER(STAT_InterestFactoryAddUserDefinedQueries);
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	FrequencyToConstraintsMap FrequencyConstraintsMap = GetUserDefinedFrequencyToConstraintsMap(InActor);

	for (const auto& FrequencyToConstraints : FrequencyConstraintsMap)
	{
		Query UserQuery;
		QueryConstraint UserConstraint;

		UserQuery.Frequency = FrequencyToConstraints.Key;

		// If there is only one constraint, don't make the constraint an OR.
		if (FrequencyToConstraints.Value.Num() == 1)
		{
			UserConstraint = FrequencyToConstraints.Value[0];
		}
		else
		{
			UserConstraint.OrConstraint.Append(FrequencyToConstraints.Value);
		}

		if (!UserConstraint.IsValid())
		{
			continue;
		}

		// All constraints have to be limited to the checked out levels, so create an AND constraint with the level.
		UserQuery.Constraint.AndConstraint.Add(UserConstraint);
		UserQuery.Constraint.AndConstraint.Add(LevelConstraint);

		// Make sure that the Entity is not marked as bHidden
		QueryConstraint VisibilityConstraint;
		VisibilityConstraint = CreateActorVisibilityConstraint();
		UserQuery.Constraint.AndConstraint.Add(VisibilityConstraint);

		// We enforce result type even for user defined queries. Here we are assuming what a user wants from their defined
		// queries are for their players to check out more actors than they normally would, so use the client non auth result type,
		// which includes all components required for a client to see non-authoritative actors.
		UserQuery.ResultComponentIds = ClientNonAuthInterestResultType.ComponentIds;
		UserQuery.ResultComponentSetIds = ClientNonAuthInterestResultType.ComponentSetsIds;

		AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, UserQuery);

		// Add the user interest to the server as well if load balancing is enabled and the client queries on server flag is flipped
		// Need to check if load balancing is enabled otherwise there is not chance the client could see and entity the server can't,
		// which is what the client queries on server flag is to avoid.
		if (Settings->bEnableClientQueriesOnServer)
		{
			Query ServerUserQuery;
			ServerUserQuery.Constraint = UserConstraint;
			ServerUserQuery.Frequency = FrequencyToConstraints.Key;
			ServerUserQuery.ResultComponentIds = ServerNonAuthInterestResultType.ComponentIds;
			ServerUserQuery.ResultComponentSetIds = ServerNonAuthInterestResultType.ComponentSetsIds;

			AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, ServerUserQuery);
		}
	}
}

FrequencyToConstraintsMap InterestFactory::GetUserDefinedFrequencyToConstraintsMap(const AActor* InActor) const
{
	// This function builds a frequency to constraint map rather than queries. It does this for two reasons:
	// - We need to set the result type later
	// - The map implicitly removes duplicates queries that have the same constraint. Result types are set for each query and these are
	// large,
	//   so worth simplifying as much as possible.
	FrequencyToConstraintsMap FrequencyToConstraints;

	if (const APlayerController* PlayerController = Cast<APlayerController>(InActor))
	{
		// If this is for a player controller, loop through the pawns of the controller as well, because we only add interest to
		// the player controller entity but interest can be specified on the pawn of the controller as well.
		GetActorUserDefinedQueryConstraints(InActor, FrequencyToConstraints, true);
		GetActorUserDefinedQueryConstraints(PlayerController->GetPawn(), FrequencyToConstraints, true);
	}
	else
	{
		GetActorUserDefinedQueryConstraints(InActor, FrequencyToConstraints, false);
	}

	return FrequencyToConstraints;
}

void InterestFactory::GetActorUserDefinedQueryConstraints(const AActor* InActor, FrequencyToConstraintsMap& OutFrequencyToConstraints,
														  bool bRecurseChildren) const
{
	if (InActor == nullptr)
	{
		return;
	}

	// The defined actor interest component populates the frequency to constraints map with the user defined queries.
	TArray<UActorInterestComponent*> ActorInterestComponents;
	InActor->GetComponents<UActorInterestComponent>(ActorInterestComponents);
	if (ActorInterestComponents.Num() == 1)
	{
		ActorInterestComponents[0]->PopulateFrequencyToConstraintsMap(*ClassInfoManager, OutFrequencyToConstraints);
	}
	else if (ActorInterestComponents.Num() > 1)
	{
		UE_LOG(LogInterestFactory, Error, TEXT("%s has more than one ActorInterestComponent"), *InActor->GetPathName());
		checkNoEntry()
	}

	if (bRecurseChildren)
	{
		for (const auto& Child : InActor->Children)
		{
			GetActorUserDefinedQueryConstraints(Child, OutFrequencyToConstraints, true);
		}
	}
}

void InterestFactory::AddNetCullDistanceQueries(Interest& OutInterest, const QueryConstraint& LevelConstraint) const
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	// The CheckoutConstraints list contains items with a constraint and a frequency.
	// They are then converted to queries by adding a result type to them, and the constraints are conjoined with the level constraint.
	for (const auto& CheckoutRadiusConstraintFrequencyPair : ClientCheckoutRadiusConstraint)
	{
		if (!CheckoutRadiusConstraintFrequencyPair.Constraint.IsValid())
		{
			continue;
		}

		Query NewQuery;

		NewQuery.Constraint.AndConstraint.Add(CheckoutRadiusConstraintFrequencyPair.Constraint);

		if (LevelConstraint.IsValid())
		{
			NewQuery.Constraint.AndConstraint.Add(LevelConstraint);
		}

		// Make sure that the Entity is not marked as bHidden
		QueryConstraint VisibilityConstraint;
		VisibilityConstraint = CreateActorVisibilityConstraint();
		NewQuery.Constraint.AndConstraint.Add(VisibilityConstraint);

		NewQuery.Frequency = CheckoutRadiusConstraintFrequencyPair.Frequency;
		NewQuery.ResultComponentIds = ClientNonAuthInterestResultType.ComponentIds;
		NewQuery.ResultComponentSetIds = ClientNonAuthInterestResultType.ComponentSetsIds;

		AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, NewQuery);

		// Add the queries to the server as well to ensure that all entities checked out on the client will be present on the server.
		if (Settings->bEnableClientQueriesOnServer)
		{
			Query ServerQuery;
			ServerQuery.Constraint = CheckoutRadiusConstraintFrequencyPair.Constraint;
			ServerQuery.Frequency = CheckoutRadiusConstraintFrequencyPair.Frequency;
			ServerQuery.ResultComponentIds = ServerNonAuthInterestResultType.ComponentIds;
			ServerQuery.ResultComponentSetIds = ServerNonAuthInterestResultType.ComponentSetsIds;

			AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, ServerQuery);
		}
	}
}

void InterestFactory::AddComponentQueryPairToInterestComponent(Interest& OutInterest, const Worker_ComponentId ComponentId,
															   const Query& QueryToAdd)
{
	if (!OutInterest.ComponentInterestMap.Contains(ComponentId))
	{
		ComponentSetInterest NewComponentInterest;
		OutInterest.ComponentInterestMap.Add(ComponentId, NewComponentInterest);
	}
	OutInterest.ComponentInterestMap[ComponentId].Queries.Add(QueryToAdd);
}

bool InterestFactory::ShouldAddNetCullDistanceInterest(const AActor* InActor) const
{
	if (GetDefault<USpatialGDKSettings>()->bUseClientEntityInterestQueries)
	{
		return false;
	}

	// If the actor has a component to specify interest and that indicates that we shouldn't add
	// constraints based on NetCullDistanceSquared, abort. There is a check elsewhere to ensure that
	// there is at most one ActorInterestQueryComponent.
	TArray<UActorInterestComponent*> ActorInterestComponents;
	InActor->GetComponents<UActorInterestComponent>(ActorInterestComponents);
	if (ActorInterestComponents.Num() == 1)
	{
		const UActorInterestComponent* ActorInterest = ActorInterestComponents[0];
		if (!ActorInterest->bUseNetCullDistanceSquaredForCheckoutRadius)
		{
			return false;
		}
	}

	return true;
}

QueryConstraint UnrealServerInterestFactory::CreateAlwaysInterestedConstraint(const AActor* InActor, const FClassInfo& InInfo) const
{
	QueryConstraint AlwaysInterestedConstraint;

	for (const FInterestPropertyInfo& PropertyInfo : InInfo.InterestProperties)
	{
		uint8* Data = (uint8*)InActor + PropertyInfo.Offset;
		if (GDK_PROPERTY(ObjectPropertyBase)* ObjectProperty = GDK_CASTFIELD<GDK_PROPERTY(ObjectPropertyBase)>(PropertyInfo.Property))
		{
			AddObjectToConstraint(ObjectProperty, Data, AlwaysInterestedConstraint);
		}
		else if (GDK_PROPERTY(ArrayProperty)* ArrayProperty = GDK_CASTFIELD<GDK_PROPERTY(ArrayProperty)>(PropertyInfo.Property))
		{
			FScriptArrayHelper ArrayHelper(ArrayProperty, Data);
			for (int i = 0; i < ArrayHelper.Num(); i++)
			{
				AddObjectToConstraint(GDK_CASTFIELD<GDK_PROPERTY(ObjectPropertyBase)>(ArrayProperty->Inner), ArrayHelper.GetRawPtr(i),
									  AlwaysInterestedConstraint);
			}
		}
		else
		{
			checkNoEntry();
		}
	}

	return AlwaysInterestedConstraint;
}

QueryConstraint CreateOrConstraint(const TArray<Worker_ComponentId>& ComponentIds)
{
	QueryConstraint ComponentOrConstraint;

	for (Worker_ComponentId ComponentId : ComponentIds)
	{
		QueryConstraint Constraint;
		Constraint.ComponentConstraint = ComponentId;
		ComponentOrConstraint.OrConstraint.Add(Constraint);
	}

	return ComponentOrConstraint;
}

QueryConstraint InterestFactory::CreateGDKSnapshotEntitiesConstraint() const
{
	return CreateOrConstraint({ SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID,
								SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID, SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID });
}

QueryConstraint InterestFactory::CreateClientAlwaysRelevantConstraint() const
{
	return CreateOrConstraint({ SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID });
}

QueryConstraint InterestFactory::CreateServerAlwaysRelevantConstraint() const
{
	return CreateOrConstraint(
		{ SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID, SpatialConstants::SERVER_ONLY_ALWAYS_RELEVANT_COMPONENT_ID });
}

QueryConstraint InterestFactory::CreateActorVisibilityConstraint() const
{
	QueryConstraint ActorVisibilityConstraint;
	ActorVisibilityConstraint.ComponentConstraint = SpatialConstants::VISIBLE_COMPONENT_ID;

	return ActorVisibilityConstraint;
}

QueryConstraint InterestFactory::CreateLevelConstraints(const AActor* InActor) const
{
	QueryConstraint LevelConstraint;

	QueryConstraint DefaultConstraint;
	DefaultConstraint.ComponentConstraint = SpatialConstants::NOT_STREAMED_COMPONENT_ID;
	LevelConstraint.OrConstraint.Add(DefaultConstraint);

	UNetConnection* Connection = InActor->GetNetConnection();
	if (!ensureAlwaysMsgf(Connection != nullptr,
						  TEXT("Failed to create interest level constraints. Couldn't access net connection for Actor %s"),
						  *GetNameSafe(InActor)))
	{
		return QueryConstraint{};
	}

	APlayerController* PlayerController = Connection->GetPlayerController(nullptr);
	if (!ensureAlwaysMsgf(PlayerController != nullptr,
						  TEXT("Failed to create interest level constraints. Couldn't find player controller for Actor %s"),
						  *GetNameSafe(InActor)))
	{
		return QueryConstraint{};
	}

	const TSet<FName>& LoadedLevels = PlayerController->NetConnection->ClientVisibleLevelNames;

	// Create component constraints for every loaded sub-level
	for (const auto& LevelPath : LoadedLevels)
	{
		const Worker_ComponentId ComponentId = ClassInfoManager->GetComponentIdFromLevelPath(LevelPath.ToString());
		if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			QueryConstraint SpecificLevelConstraint;
			SpecificLevelConstraint.ComponentConstraint = ComponentId;
			LevelConstraint.OrConstraint.Add(SpecificLevelConstraint);
		}
		else
		{
			UE_LOG(LogInterestFactory, Error,
				   TEXT("Error creating query constraints for Actor %s. "
						"Could not find Streaming Level Component for Level %s. Have you generated schema?"),
				   *InActor->GetName(), *LevelPath.ToString());
		}
	}

	return LevelConstraint;
}

UnrealServerInterestFactory::UnrealServerInterestFactory(USpatialClassInfoManager* InClassInfoManager,
														 USpatialPackageMapClient* InPackageMap)
	: InterestFactory(InClassInfoManager)
	, PackageMap(InPackageMap)
{
}

void UnrealServerInterestFactory::AddObjectToConstraint(GDK_PROPERTY(ObjectPropertyBase) * Property, uint8* Data,
														QueryConstraint& OutConstraint) const
{
	UObject* ObjectOfInterest = Property->GetObjectPropertyValue(Data);

	if (ObjectOfInterest == nullptr)
	{
		return;
	}

	FUnrealObjectRef UnrealObjectRef = PackageMap->GetUnrealObjectRefFromObject(ObjectOfInterest);

	if (!UnrealObjectRef.IsValid())
	{
		return;
	}

	QueryConstraint EntityIdConstraint;
	EntityIdConstraint.EntityIdConstraint = UnrealObjectRef.Entity;
	OutConstraint.OrConstraint.Add(EntityIdConstraint);
}

bool UnrealServerInterestFactory::DoOwnersHaveEntityId(const AActor* Actor) const
{
	AActor* Owner = Actor->GetOwner();

	while (Owner != nullptr && !Owner->IsPendingKillPending() && Owner->GetIsReplicated())
	{
		if (PackageMap->GetEntityIdFromObject(Owner) == SpatialConstants::INVALID_ENTITY_ID)
		{
			return false;
		}
		Owner = Owner->GetOwner();
	}

	return true;
}

void UnrealServerInterestFactory::AddServerActorOwnerInterest(Interest& OutInterest, const AActor* InActor,
															  const Worker_EntityId& EntityId) const
{
	AActor* Owner = InActor->GetOwner();
	Query OwnerChainQuery;

	while (Owner != nullptr && !Owner->IsPendingKillPending() && Owner->GetIsReplicated())
	{
		QueryConstraint OwnerQuery;
		OwnerQuery.EntityIdConstraint = PackageMap->GetEntityIdFromObject(Owner);
		if (OwnerQuery.EntityIdConstraint == SpatialConstants::INVALID_ENTITY_ID)
		{
			UE_LOG(LogInterestFactory, Warning,
				   TEXT("Interest for Actor %s (%llu) is out of date because owner %s does not have an entity id."
						"USpatialActorChannel::NeedOwnerInterestUpdate should be set in order to eventually update it"),
				   *InActor->GetName(), EntityId, *Owner->GetName());
			return;
		}
		OwnerChainQuery.Constraint.OrConstraint.Add(OwnerQuery);
		Owner = Owner->GetOwner();
	}

	if (OwnerChainQuery.Constraint.OrConstraint.Num() != 0)
	{
		OwnerChainQuery.ResultComponentIds = ServerNonAuthInterestResultType.ComponentIds;
		OwnerChainQuery.ResultComponentSetIds = ServerNonAuthInterestResultType.ComponentSetsIds;
		AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, OwnerChainQuery);
	}
}

} // namespace SpatialGDK
