// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Logging/LogMacros.h"

#if WITH_EDITOR
#include "Engine/GameInstance.h"
#define SPATIAL_LOG(CategoryName, Verbosity, Format, ...) \
{\
	const UGameInstance* _GameInstance = (this->GetWorld() ? this->GetWorld()->GetGameInstance() : nullptr); \
	UE_LOG(CategoryName, Verbosity, "[%s] " Format, (_GameInstance ? *(_GameInstance->GetSpatialWorkerLabel()) : TEXT("UNKNOWN")), ##__VA_ARGS__) \
}
#else // not WITH_EDITOR
#define SPATIAL_LOG(CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, Format, ##__VA_ARGS__)
#endif // WITH_EDITOR
