#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"

#include "SpatialGDKSettings.generated.h"

UCLASS(config = SpatialGDKSettings, defaultconfig)

class SPATIALGDK_API USpatialGDKSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKSettings(const FObjectInitializer& ObjectInitializer);

	/** The number of entity IDs to be reserved when the entity pool is first created */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (ConfigRestartRequired = false, DisplayName = "Initial Entity ID Reservation Count"))
	uint32 EntityPoolInitialReservationCount;


	/** The minimum number of entity IDs available in the pool before a new batch is reserved */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (ConfigRestartRequired = false, DisplayName = "Pool Refresh Minimum Threshold"))
	uint32 EntityPoolRefreshThreshold;


	/** The number of entity IDs reserved when the minimum threshold is reached */
	UPROPERTY(EditAnywhere, config, Category = "Entity Pool", meta = (ConfigRestartRequired = false, DisplayName = "Refresh Count"))
	uint32 EntityPoolRefreshCount;

	virtual FString ToString();
};

