// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "HandoverCube.h"
#include "Net/UnrealNetwork.h"
#include "Utils/SpatialStatics.h"

AHandoverCube::AHandoverCube()
{
	LockingServerID = 0;
	AuthorityChanges = 0;
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

void AHandoverCube::OnAuthorityGained()
{
	Super::OnAuthorityGained();

	++AuthorityChanges;
}

void AHandoverCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHandoverCube, LockingServerID);
	DOREPLIFETIME(AHandoverCube, AuthorityChanges);
}
