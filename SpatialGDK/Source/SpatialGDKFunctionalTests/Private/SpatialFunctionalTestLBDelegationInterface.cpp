// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialFunctionalTestLBDelegationInterface.h"
#include "GameFramework/Actor.h"
#include "SpatialCommonTypes.h"

bool ISpatialFunctionalTestLBDelegationInterface::AddActorDelegation(AActor* Actor, VirtualWorkerId WorkerId, bool bPersistOnTestFinished /*= false*/)
{
	if (Actor == nullptr)
	{
		return false;
	}

	uint32 ActorId = Actor->GetUniqueID();

	FSpatialFunctionalTestActorDelegation* Delegation = Delegations.Find(ActorId);
	if (Delegation == nullptr)
	{
		FSpatialFunctionalTestActorDelegation NewDelegation;
		NewDelegation.Id = ActorId;
		NewDelegation.ActorPtr = Actor;
		NewDelegation.bPersistOnTestFinished = bPersistOnTestFinished;
		Delegations.Add(ActorId, NewDelegation);
		return true;
	}

	if (Delegation->ActorPtr.Get() == nullptr) // was used but no longer is, we can reuse it
	{
		Delegation->ActorPtr = Actor;
		Delegation->bPersistOnTestFinished = bPersistOnTestFinished;
		return true;
	}
	// used by something else, should never happen
	ensureMsgf(false, TEXT("Trying to add delegation for Actor %s but there was already one with the same id for %s, this should never happen"), *Actor->GetName(), *Delegation->ActorPtr->GetName());
	return false;
}

bool ISpatialFunctionalTestLBDelegationInterface::RemoveActorDelegation(AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	uint32 ActorId = Actor->GetUniqueID();

	int32 NumRemoved = Delegations.Remove(Actor->GetUniqueID());
	ensureMsgf(NumRemoved == 1, TEXT("Removing delegation for %s, there should be 1 registered but found %d"), *Actor->GetName(), NumRemoved);
	return NumRemoved > 0;
}

bool ISpatialFunctionalTestLBDelegationInterface::HasActorDelegation(AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}
	FSpatialFunctionalTestActorDelegation* Delegation = Delegations.Find(Actor->GetUniqueID());
	return Delegation != nullptr && Delegation->ActorPtr.Get() == Actor;
}

void ISpatialFunctionalTestLBDelegationInterface::RemoveAllActorDelegations(bool bRemovePersistent /*= false*/)
{
	if( bRemovePersistent )
	{
		Delegations.Empty();
		return;
	}

	auto It = Delegations.CreateIterator();
	while (It)
	{
		if(!It->Value.bPersistOnTestFinished)
		{
			It.RemoveCurrent();
		}
		++It;
	}
}
