// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicReplicationHandoverCube.h"

#include "Net/UnrealNetwork.h"

ADynamicReplicationHandoverCube::ADynamicReplicationHandoverCube()
{
	bReplicates = false;
}

void ADynamicReplicationHandoverCube::OnAuthorityGained()
{
	if (ShouldResetValueToDefaultCounter == 1)
	{
		HandoverTestProperty = BasicTestPropertyValue;
		ReplicatedTestProperty = BasicTestPropertyValue;
		ShouldResetValueToDefaultCounter = 2;
	}
}

void ADynamicReplicationHandoverCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADynamicReplicationHandoverCube, ReplicatedTestProperty);
}
