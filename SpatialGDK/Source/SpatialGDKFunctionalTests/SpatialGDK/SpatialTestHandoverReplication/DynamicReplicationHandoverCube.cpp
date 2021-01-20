// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicReplicationHandoverCube.h"

ADynamicReplicationHandoverCube::ADynamicReplicationHandoverCube()
{
	bReplicates = false;
}

void ADynamicReplicationHandoverCube::OnAuthorityGained()
{
	if (ShouldResetValueToDefaultCounter == 1)
	{
		HandoverTestProperty = BasicTestPropertyValue;
		ShouldResetValueToDefaultCounter = 2;
	}
}
