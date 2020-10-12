// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialReplicationGraphLoadBalancingHandler.h"

#include "EngineClasses/SpatialReplicationGraph.h"

FSpatialReplicationGraphLoadBalancingContext::FSpatialReplicationGraphLoadBalancingContext(USpatialNetDriver* InNetDriver,
																						   USpatialReplicationGraph* InReplicationGraph,
																						   FPerConnectionActorInfoMap& InInfoMap,
																						   FPrioritizedRepList& InRepList)
	: NetDriver(InNetDriver)
	, ReplicationGraph(InReplicationGraph)
	, InfoMap(InInfoMap)
	, ActorsToReplicate(InRepList)
{
}

FSpatialReplicationGraphLoadBalancingContext::FRepListArrayAdaptor FSpatialReplicationGraphLoadBalancingContext::GetActorsBeingReplicated()
{
	return FRepListArrayAdaptor(ActorsToReplicate);
}

void FSpatialReplicationGraphLoadBalancingContext::RemoveAdditionalActor(AActor* Actor)
{
	AdditionalActorsToReplicate.Remove(Actor);
}

void FSpatialReplicationGraphLoadBalancingContext::AddActorToReplicate(AActor* Actor)
{
	ReplicationGraph->ForceNetUpdate(Actor);
	AdditionalActorsToReplicate.Add(Actor);
}

#if ENGINE_MINOR_VERSION > 25
FGlobalActorReplicationInfo::FDependantListType FSpatialReplicationGraphLoadBalancingContext::GetDependentActors(AActor* Actor)
{
	static FGlobalActorReplicationInfo::FDependantListType EmptyList = [] {
		FGlobalActorReplicationInfo::FDependantListType List;
		List.Reset(0);
		return List;
	}();

	if (FGlobalActorReplicationInfo* GlobalActorInfo = ReplicationGraph->GetGlobalActorReplicationInfoMap().Find(Actor))
	{
		return GlobalActorInfo->GetDependentActorList();
	}
	return EmptyList;
}
#else
FActorRepListRefView FSpatialReplicationGraphLoadBalancingContext::GetDependentActors(AActor* Actor)
{
	static FActorRepListRefView EmptyList = [] {
		FActorRepListRefView List;
		List.Reset(0);
		return List;
	}();

	if (FGlobalActorReplicationInfo* GlobalActorInfo = ReplicationGraph->GetGlobalActorReplicationInfoMap().Find(Actor))
	{
		const FActorRepListRefView& DependentActorList = GlobalActorInfo->GetDependentActorList();
		if (DependentActorList.IsValid())
		{
			return DependentActorList;
		}
	}
	return EmptyList;
}
#endif

EActorMigrationResult FSpatialReplicationGraphLoadBalancingContext::IsActorReadyForMigration(AActor* Actor)
{
	if (!Actor->HasAuthority())
	{
		return EActorMigrationResult::NotAuthoritative;
	}

	if (!Actor->IsActorReady())
	{
		return EActorMigrationResult::NotReady;
	}

	// The following checks are extracted from UReplicationGraph::ReplicateActorListsForConnections_Default
	// More accurately, from the loop with the section named NET_ReplicateActors_PrioritizeForConnection
	// The part called "Distance Scaling" is ignored, since it is SpatialOS's job.

	if (!Actor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_SpatialType))
	{
		return EActorMigrationResult::NoSpatialClassFlags;
	}

	FConnectionReplicationActorInfo& ConnectionData = InfoMap.FindOrAdd(Actor);
	if (ConnectionData.bDormantOnConnection)
	{
		return EActorMigrationResult::DormantOnConnection;
	}

	return EActorMigrationResult::Success;
}
