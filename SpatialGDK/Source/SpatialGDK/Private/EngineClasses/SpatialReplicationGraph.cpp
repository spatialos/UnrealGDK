// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialReplicationGraph.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialReplicationGraphLoadBalancingHandler.h"
#include "Utils/SpatialActorUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialReplicationGraph);

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

	bUseNarrowPhaseNCDInterestCulling = GetDefault<USpatialGDKSettings>()->bUseNarrowPhaseNCDInterestCulling;
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

// IMPROBABLE-BEGIN - Client entity list interest
// To get a list of Actors relevant to a client, we iterate global and connection nodes.
// This functionality lives outside the normal rep graph calls because we call a different function on rep graph nodes
// so don't share benefit of iterating at the same time.
// Rep graph nodes need to implement a new GetClientInterestedActors function to return Actors a client is interested in.
// We don't use the GatherReplicatedActorList function for this job because that applies things like rate limiting
// through returning an Actor on alternating calls. For interest, we want to get a conservative list of Actors that
// we expect that node would replicate over multiple subsequent replication calls (at least until next interest change).
TArray<AActor*> USpatialReplicationGraph::GatherClientInterestedActors(UNetConnection* NetConnection)
{
	QUICK_SCOPE_CYCLE_COUNTER(NET_ClientEntityInterest__GatherClientInterestedActors);

	UNetReplicationGraphConnection* ConnectionManager =
		Cast<UNetReplicationGraphConnection>(NetConnection->GetReplicationConnectionDriver());
	if (ConnectionManager == nullptr)
	{
		UE_LOG(
			LogSpatialReplicationGraph, Error,
			TEXT("Replication connection driver was null when trying to calculate client interest, did we call this before replication?"))
		return TArray<AActor*>{};
	}

	FNetViewerArray Viewer;
	Viewer.Emplace(NetConnection, 0);
	TSet<FName> AllClientVisibleLevelNames;
	ConnectionManager->GetClientVisibleLevelNames(AllClientVisibleLevelNames);

	// Iterate across all global and connection relevant nodes and gather a (broadphase) list of client interested Actors
	// Ideally we do interest list caching per node, and only gather new lists from nodes where interest was marked dirty.
	FGatheredReplicationActorLists GatheredReplicationListsForConnection;
	{
		QUICK_SCOPE_CYCLE_COUNTER(NET_ClientEntityInterest_GatherRepListsFromNodes);

		const FConnectionGatherActorListParameters ClientActorInterestParameters(Viewer, *ConnectionManager, AllClientVisibleLevelNames,
																				 GetReplicationGraphFrame(),
																				 GatheredReplicationListsForConnection, true);
		for (UReplicationGraphNode* Node : GlobalGraphNodes)
		{
			Node->GatherClientInterestedActors(ClientActorInterestParameters);
		}

		for (UReplicationGraphNode* Node : ConnectionManager->GetConnectionGraphNodes())
		{
			Node->GatherClientInterestedActors(ClientActorInterestParameters);
		}
	}

	// Parse gathered rep lists into a list of Actors to return to the InterestFactory.
	// - remove duplicate Actors from consideration
	// - ignore our non-spatial and dormant Actors
	// - add dependent Actors
	TArray<AActor*> ClientInterestedActors =
		ExtractClientInterestActorsFromGather(ConnectionManager, GatheredReplicationListsForConnection, Viewer);

	return ClientInterestedActors;
}

