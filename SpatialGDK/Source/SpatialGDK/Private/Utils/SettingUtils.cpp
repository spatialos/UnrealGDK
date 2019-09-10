// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SettingUtils.h"
#include "SpatialConstants.h"
#include "SpatialPersistenceConfig.h"
#include "SpatialWorldSettings.h"

namespace SpatialGDK
{
bool ShouldClassPersist(const ASpatialWorldSettings* SpatialWorldSettings, const UClass* InClass)
{
	// If persistence settings don't exist, persist everything by default
	if (SpatialWorldSettings == nullptr)
	{
		return true;
	}

	const USpatialPersistenceConfig* SpatialPersistenceConfig = SpatialWorldSettings->GetSpatialPersistenceConfig();

	if (SpatialPersistenceConfig == nullptr)
	{
		return true;
	}

	const EPersistenceSelectionMode Mode = SpatialPersistenceConfig->GetPersistenceSelectionMode();
	const TArray<UClass*>& ClassList = SpatialPersistenceConfig->GetClassList();

	for (const UClass* Class : ClassList)
	{
		if (InClass->IsChildOf(Class))
		{
			return Mode == EPersistenceSelectionMode::PERSISTENCE_SELECTION_MODE_Exclude ? false : true;
		}
	}

	return Mode == EPersistenceSelectionMode::PERSISTENCE_SELECTION_MODE_Exclude ? true : false;
}
} // namespace SpatialGDK
