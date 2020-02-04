// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"

#include "UObject/Package.h"

#include "SpatialGDKServiceSettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialServiceSettings, Log, All);

UCLASS(config = SpatialGDKServiceSettings, defaultconfig)
class SPATIALGDKSERVICES_API USpatialGDKServiceSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKServiceSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, config, Category = "Editor Settings")
		bool bOpenSpatialOutputLogOnPIESessionEnd;

private:

};
