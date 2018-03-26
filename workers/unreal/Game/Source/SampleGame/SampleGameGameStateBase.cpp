// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SampleGameGameStateBase.h"

void ASampleGameGameStateBase::FakeServerHasBegunPlay()
{
	if (!GetWorld()) 
	{
		return;
	}
	Role = ROLE_SimulatedProxy;
	
	bReplicatedHasBegunPlay = true;
	OnRep_ReplicatedHasBegunPlay();
}
