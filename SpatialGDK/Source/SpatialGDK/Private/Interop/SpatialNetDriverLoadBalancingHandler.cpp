// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialNetDriverLoadBalancingHandler.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"

FSpatialNetDriverLoadBalancingContext::FSpatialNetDriverLoadBalancingContext(USpatialNetDriver* InNetDriver,
																			 TArray<FNetworkObjectInfo*>& InOutNetworkObjects)
	: NetDriver(InNetDriver)
	, NetworkObjects(InOutNetworkObjects)
{
}

void FSpatialNetDriverLoadBalancingContext::UpdateWithAdditionalActors()
{
	for (auto ActorInfoIter = AdditionalActorsToReplicate.CreateIterator(); ActorInfoIter; ++ActorInfoIter)
	{
		FNetworkObjectInfo* ActorInfo = *ActorInfoIter;
		AActor* Actor = ActorInfo->Actor;

		ActorInfo->bPendingNetUpdate = false;

		// Call PreReplication on all actors that will be considered
		Actor->CallPreReplication(NetDriver);

		// Add it to the consider list.
		NetworkObjects.Add(ActorInfo);
	}
}

bool FSpatialNetDriverLoadBalancingContext::IsActorReadyForMigration(AActor* Actor)
{
	// Auth check.
	if (!Actor->HasAuthority())
	{
		return false;
	}

	// These checks are extracted from UNetDriver::ServerReplicateActors_BuildNetworkObjects

	if (Actor->IsPendingKillPending())
	{
		return false;
	}

	// Verify the actor is actually initialized (it might have been intentionally spawn deferred until a later frame)
	if (!Actor->IsActorInitialized())
	{
		return false;
	}

	// Don't send actors that may still be streaming in or out
	ULevel* Level = Actor->GetLevel();
	if (Level->HasVisibilityChangeRequestPending() || Level->bIsAssociatingLevel)
	{
		return false;
	}

	if (Actor->NetDormancy == DORM_Initial && Actor->IsNetStartupActor())
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
	if (FNetworkObjectInfo* Info = NetDriver->FindNetworkObjectInfo(Actor))
	{
		AdditionalActorsToReplicate.Add(Info);
	}
}

TArray<AActor*>& FSpatialNetDriverLoadBalancingContext::GetDependentActors(AActor* Actor)
{
	return Actor->Children;
}
