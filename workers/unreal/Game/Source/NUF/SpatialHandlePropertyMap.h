// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct RepHandleData
{
	UProperty* Parent;
	UProperty* Property;
	int32 Offset;
};

using RepHandlePropertyMap = TMap<int32, RepHandleData>;



#include "Net/RepLayout.h"
#include "Engine/ActorChannel.h"
#include "SpatialNetDriver.h"
#include "SpatialOS.h"

class ConditionMapFilter
{
public:
	ConditionMapFilter(UActorChannel* ActorChannel)
	{
		// Reconstruct replication flags on the client side.
		FReplicationFlags RepFlags;
		RepFlags.bReplay = 0;
		RepFlags.bNetInitial = 1; // The server will only ever send one update for bNetInitial, so just let them through here.
		RepFlags.bNetSimulated = ActorChannel->Actor->Role == ROLE_SimulatedProxy;
		// TODO(david): Just pretend everything is the owner. This is not great behaviour.
		RepFlags.bNetOwner = 1;// ActorChannel->Actor->IsOwnedBy(ActorChannel->Connection->PlayerController);
		RepFlags.bRepPhysics = ActorChannel->Actor->ReplicatedMovement.bRepPhysics;

		UE_LOG(LogTemp, Warning, TEXT("CMF Actor %s NetOwner %d Simulated %d RepPhysics %d Client %s"), *ActorChannel->Actor->GetName(),
			RepFlags.bNetOwner,
			RepFlags.bNetSimulated,
			RepFlags.bRepPhysics,
			*Cast<USpatialNetDriver>(ActorChannel->Connection->Driver)->GetSpatialOS()->GetWorkerConfiguration().GetWorkerId());

		// Build a ConditionMap. This code is taken directly from FRepLayout::RebuildConditionalProperties
		static_assert(COND_Max == 14, "We are expecting 14 rep conditions"); // Guard in case more are added.
		const bool bIsInitial = RepFlags.bNetInitial ? true : false;
		const bool bIsOwner = RepFlags.bNetOwner ? true : false;
		const bool bIsSimulated = RepFlags.bNetSimulated ? true : false;
		const bool bIsPhysics = RepFlags.bRepPhysics ? true : false;
		const bool bIsReplay = RepFlags.bReplay ? true : false;

		ConditionMap[COND_None] = true;
		ConditionMap[COND_InitialOnly] = bIsInitial;

		ConditionMap[COND_OwnerOnly] = bIsOwner;
		ConditionMap[COND_SkipOwner] = !bIsOwner;

		ConditionMap[COND_SimulatedOnly] = bIsSimulated;
		ConditionMap[COND_SimulatedOnlyNoReplay] = bIsSimulated && !bIsReplay;
		ConditionMap[COND_AutonomousOnly] = !bIsSimulated;

		ConditionMap[COND_SimulatedOrPhysics] = bIsSimulated || bIsPhysics;
		ConditionMap[COND_SimulatedOrPhysicsNoReplay] = (bIsSimulated || bIsPhysics) && !bIsReplay;

		ConditionMap[COND_InitialOrOwner] = bIsInitial || bIsOwner;
		ConditionMap[COND_ReplayOrOwner] = bIsReplay || bIsOwner;
		ConditionMap[COND_ReplayOnly] = bIsReplay;
		ConditionMap[COND_SkipReplay] = !bIsReplay;

		ConditionMap[COND_Custom] = true;
	}

	bool PropertyIsRelevant(ELifetimeCondition Condition) const
	{
		return ConditionMap[Condition];
	}

private:
	bool ConditionMap[COND_Max];

};
