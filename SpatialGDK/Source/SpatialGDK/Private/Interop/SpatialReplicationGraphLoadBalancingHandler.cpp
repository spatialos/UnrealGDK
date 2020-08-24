// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialReplicationGraphLoadBalancingHandler.h"
#include "EngineClasses/SpatialReplicationGraph.h"

FSpatialReplicationGraphLoadBalancingContext::FSpatialReplicationGraphLoadBalancingContext(USpatialNetDriver* InNetDriver,
																						   USpatialReplicationGraph* InReplicationGraph,
																						   FPrioritizedRepList& InRepList)
	: NetDriver(InNetDriver)
	, ReplicationGraph(InReplicationGraph)
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

FActorRepListRefView FSpatialReplicationGraphLoadBalancingContext::GetDependentActors(AActor* Actor)
{
	static FActorRepListRefView s_EmptyList = [] {
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
	return s_EmptyList;
}

bool FSpatialReplicationGraphLoadBalancingContext::IsActorReadyForMigration(AActor* Actor)
{
	// Auth check.
	if (!Actor->HasAuthority())
	{
		return false;
	}

	if (!Actor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_SpatialType))
	{
		return false;
	}

	// Additional check that the actor is seen by the spatial runtime.
	Worker_EntityId EntityId = NetDriver->PackageMap->GetEntityIdFromObject(Actor);
	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		return false;
	}

	if (!NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::POSITION_COMPONENT_ID))
	{
		return false;
	}

	return true;
}
