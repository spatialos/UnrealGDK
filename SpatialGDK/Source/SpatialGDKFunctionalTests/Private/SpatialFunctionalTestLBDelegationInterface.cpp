// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialFunctionalTestLBDelegationInterface.h"
#include "GameFramework/Actor.h"
#include "SpatialCommonTypes.h"
#include "Utils/SpatialStatics.h"

bool ISpatialFunctionalTestLBDelegationInterface::AddActorDelegation(AActor* Actor, VirtualWorkerId WorkerId, bool bPersistOnTestFinished /*= false*/)
{
	if (Actor == nullptr)
	{
		return false;
	}

	int64 ActorId = USpatialStatics::GetActorEntityId(Actor);

	if (ActorId == 0)
	{
		ensureMsgf(false, TEXT("Trying to add delegation for Actor %s but it still doesn't have an Spatial Entity Id"), *Actor->GetName());
		return false;
	}

	FSpatialFunctionalTestActorDelegation* Delegation = Delegations.Find(ActorId);
	if (Delegation == nullptr)
	{
		FSpatialFunctionalTestActorDelegation NewDelegation;
		NewDelegation.EntityId = ActorId;
		NewDelegation.ActorPtr = Actor;
		NewDelegation.WorkerId = WorkerId;
		NewDelegation.bPersistOnTestFinished = bPersistOnTestFinished;
		Delegations.Add(ActorId, NewDelegation);
		return true;
	}

	if (Delegation->ActorPtr.Get() == nullptr) // was used but no longer is, we can reuse it
	{
		Delegation->ActorPtr = Actor;
		Delegation->WorkerId = WorkerId;
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

	int64 ActorId = USpatialStatics::GetActorEntityId(Actor);

	int32 NumRemoved = Delegations.Remove(ActorId);
	ensureMsgf(NumRemoved == 1, TEXT("Removing delegation for %s, there should be 1 registered but found %d"), *Actor->GetName(), NumRemoved);
	return NumRemoved > 0;
}

bool ISpatialFunctionalTestLBDelegationInterface::HasActorDelegation(AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}
	FSpatialFunctionalTestActorDelegation* Delegation = Delegations.Find(USpatialStatics::GetActorEntityId(Actor));
	if (Delegation == nullptr)
	{
		return false;
	}
	if( Delegation->ActorPtr.Get() != Actor )
	{
		FString OtherActorName = Delegation->ActorPtr.Get() != nullptr ? Delegation->ActorPtr->GetName() : FString("NULL");
		ensureMsgf(false, TEXT("Found delegation with id %d, but the Actor references don't match; was expecting %s and got %s"), *Actor->GetName(), *OtherActorName);
		return false;
	}
	return true;
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
