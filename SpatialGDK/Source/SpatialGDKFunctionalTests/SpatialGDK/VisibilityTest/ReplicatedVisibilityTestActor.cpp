// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "ReplicatedVisibilityTestActor.h"

AReplicatedVisibilityTestActor::AReplicatedVisibilityTestActor()
{
	bNetLoadOnClient = false;
	bNetLoadOnNonAuthServer = true;
	SetActorEnableCollision(false);
}
