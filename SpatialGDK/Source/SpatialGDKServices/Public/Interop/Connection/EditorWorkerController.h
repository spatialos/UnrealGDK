// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"

namespace SpatialGDKServices
{
FString SPATIALGDKSERVICES_API InitWorkers(const FString& WorkerType, bool bConnectAsClient);
void SPATIALGDKSERVICES_API OnSpatialShutdown();
} // namespace SpatialGDKServices

#endif
