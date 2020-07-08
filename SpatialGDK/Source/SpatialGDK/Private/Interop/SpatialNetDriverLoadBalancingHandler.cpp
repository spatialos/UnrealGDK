// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialNetDriverLoadBalancingHandler.h"

FSpatialNetDriverLoadBalancingHandler::FSpatialNetDriverLoadBalancingHandler(USpatialNetDriver* InNetDriver, TArray<FNetworkObjectInfo*>& InOutNetworkObjects)
	: TSpatialLoadBalancingHandler<FSpatialNetDriverLoadBalancingHandler>(InNetDriver)
	, NetworkObjects(InOutNetworkObjects)
{

}

void FSpatialNetDriverLoadBalancingHandler::HandleLoadBalancing()
{
	TSpatialLoadBalancingHandler<FSpatialNetDriverLoadBalancingHandler>::HandleLoadBalancing();

	// Processing actors that we pull from actor hierarchies, in order to migrate them together.
	// This loop is extracted from UNetDriver::ServerReplicateActors_BuildNetworkObjects
	for (auto ActorInfoIter = AdditionalActorsToReplicate.CreateIterator(); ActorInfoIter; ++ActorInfoIter)
	{
		FNetworkObjectInfo* ActorInfo = *ActorInfoIter;
		AActor* Actor = ActorInfo->Actor;

		if (Actor->IsPendingKillPending())
		{
			continue;
		}

		// Verify the actor is actually initialized (it might have been intentionally spawn deferred until a later frame)
		if (!Actor->IsActorInitialized())
		{
			continue;
		}

		// Don't send actors that may still be streaming in or out
		ULevel* Level = Actor->GetLevel();
		if (Level->HasVisibilityChangeRequestPending() || Level->bIsAssociatingLevel)
		{
			continue;
		}

		if (Actor->NetDormancy == DORM_Initial && Actor->IsNetStartupActor())
		{
			continue;
		}

		ActorInfo->bPendingNetUpdate = false;

		// Call PreReplication on all actors that will be considered
		Actor->CallPreReplication(NetDriver);

		// Add it to the consider list.
		NetworkObjects.Add(ActorInfo);
	}
}

FSpatialNetDriverLoadBalancingHandler::FNetworkObjectsArrayAdaptor FSpatialNetDriverLoadBalancingHandler::GetActorsBeingReplicated()
{
	return FNetworkObjectsArrayAdaptor(NetworkObjects);
}

void FSpatialNetDriverLoadBalancingHandler::RemoveAdditionalActor(AActor* Actor)
{
	if (FNetworkObjectInfo* Info = NetDriver->FindNetworkObjectInfo(Actor))
	{
		AdditionalActorsToReplicate.Remove(Info);
	}
}

void FSpatialNetDriverLoadBalancingHandler::AddActorToReplicate(AActor* Actor)
{
	if(FNetworkObjectInfo* Info = NetDriver->FindNetworkObjectInfo(Actor))
	{
		AdditionalActorsToReplicate.Add(Info);
	}
}

TArray<AActor*>& FSpatialNetDriverLoadBalancingHandler::GetDependentActors(AActor* Actor)
{
	return Actor->Children;
}

