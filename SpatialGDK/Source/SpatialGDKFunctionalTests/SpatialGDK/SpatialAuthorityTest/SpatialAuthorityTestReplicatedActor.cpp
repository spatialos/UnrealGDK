// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialAuthorityTestReplicatedActor.h"

ASpatialAuthorityTestReplicatedActor::ASpatialAuthorityTestReplicatedActor()
	: Super()
{
	bReplicates = true;
}

void ASpatialAuthorityTestReplicatedActor::OnAuthorityGained()
{
	// Clear Authority on Tick.
	AuthorityOnTick = 0; 
}
