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
#include "Utils/SpatialActorUtils.h"

#include "Engine/World.h"
#include "Engine/Classes/GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY(LogInterestFactory);

DECLARE_STATS_GROUP(TEXT("InterestFactory"), STATGROUP_SpatialInterestFactory, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("AddUserDefinedQueries"), STAT_InterestFactoryAddUserDefinedQueries, STATGROUP_SpatialInterestFactory);

namespace SpatialGDK
{

InterestFactory::InterestFactory(USpatialClassInfoManager* InClassInfoManager, USpatialPackageMapClient* InPackageMap)
	: ClassInfoManager(InClassInfoManager)
	, PackageMap(InPackageMap)
{
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
	SchemaResultType ClientNonAuthResultType;

	// Add the required unreal components
	ClientNonAuthResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST);

	// Add all data components- clients don't need to see handover or owner only components on other entities.
	ClientNonAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Data));

	return ClientNonAuthResultType;
}

SchemaResultType InterestFactory::CreateClientAuthInterestResultType()
{
	SchemaResultType ClientAuthResultType;

	// Add the required known components
	ClientAuthResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_AUTH_CLIENT_INTEREST);
	ClientAuthResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_CLIENT_INTEREST);

	// Add all the generated unreal components
	ClientAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Data));
	ClientAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_OwnerOnly));

	return ClientAuthResultType;
}

SchemaResultType InterestFactory::CreateServerNonAuthInterestResultType()
{
	SchemaResultType ServerNonAuthResultType;

	// Add the required unreal components
	ServerNonAuthResultType.Append(SpatialConstants::REQUIRED_COMPONENTS_FOR_NON_AUTH_SERVER_INTEREST);

	// Add all data, owner only, and handover components
	ServerNonAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Data));
	ServerNonAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_OwnerOnly));
	ServerNonAuthResultType.Append(ClassInfoManager->GetComponentIdsForComponentType(ESchemaComponentType::SCHEMA_Handover));

	return ServerNonAuthResultType;
}

SchemaResultType InterestFactory::CreateServerAuthInterestResultType()
{
	// Just the components that we won't have already checked out through authority
	return SpatialConstants::REQUIRED_COMPONENTS_FOR_AUTH_SERVER_INTEREST;
}

Worker_ComponentData InterestFactory::CreateInterestData(AActor* InActor, const FClassInfo& InInfo, const Worker_EntityId InEntityId, bool& bOutOwnerReady) const
{
	return CreateInterest(InActor, InInfo, InEntityId, bOutOwnerReady).CreateInterestData();
}

Worker_ComponentUpdate InterestFactory::CreateInterestUpdate(AActor* InActor, const FClassInfo& InInfo, const Worker_EntityId InEntityId, bool& bOutOwnerReady) const
{
	return CreateInterest(InActor, InInfo, InEntityId, bOutOwnerReady).CreateInterestUpdate();
}

Interest InterestFactory::CreateServerWorkerInterest(const UAbstractLBStrategy* LBStrategy)
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	// Build the Interest component as we go by updating the component-> query list mappings.
	Interest ServerInterest;
	ComponentInterest ServerComponentInterest;
	Query ServerQuery;
	QueryConstraint Constraint;

	// Set the result type of the query
	ServerQuery.ResultComponentIds = ServerNonAuthInterestResultType;

	// Ensure server worker receives always relevant entities
	QueryConstraint AlwaysRelevantConstraint = CreateAlwaysRelevantConstraint();

	Constraint = AlwaysRelevantConstraint;

	// Also add the server worker interest defined by the load balancing strategy if there is more than one worker.
	if (LBStrategy->GetMinimumRequiredWorkers() > 1)
	{
		check(LBStrategy != nullptr);

		// The load balancer won't be ready when the worker initially connects to SpatialOS. It needs
		// to wait for the virtual worker mappings to be replicated.
		// This function will be called again when that is the case in order to update the interest on the server entity.
		if (LBStrategy->IsReady())
		{
			QueryConstraint LoadBalancerConstraint = LBStrategy->GetWorkerInterestQueryConstraint();

			// Rather than adding the load balancer constraint at the end, reorder the constraints to have the large spatial
			// constraint at the front. This is more likely to be efficient.
			QueryConstraint NewConstraint;
			NewConstraint.OrConstraint.Add(LoadBalancerConstraint);
			NewConstraint.OrConstraint.Add(AlwaysRelevantConstraint);
			Constraint = NewConstraint;
		}
	}

	ServerQuery.Constraint = Constraint;
	AddComponentQueryPairToInterestComponent(ServerInterest, SpatialConstants::POSITION_COMPONENT_ID, ServerQuery);

	// Add another query to get the worker system entities.
	// It allows us to know when a client has disconnected.
	// TODO UNR-3042 : Migrate the VirtualWorkerTranslationManager to use the checked-out worker components instead of making a query.

	ServerQuery = Query();
	ServerQuery.ResultComponentIds = SchemaResultType{ SpatialConstants::WORKER_COMPONENT_ID };
	ServerQuery.Constraint.ComponentConstraint = SpatialConstants::WORKER_COMPONENT_ID;
	AddComponentQueryPairToInterestComponent(ServerInterest, SpatialConstants::POSITION_COMPONENT_ID, ServerQuery);

	return ServerInterest;
}

