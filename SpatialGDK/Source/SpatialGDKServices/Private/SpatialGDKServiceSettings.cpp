// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKServiceSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialServiceSettings);
#define LOCTEXT_NAMESPACE "USpatialGDKServiceSettings"

USpatialGDKServiceSettings::USpatialGDKServiceSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bOpenSpatialOutputLogOnPIESessionEnd(true)
{
}
