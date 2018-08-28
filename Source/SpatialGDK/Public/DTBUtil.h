// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialInterop.h"

bool FORCEINLINE ShouldUseDTB(USpatialInterop* DTBManager, UClass* Class)
{
	if (!DTBManager) return false;
	for (UClass* DTBClass : DTBManager->DTBClasses)
	{
		if (Class->IsChildOf(DTBClass)) return true;
	}
	return false;
}