Interest InterestFactory::CreateInterest(AActor* InActor, const FClassInfo& InInfo, const Worker_EntityId InEntityId, bool& bOutOwnerReady) const
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	// The interest is built progressively by adding the different component query pairs to build the full map.
	Interest ResultInterest;

	if (InActor->IsA(APlayerController::StaticClass()))
	{
		// Put the "main" interest queries on the player controller
		AddPlayerControllerActorInterest(ResultInterest, InActor, InInfo);
	}

	// Clients need to see owner only and server RPC components on entities they have authority over
	AddClientSelfInterest(ResultInterest, InEntityId);

	// Every actor needs a self query for the server to the client RPC endpoint
	AddServerSelfInterest(ResultInterest, InEntityId);

	bOutOwnerReady = AddServerOwnerInterest(ResultInterest, InActor);

	return ResultInterest;
}

void InterestFactory::AddPlayerControllerActorInterest(Interest& OutInterest, const AActor* InActor, const FClassInfo& InInfo) const
{
	QueryConstraint LevelConstraint = CreateLevelConstraints(InActor);

	AddAlwaysRelevantAndInterestedQuery(OutInterest, InActor, InInfo, LevelConstraint);

	AddUserDefinedQueries(OutInterest, InActor, LevelConstraint);

	// Either add the NCD interest because there are no user interest queries, or because the user interest specified we should.
	if (ShouldAddNetCullDistanceInterest(InActor))
	{
		AddNetCullDistanceQueries(OutInterest, LevelConstraint);
	}
}

void InterestFactory::AddClientSelfInterest(Interest& OutInterest, const Worker_EntityId& EntityId) const
{
	Query NewQuery;
	// Just an entity ID constraint is fine, as clients should not become authoritative over entities outside their loaded levels
	NewQuery.Constraint.EntityIdConstraint = EntityId;
	NewQuery.ResultComponentIds = ClientAuthInterestResultType;

	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::GetClientAuthorityComponent(GetDefault<USpatialGDKSettings>()->UseRPCRingBuffer()), NewQuery);
}

void InterestFactory::AddServerSelfInterest(Interest& OutInterest, const Worker_EntityId& EntityId) const
{
	// Add a query for components all servers need to read client data
	Query ClientQuery;
	ClientQuery.Constraint.EntityIdConstraint = EntityId;
	// Temp fix for invalid initial auth server checkout constraints - UNR-3683
	// Using full snapshot ensures all components are available on checkout. Remove when root issue is resolved.
	ClientQuery.FullSnapshotResult = true;
	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::POSITION_COMPONENT_ID, ClientQuery);

	// Add a query for the load balancing worker (whoever is delegated the ACL) to read the authority intent
	Query LoadBalanceQuery;
	LoadBalanceQuery.Constraint.EntityIdConstraint = EntityId;
	LoadBalanceQuery.ResultComponentIds = SchemaResultType{ SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID, SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID };
	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::ENTITY_ACL_COMPONENT_ID, LoadBalanceQuery);
}

