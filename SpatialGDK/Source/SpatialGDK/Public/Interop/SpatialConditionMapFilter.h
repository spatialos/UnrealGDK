// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Engine/NetConnection.h"
#include "Net/RepLayout.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"

class FSpatialConditionMapFilter
{
public:
	FSpatialConditionMapFilter(USpatialActorChannel* ActorChannel, bool bIsClient)
	{
		// Reconstruct replication flags on the client side.
		FReplicationFlags RepFlags;
		RepFlags.bReplay = 0;
		RepFlags.bNetInitial = 1; // The server will only ever send one update for bNetInitial, so just let them through here.
		RepFlags.bNetSimulated = ActorChannel->Actor->Role == ROLE_SimulatedProxy;
		RepFlags.bNetOwner = bIsClient;
#if ENGINE_MINOR_VERSION <= 23
		RepFlags.bRepPhysics = ActorChannel->Actor->ReplicatedMovement.bRepPhysics;
#else
		RepFlags.bRepPhysics = ActorChannel->Actor->GetReplicatedMovement().bRepPhysics;
#endif

#if 0
		UE_LOG(LogTemp, Verbose, TEXT("CMF Actor %s (%lld) NetOwner %d Simulated %d RepPhysics %d Client %s"),
			*ActorChannel->Actor->GetName(),
			ActorChannel->GetEntityId(),
			RepFlags.bNetOwner,
			RepFlags.bNetSimulated,
			RepFlags.bRepPhysics);
#endif

		// Build a ConditionMap. This code is taken directly from FRepLayout::BuildConditionMapFromRepFlags
		static_assert(COND_Max == 16, "We are expecting 16 rep conditions"); // Guard in case more are added.
		const bool bIsInitial = RepFlags.bNetInitial ? true : false;
		const bool bIsOwner = RepFlags.bNetOwner ? true : false;
		const bool bIsSimulated = RepFlags.bNetSimulated ? true : false;
		const bool bIsPhysics = RepFlags.bRepPhysics ? true : false;
		const bool bIsReplay = RepFlags.bReplay ? true : false;

		ConditionMap[COND_None] = true;
		ConditionMap[COND_InitialOnly] = bIsInitial;

		ConditionMap[COND_OwnerOnly] = bIsOwner;
		ConditionMap[COND_SkipOwner] =
			!ActorChannel
				 ->IsAuthoritativeClient(); // TODO: UNR-3714, this is a best-effort measure, but SkipOwner is currently quite broken

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
		ConditionMap[COND_Never] = false;
	}

	bool IsRelevant(ELifetimeCondition Condition) const { return ConditionMap[Condition]; }

private:
	bool ConditionMap[COND_Max];
};
