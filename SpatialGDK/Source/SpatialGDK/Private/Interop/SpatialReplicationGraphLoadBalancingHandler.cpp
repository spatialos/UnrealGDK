// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialReplicationGraphLoadBalancingHandler.h"
#include "EngineClasses/SpatialReplicationGraph.h"

FSpatialReplicationGraphLoadBalancingHandler::FSpatialReplicationGraphLoadBalancingHandler(USpatialNetDriver* InNetDriver, USpatialReplicationGraph* InReplicationGraph, FPrioritizedRepList& InRepList)
	: TSpatialLoadBalancingHandler<FSpatialReplicationGraphLoadBalancingHandler>(InNetDriver)
	, ReplicationGraph(InReplicationGraph)
	, ActorsToReplicate(InRepList)
{

}


FSpatialReplicationGraphLoadBalancingHandler::FRepListArrayAdaptor FSpatialReplicationGraphLoadBalancingHandler::GetActorsBeingReplicated()
{
	return FRepListArrayAdaptor(ActorsToReplicate);
}

void FSpatialReplicationGraphLoadBalancingHandler::RemoveAdditionalActor(AActor* Actor)
{

}

void FSpatialReplicationGraphLoadBalancingHandler::AddActorToReplicate(AActor* Actor)
{
	ReplicationGraph->ForceNetUpdate(Actor);
}

FActorRepListRefView FSpatialReplicationGraphLoadBalancingHandler::GetDependentActors(AActor* Actor)
{
	if (FGlobalActorReplicationInfo* GlobalActorInfo = ReplicationGraph->GetGlobalActorReplicationInfoMap().Find(Actor))
	{
		const FActorRepListRefView& DependentActorList = GlobalActorInfo->GetDependentActorList();
		if (DependentActorList.IsValid())
		{
			return DependentActorList;
		}
	}
	return FActorRepListRefView();
}