bool InterestFactory::AddServerOwnerInterest(Interest& OutInterest, const AActor* InActor) const
{
	AActor* NetOwner = SpatialGDK::GetHierarchyRoot(InActor);
	if (NetOwner == nullptr || !NetOwner->GetIsReplicated())
	{
		return true;
	}

	Worker_EntityId OwnerId = PackageMap->GetEntityIdFromObject(NetOwner);
	if (OwnerId == SpatialConstants::INVALID_ENTITY_ID)
	{
		return false;
	}

	Query OwnerQuery;
	OwnerQuery.Constraint.EntityIdConstraint = OwnerId;
	OwnerQuery.FullSnapshotResult = true;

	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::POSITION_COMPONENT_ID, OwnerQuery);

	return true;
}

void InterestFactory::AddAlwaysRelevantAndInterestedQuery(Interest& OutInterest, const AActor* InActor, const FClassInfo& InInfo, const QueryConstraint& LevelConstraint) const
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	QueryConstraint AlwaysInterestedConstraint = CreateAlwaysInterestedConstraint(InActor, InInfo);
	QueryConstraint AlwaysRelevantConstraint = CreateAlwaysRelevantConstraint();

	QueryConstraint SystemDefinedConstraints;

	if (AlwaysInterestedConstraint.IsValid())
	{
		SystemDefinedConstraints.OrConstraint.Add(AlwaysInterestedConstraint);
	}

	if (AlwaysRelevantConstraint.IsValid())
	{
		SystemDefinedConstraints.OrConstraint.Add(AlwaysRelevantConstraint);
	}

	// Add the level constraint here as all client queries need to make sure they don't check out anything outside their loaded levels.
	QueryConstraint SystemAndLevelConstraint;
	SystemAndLevelConstraint.AndConstraint.Add(SystemDefinedConstraints);
	SystemAndLevelConstraint.AndConstraint.Add(LevelConstraint);

	Query ClientSystemQuery;
	ClientSystemQuery.Constraint = SystemAndLevelConstraint;
	ClientSystemQuery.ResultComponentIds = ClientNonAuthInterestResultType;

	AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::GetClientAuthorityComponent(Settings->UseRPCRingBuffer()), ClientSystemQuery);

	// Add always interested constraint to the server as well to make sure the server sees the same as the client.
	// The always relevant constraint is added as part of the server worker query, so leave that out here.
	// Servers also don't need to be level constrained.
	if (Settings->bEnableClientQueriesOnServer)
	{
		Query ServerSystemQuery;
		QueryConstraint ServerSystemConstraint;
		ServerSystemConstraint.OrConstraint.Add(AlwaysInterestedConstraint);
		ServerSystemQuery.Constraint = ServerSystemConstraint;
		ServerSystemQuery.ResultComponentIds = ServerNonAuthInterestResultType;

		AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::POSITION_COMPONENT_ID, ServerSystemQuery);
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

		// We enforce result type even for user defined queries. Here we are assuming what a user wants from their defined
		// queries are for their players to check out more actors than they normally would, so use the client non auth result type,
		// which includes all components required for a client to see non-authoritative actors.
		UserQuery.ResultComponentIds = ClientNonAuthInterestResultType;

		AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::GetClientAuthorityComponent(Settings->UseRPCRingBuffer()), UserQuery);

		// Add the user interest to the server as well if load balancing is enabled and the client queries on server flag is flipped
		// Need to check if load balancing is enabled otherwise there is not chance the client could see and entity the server can't,
		// which is what the client queries on server flag is to avoid.
		if (Settings->bEnableClientQueriesOnServer)
		{
			Query ServerUserQuery;
			ServerUserQuery.Constraint = UserConstraint;
			ServerUserQuery.Frequency = FrequencyToConstraints.Key;
			ServerUserQuery.ResultComponentIds = ServerNonAuthInterestResultType;

			AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::POSITION_COMPONENT_ID, ServerUserQuery);
		}
	}
}

