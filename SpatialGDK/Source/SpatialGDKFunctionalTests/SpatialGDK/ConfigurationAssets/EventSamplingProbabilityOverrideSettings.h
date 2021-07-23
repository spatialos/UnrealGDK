// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKSettings.h"
#include "EventSamplingProbabilityOverrideSettings.generated.h"

UCLASS()
class UEventSamplingProbabilityOverrideSettings : public UEventTracingSamplingSettings
{
	GENERATED_BODY()

public:

	UEventSamplingProbabilityOverrideSettings();

	static FName OverridenEventType;

};
