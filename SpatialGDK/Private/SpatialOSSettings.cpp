// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSettings.h"

FSpatialOSWorkerOverrideSettings::FSpatialOSWorkerOverrideSettings() : bDisableRendering(false)
{
}

USpatialGDKSettings::USpatialGDKSettings(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer), bUseUserWorkerConfigurations(false)
{
}
