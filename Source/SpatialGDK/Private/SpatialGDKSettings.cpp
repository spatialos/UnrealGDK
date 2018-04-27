// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSettings.h"

FSpatialGDKWorkerOverrideSettings::FSpatialGDKWorkerOverrideSettings() : bDisableRendering(false)
{
}

USpatialGDKSettings::USpatialGDKSettings(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer), bUseUserWorkerConfigurations(false)
{
}
