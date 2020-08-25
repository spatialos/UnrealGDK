// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "HandoverCube.h"
#include "Utils/SpatialStatics.h"
#include "Net/UnrealNetwork.h"

AHandoverCube::AHandoverCube()
{
	SetReplicateMovement(true);
	LockingServerID = 0;
}

void AHandoverCube::AcquireLock_Implementation(int ServerID)
{
	if (!USpatialStatics::IsLocked(this))
	{
		LockTocken = USpatialStatics::AcquireLock(this);
		LockingServerID = ServerID;
	}
}

void AHandoverCube::ReleaseLock_Implementation()
{
	if (USpatialStatics::IsLocked(this))
	{
		USpatialStatics::ReleaseLock(this, LockTocken);
		LockingServerID = 0;
	}
}

void AHandoverCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHandoverCube, LockingServerID);
}
