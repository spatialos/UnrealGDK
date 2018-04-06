// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOSSettings.h"

FSpatialOSWorkerOverrideSettings::FSpatialOSWorkerOverrideSettings() : bDisableRendering(false)
{
}

USpatialOSSettings::USpatialOSSettings(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer), bUseUserWorkerConfigurations(false)
{
}
