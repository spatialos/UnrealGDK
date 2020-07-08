// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialReplicationGraph.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialReplicationGraphLoadBalancingHandler.h"
#include "Interop/SpatialSender.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/AbstractLockingPolicy.h"
#include "Schema/SpatialDebugging.h"
#include "Utils/SpatialActorUtils.h"
#include "Utils/SpatialLoadBalancingHandler.h"

USpatialReplicationGraph::~USpatialReplicationGraph()
{
	if(LoadBalancingHandler)
	{
		delete LoadBalancingHandler;
	}
}

UActorChannel* USpatialReplicationGraph::GetOrCreateSpatialActorChannel(UObject* TargetObject)
{
	if (TargetObject != nullptr)
	{
		if (USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(NetDriver))
		{
			return SpatialNetDriver->GetOrCreateSpatialActorChannel(TargetObject);
		}

		checkNoEntry();
	}

	return nullptr;
}

void USpatialReplicationGraph::OnOwnerUpdated(AActor* Actor, AActor* OldOwner)
{
	AActor* NewOwner = Actor->GetOwner();

	if (OldOwner == NewOwner)
	{
		return;
	}

	if(OldOwner != nullptr)
	{
		GlobalActorReplicationInfoMap.RemoveDependentActor(OldOwner, Actor);
	}

	if(NewOwner != nullptr)
	{
		GlobalActorReplicationInfoMap.AddDependentActor(NewOwner, Actor);
	}
	
}

void USpatialReplicationGraph::PreReplicateActors(UNetReplicationGraphConnection& ConnectionManager)
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(NetDriver);

	if(LoadBalancingHandler == nullptr)
	{
		LoadBalancingHandler = new FSpatialReplicationGraphLoadBalancingHandler(SpatialNetDriver, this, PrioritizedReplicationList);
	}

	LoadBalancingHandler->HandleLoadBalancing();
}

void USpatialReplicationGraph::PostReplicateActors(UNetReplicationGraphConnection& ConnectionManager)
{
	LoadBalancingHandler->ProcessMigrations();
}
