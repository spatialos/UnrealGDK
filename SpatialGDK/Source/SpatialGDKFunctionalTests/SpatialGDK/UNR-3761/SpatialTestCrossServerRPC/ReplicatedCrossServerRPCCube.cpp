// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedCrossServerRPCCube.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Net/UnrealNetwork.h"

AReplicatedCrossServerRPCCube::AReplicatedCrossServerRPCCube()
{
	bAlwaysRelevant = true;
	bNetLoadOnClient = true;
	bNetLoadOnNonAuthServer = true;
}

void AReplicatedCrossServerRPCCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedCrossServerRPCCube, ReceivedCrossServerRPCS);
	DOREPLIFETIME(AReplicatedCrossServerRPCCube, AuthEntityId);
}

void AReplicatedCrossServerRPCCube::CrossServerTestRPC_Implementation(int SendingServerID)
{
	ReceivedCrossServerRPCS.Add(SendingServerID);
}

void AReplicatedCrossServerRPCCube::RecordEntityId()
{
	if (HasAuthority())
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		AuthEntityId = SpatialNetDriver->PackageMap->GetEntityIdFromObject(this);
	}
}
