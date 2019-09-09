// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "UObject/ObjectMacros.h"

#include "SpatialWorldSettings.generated.h"

class USpatialPersistenceConfig;

UCLASS()
class SPATIALGDK_API ASpatialWorldSettings : public AWorldSettings
{
	GENERATED_UCLASS_BODY()

public:
	const USpatialPersistenceConfig* GetSpatialPersistenceConfig() const;

private:
	UPROPERTY(EditAnywhere, Category = Spatial, meta = (DisplayName = "Spatial Persistence"))
	class USpatialPersistenceConfig* SpatialPersistenceConfig;
};

