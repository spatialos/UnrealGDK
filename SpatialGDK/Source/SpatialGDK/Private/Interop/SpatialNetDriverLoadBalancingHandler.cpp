// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialNetDriverLoadBalancingHandler.h"

#include "EngineClasses/SpatialNetDriver.h"

FSpatialNetDriverLoadBalancingContext::FSpatialNetDriverLoadBalancingContext(USpatialNetDriver* InNetDriver, TArray<FNetworkObjectInfo*>& InOutNetworkObjects)
	: NetDriver(InNetDriver)
	, NetworkObjects(InOutNetworkObjects)
{

}

void FSpatialNetDriverLoadBalancingContext::UpdateWithAdditionalActors()
{
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

FSpatialNetDriverLoadBalancingContext::FNetworkObjectsArrayAdaptor FSpatialNetDriverLoadBalancingContext::GetActorsBeingReplicated()
{
	return FNetworkObjectsArrayAdaptor(NetworkObjects);
}

void FSpatialNetDriverLoadBalancingContext::RemoveAdditionalActor(AActor* Actor)
{
	if (FNetworkObjectInfo* Info = NetDriver->FindNetworkObjectInfo(Actor))
	{
		AdditionalActorsToReplicate.Remove(Info);
	}
}

void FSpatialNetDriverLoadBalancingContext::AddActorToReplicate(AActor* Actor)
{
	if(FNetworkObjectInfo* Info = NetDriver->FindNetworkObjectInfo(Actor))
	{
		AdditionalActorsToReplicate.Add(Info);
	}
}

TArray<AActor*>& FSpatialNetDriverLoadBalancingContext::GetDependentActors(AActor* Actor)
{
	return Actor->Children;
}

