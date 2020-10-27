// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialReplicationGraph.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialReplicationGraphLoadBalancingHandler.h"
#include "Utils/SpatialActorUtils.h"

void USpatialReplicationGraph::InitForNetDriver(UNetDriver* InNetDriver)
{
	UReplicationGraph::InitForNetDriver(InNetDriver);

	if (USpatialStatics::IsMultiWorkerEnabled())
	{
		if (USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(InNetDriver))
		{
			LoadBalancingHandler = MakeUnique<FSpatialLoadBalancingHandler>(SpatialNetDriver);
		}
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

	if (OldOwner != nullptr)
	{
		GlobalActorReplicationInfoMap.RemoveDependentActor(OldOwner, Actor);
	}

	if (NewOwner != nullptr)
	{
		GlobalActorReplicationInfoMap.Get(NewOwner);
		GlobalActorReplicationInfoMap.Get(Actor);
		GlobalActorReplicationInfoMap.AddDependentActor(NewOwner, Actor);
	}
}

void USpatialReplicationGraph::PreReplicateActors(UNetReplicationGraphConnection* ConnectionManager)
{
	if (LoadBalancingHandler.IsValid())
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(NetDriver);
		FSpatialReplicationGraphLoadBalancingContext LoadBalancingCtx(SpatialNetDriver, this, ConnectionManager->ActorInfoMap,
																	  PrioritizedReplicationList);
		LoadBalancingHandler->EvaluateActorsToMigrate(LoadBalancingCtx);

		for (AActor* Actor : LoadBalancingCtx.AdditionalActorsToReplicate)
		{
			// Only add net owners to the list as they will visit their dependents when replicated.
			AActor* NetOwner = SpatialGDK::GetReplicatedHierarchyRoot(Actor);
			if (NetOwner == Actor)
			{
				FConnectionReplicationActorInfo& ConnectionData = ConnectionManager->ActorInfoMap.FindOrAdd(Actor);
				FGlobalActorReplicationInfo& GlobalData = GlobalActorReplicationInfoMap.Get(Actor);
				PrioritizedReplicationList.Items.Emplace(FPrioritizedRepList::FItem(0, Actor, &GlobalData, &ConnectionData));
			}
		}
	}
}

void USpatialReplicationGraph::PostReplicateActors(UNetReplicationGraphConnection* ConnectionManager)
{
	if (LoadBalancingHandler.IsValid())
	{
		LoadBalancingHandler->ProcessMigrations();
	}
}
