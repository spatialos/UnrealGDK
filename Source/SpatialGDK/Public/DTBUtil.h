// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "DTBManager.h"

bool FORCEINLINE ShouldUseDTB(UDTBManager* DTBManager, UClass* Class)
{
	//return Class->IsChildOf(FindObject<UClass>(ANY_PACKAGE, TEXT("DTBActor")));
	if (!DTBManager) return false;
	for (UClass* DTBClass : DTBManager->DTBClasses)
	{
		if (Class->IsChildOf(DTBClass)) return true;
	}
	return false;
}
