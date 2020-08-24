// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "HandoverCube.h"
#include "Utils/SpatialStatics.h"

AHandoverCube::AHandoverCube()
{
	SetReplicateMovement(true);
}

void AHandoverCube::AcquireLock_Implementation()
{
	if (!USpatialStatics::IsLocked(this))
	{
		LockTocken = USpatialStatics::AcquireLock(this); 
	}
}

void AHandoverCube::ReleaseLock_Implementation()
{
	if (USpatialStatics::IsLocked(this))
	{
		USpatialStatics::ReleaseLock(this, LockTocken);
	}
}
