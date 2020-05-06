// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"

namespace SpatialGDKServices
{
void SPATIALGDKSERVICES_API InitWorkers(bool bConnectAsClient, int32 PlayInEditorID, FString& OutWorkerId);
void SPATIALGDKSERVICES_API OnSpatialShutdown();
} // namespace SpatialGDKServices

#endif // WITH_EDITOR
