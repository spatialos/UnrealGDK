// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPersistenceConfig.h"

USpatialPersistenceConfig::USpatialPersistenceConfig(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	, PersistenceSelectionMode(EPersistenceSelectionMode::PERSISTENCE_SELECTION_MODE_Exclude)
{

}

EPersistenceSelectionMode USpatialPersistenceConfig::GetPersistenceSelectionMode() const
{
	return PersistenceSelectionMode;
}

const TArray<UClass*>& USpatialPersistenceConfig::GetClassList() const
{
	return ClassList;
}
