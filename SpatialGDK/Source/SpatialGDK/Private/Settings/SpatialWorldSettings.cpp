// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialWorldSettings.h"

ASpatialWorldSettings::ASpatialWorldSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const USpatialPersistenceConfig* ASpatialWorldSettings::GetSpatialPersistenceConfig() const
{
	return SpatialPersistenceConfig;
}

