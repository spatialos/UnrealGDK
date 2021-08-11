// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKSettings.h"
#include "CustomGDKFilterSettings.generated.h"

UCLASS()
class UCustomGDKFilterSettings : public UEventTracingSamplingSettings
{
	GENERATED_BODY()

public:
	UCustomGDKFilterSettings()
	{
		SamplingProbability = 1.0;
		GDKEventPreFilter = "type = \"user.send_rpc\"";
		GDKEventPostFilter = "true";
		RuntimeEventPreFilter = "true";
		RuntimeEventPostFilter = "true";
	}
};
