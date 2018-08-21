// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialGDKEditorToolbarSettings.h"

bool FORCEINLINE ShouldUseDTB(UClass* Class)
{
	//return Class->IsChildOf(FindObject<UClass>(ANY_PACKAGE, TEXT("DTBActor")));
	const USpatialGDKEditorToolbarSettings* SpatialGDKToolbarSettings = GetDefault<USpatialGDKEditorToolbarSettings>();
	for (auto& Item : SpatialGDKToolbarSettings->InteropCodegenClasses)
	{
		if (Class->IsChildOf(Item.ReplicatedClass)) return true;
	}
	return false;
}