FrequencyToConstraintsMap InterestFactory::GetUserDefinedFrequencyToConstraintsMap(const AActor* InActor) const
{
	// This function builds a frequency to constraint map rather than queries. It does this for two reasons:
	// - We need to set the result type later
	// - The map implicitly removes duplicates queries that have the same constraint. Result types are set for each query and these are large,
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

void InterestFactory::GetActorUserDefinedQueryConstraints(const AActor* InActor, FrequencyToConstraintsMap& OutFrequencyToConstraints, bool bRecurseChildren) const
{
	check(ClassInfoManager);

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

		NewQuery.Frequency = CheckoutRadiusConstraintFrequencyPair.Frequency;
		NewQuery.ResultComponentIds = ClientNonAuthInterestResultType;

		AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::GetClientAuthorityComponent(Settings->UseRPCRingBuffer()), NewQuery);

		// Add the queries to the server as well to ensure that all entities checked out on the client will be present on the server.
		if (Settings->bEnableClientQueriesOnServer)
		{
			Query ServerQuery;
			ServerQuery.Constraint = CheckoutRadiusConstraintFrequencyPair.Constraint;
			ServerQuery.Frequency = CheckoutRadiusConstraintFrequencyPair.Frequency;
			ServerQuery.ResultComponentIds = ServerNonAuthInterestResultType;

			AddComponentQueryPairToInterestComponent(OutInterest, SpatialConstants::POSITION_COMPONENT_ID, ServerQuery);
		}
	}
}

void InterestFactory::AddComponentQueryPairToInterestComponent(Interest& OutInterest, const Worker_ComponentId ComponentId, const Query& QueryToAdd) const
{
	if (!OutInterest.ComponentInterestMap.Contains(ComponentId))
	{
		ComponentInterest NewComponentInterest;
		OutInterest.ComponentInterestMap.Add(ComponentId, NewComponentInterest);
	}
	OutInterest.ComponentInterestMap[ComponentId].Queries.Add(QueryToAdd);
}

bool InterestFactory::ShouldAddNetCullDistanceInterest(const AActor* InActor) const
{
	// If the actor has a component to specify interest and that indicates that we shouldn't add
	// constraints based on NetCullDistanceSquared, abort. There is a check elsewhere to ensure that
	// there is at most one ActorInterestQueryComponent.
	TArray<UActorInterestComponent*> ActorInterestComponents;
	InActor->GetComponents<UActorInterestComponent>(ActorInterestComponents);
	if (ActorInterestComponents.Num() == 1)
	{
		const UActorInterestComponent* ActorInterest = ActorInterestComponents[0];
		check(ActorInterest);
		if (!ActorInterest->bUseNetCullDistanceSquaredForCheckoutRadius)
		{
			return false;
		}
	}

	return true;
}

QueryConstraint InterestFactory::CreateAlwaysInterestedConstraint(const AActor* InActor, const FClassInfo& InInfo) const
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
				AddObjectToConstraint(GDK_CASTFIELD<GDK_PROPERTY(ObjectPropertyBase)>(ArrayProperty->Inner), ArrayHelper.GetRawPtr(i), AlwaysInterestedConstraint);
			}
		}
		else
		{
			checkNoEntry();
		}
	}

	return AlwaysInterestedConstraint;
}

QueryConstraint InterestFactory::CreateAlwaysRelevantConstraint() const
{
	QueryConstraint AlwaysRelevantConstraint;

	Worker_ComponentId ComponentIds[] = {
		SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID,
		SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID,
		SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID
	};

	for (Worker_ComponentId ComponentId : ComponentIds)
	{
		QueryConstraint Constraint;
		Constraint.ComponentConstraint = ComponentId;
		AlwaysRelevantConstraint.OrConstraint.Add(Constraint);
	}

	return AlwaysRelevantConstraint;
}

QueryConstraint InterestFactory::CreateLevelConstraints(const AActor* InActor) const
{
	QueryConstraint LevelConstraint;

	QueryConstraint DefaultConstraint;
	DefaultConstraint.ComponentConstraint = SpatialConstants::NOT_STREAMED_COMPONENT_ID;
	LevelConstraint.OrConstraint.Add(DefaultConstraint);

	UNetConnection* Connection = InActor->GetNetConnection();
	check(Connection);
	APlayerController* PlayerController = Connection->GetPlayerController(nullptr);
	check(PlayerController);

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
			UE_LOG(LogInterestFactory, Error, TEXT("Error creating query constraints for Actor %s. "
				"Could not find Streaming Level Component for Level %s. Have you generated schema?"), *InActor->GetName(), *LevelPath.ToString());
		}
	}

	return LevelConstraint;
}

void InterestFactory::AddObjectToConstraint(GDK_PROPERTY(ObjectPropertyBase)* Property, uint8* Data, QueryConstraint& OutConstraint) const
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

} // namespace SpatialGDK
