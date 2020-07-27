// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/SpatialMultiserverSettings.h"
#include "SpatialGDKSettings.h"
#include "Utils/LayerInfo.h"

#include "GameFramework/WorldSettings.h"
#include "Templates/SubclassOf.h"

#include "SpatialWorldSettings.generated.h"

UCLASS()
class SPATIALGDK_API ASpatialWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Multiserver")
	TSubclassOf<USpatialMultiserverSettings> MultiserverSettingsClass;

	bool IsMultiserverEnabled() const
	{
		if (*MultiserverSettingsClass == nullptr)
		{
			return false;
		}

		const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
		if (SpatialGDKSettings->bOverrideMultiserver.IsSet())
		{
			return SpatialGDKSettings->bOverrideMultiserver.GetValue();
		}

		return true;
	}
};