TArray<AActor*> USpatialReplicationGraph::ExtractClientInterestActorsFromGather(
	UNetReplicationGraphConnection* ConnectionManager, FGatheredReplicationActorLists& GatheredReplicationListsForConnection,
	FNetViewerArray& Viewers)
{
	QUICK_SCOPE_CYCLE_COUNTER(NET_ClientEntityInterest_ExtractFromGatheredLists);

	TArray<AActor*> ClientInterestedActors{};

	FPerConnectionActorInfoMap& ConnectionActorInfoMap = ConnectionManager->ActorInfoMap;
	const uint32 FrameNum = GetReplicationGraphFrame();

	TArray<AActor*> SortingArray{};
	// Should try and reserve accurately here - could sum gathered rep list lengths (although this wouldn't factor dependent Actors) or
	// cache the last list size

	for (FActorRepListRawView& List : GatheredReplicationListsForConnection.GetLists(EActorRepListTypeFlags::Default))
	{
		for (AActor* Actor : List)
		{
			if (!Actor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_SpatialType))
			{
				UE_LOG(LogSpatialReplicationGraph, Error,
					   TEXT("Attempting to add client interest in non-spatial actor (%s), fix class routing"), *Actor->GetName());
				continue;
			}

			FConnectionReplicationActorInfo& ConnectionActorInfo = ConnectionActorInfoMap.FindOrAdd(Actor);

			// Following comment and logic is  from the replication flow - seems maybe relevant to do the same for interest?
			// Skip if dormant on this connection. We want this to always be the first/quickest check.
			if (ConnectionActorInfo.bDormantOnConnection)
			{
				continue;
			}

			// Always skip if we've already processed this Actor (because it was in multiple replication lists)
			if (ConnectionActorInfo.LastInterestUpdateFrameName == FrameNum)
			{
				continue;
			}
			ConnectionActorInfo.LastInterestUpdateFrameName = FrameNum;

			FGlobalActorReplicationInfo& GlobalActorInfo = GlobalActorReplicationInfoMap.Get(Actor);

			// At this point the replication flow checks ReadyForNextReplication
			// We avoid because we want interest to remain consistent vs. a per frame replication round robin.

			// Do we run explicit NCD calculations run per Actor?
			if (bUseNarrowPhaseNCDInterestCulling)
			{
				QUICK_SCOPE_CYCLE_COUNTER(NET_ClientEntityInterest_NarrowPhaseDistanceCalculation);

				float SmallestDistanceSq = TNumericLimits<float>::Max();

				// Multiple viewers exists in native Unreal to accommodate local multiplayer.
				int32 ViewersThatSkipActor = 0;
				for (const FNetViewer& CurViewer : Viewers)
				{
					const float DistSq = (GlobalActorInfo.WorldLocation - CurViewer.ViewLocation).SizeSquared();
					SmallestDistanceSq = FMath::Min<float>(DistSq, SmallestDistanceSq);
					if (ConnectionActorInfo.GetCullDistanceSquared() > 0.f && DistSq > ConnectionActorInfo.GetCullDistanceSquared())
					{
						++ViewersThatSkipActor;
						break;
					}
				}

				if (ViewersThatSkipActor >= Viewers.Num())
				{
					continue;
				}

				// At this point the replication flow uses distance from the player to affect replication priority.
			}

			// At this point the replication flow does starvation scaling which we don't care about for interest.

			// At this point the replication flow does pending dormancy scaling where it prioritizes replicating Actors that are changing
			// their dormancy so the change can be enacted. I guess we don't care for interest? We can just remove from interest list once
			// an Actor goes fully dormant.

			// At this point the replication flow does game code priority (where game code sets ForceNetUpdateFrame).

			// At this point the replication flow prioritizes the connection's owner and view target.

			ClientInterestedActors.Emplace(Actor);

			{
				QUICK_SCOPE_CYCLE_COUNTER(NET_ReplicateActors_DependentActors);
				GatherDependentActors(ConnectionActorInfoMap, GlobalActorInfo, ClientInterestedActors);
			}
		}
	}

	return ClientInterestedActors;
}

void USpatialReplicationGraph::GatherDependentActors(FPerConnectionActorInfoMap& ConnectionActorInfoMap,
													 FGlobalActorReplicationInfo& GlobalActorInfo, TArray<AActor*>& ClientInterestedActors)
{
	const uint32 FrameNum = GetReplicationGraphFrame();

	for (AActor* DependentActor : GlobalActorInfo.GetDependentActorList())
	{
		if (!ensureAlwaysMsgf(DependentActor != nullptr,
							  TEXT("Actor was nullptr when iterating through dependant Actors to calculate client interest")))
		{
			continue;
		}

		FConnectionReplicationActorInfo& DependentActorConnectionInfo = ConnectionActorInfoMap.FindOrAdd(DependentActor);

		// Always skip if we've already processed this Actor (because it was in another replication list for this connection)
		if (DependentActorConnectionInfo.LastInterestUpdateFrameName == FrameNum)
		{
			continue;
		}
		DependentActorConnectionInfo.LastInterestUpdateFrameName = FrameNum;

		ClientInterestedActors.Emplace(DependentActor);

		GatherDependentActors(ConnectionActorInfoMap, GlobalActorReplicationInfoMap.Get(DependentActor), ClientInterestedActors);
	}
}
