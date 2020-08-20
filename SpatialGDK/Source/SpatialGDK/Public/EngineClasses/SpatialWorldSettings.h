// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "SpatialGDKSettings.h"
// Temporary note: the below include builds and runs locally but fails on a Linux build as it cannot find the header file
#include "SpatialGDKServicesConstants.h"

#include "Utils/LayerInfo.h"
#include "Utils/SpatialStatics.h"

#include "GameFramework/WorldSettings.h"
#include "Templates/SubclassOf.h"

#include "SpatialWorldSettings.generated.h"

/**
 * MapTestingMode allows Maps using ASpatialWorldSettings to define how Tests should run by the Automation Manager.
 * It will handle it in a way that the current Project Settings will remain untouched.
 */
UENUM()
enum class EMapTestingMode : uint8
{
	// It will search for ASpatialFunctionalTest, if there are any it forces Spatial otherwise Native
	Detect							= 0,
	// Forces Spatial to be off and Play Offline (1 Client, no network), the default Native behaviour
	ForceNativeOffline			= 1,
	// Forces Spatial to be off and Play As Listen Server (1 Client that is also Server, ie authoritive Client)
	ForceNativeAsListenServer	= 2,
	// Forces Spatial to be off and Play As Client (1 Client + 1 Dedicated Server)
	ForceNativeAsClient			= 3,
	// Forces Spatial to be on. Calculates the number of players needed based on the ASpatialFunctionalTests present, 1 if none exists
	ForceSpatial					= 4,
	// Uses current settings to run the tests
	UseCurrentSettings				= 5  
};

USTRUCT(BlueprintType)
struct FMapTestingSettings
{
	GENERATED_BODY();


	/* Available Modes to run Tests:
	- Detect: It will search for ASpatialFunctionalTest, if there are any it forces Spatial otherwise Native
	- Force Native: Forces Spatial to be off and use only 1 Client, the default Native behaviour
	- Force Spatial: Forces Spatial to be on. Calculates the number of players needed based on the ASpatialFunctionalTests present, 1 if none exists
	- Use Current Settings: Uses current settings to run the tests */
	UPROPERTY(EditAnywhere, Category = "Default")
	EMapTestingMode TestingMode = EMapTestingMode::Detect;
};

UCLASS()
class SPATIALGDK_API ASpatialWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

private:
	/** Enable running different server worker types to split the simulation. */
	UPROPERTY(EditAnywhere, Config, Category = "Multi-Worker")
	bool bEnableMultiWorker;

public:
	UPROPERTY(EditAnywhere, Category = "Multi-Worker", meta = (EditCondition = "bEnableMultiWorker"))
	TSubclassOf<USpatialMultiWorkerSettings> CloudMultiWorkerSettingsClass;

	UPROPERTY(EditAnywhere, Category = "Multi-Worker", meta = (EditCondition = "bEnableMultiWorker"))
	TSubclassOf<USpatialMultiWorkerSettings> LocalMultiWorkerSettingsClass;

	TSubclassOf<USpatialMultiWorkerSettings> GetMultiWorkerSettingsClass() const
	{
		// If a local deployment is starting up or running return the local multi worker settings, otherwise return the cloud worker settings
		if (FSpatialGDKServicesModule* GDKServices = FModuleManager::GetModulePtr<FSpatialGDKServicesModule>("SpatialGDKServices"))
		{
			FLocalDeploymentManager* LocalDeploymentManager = GDKServices->GetLocalDeploymentManager();
			//if (LocalDeploymentManager->IsDeploymentStarting() || LocalDeploymentManager->IsLocalDeploymentRunning())
			if (LocalDeploymentManager->UseLocalLaunchConfig())
			{
				return LocalMultiWorkerSettingsClass;
			}
		}
		return CloudMultiWorkerSettingsClass;
	}
	
	bool IsMultiWorkerEnabled() const
	{
		if (*GetMultiWorkerSettingsClass() == nullptr)
		{
			return false;
		}

		const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
		if (SpatialGDKSettings->bOverrideMultiWorker.IsSet())
		{
			return SpatialGDKSettings->bOverrideMultiWorker.GetValue();
		}

		return true;
	}
	
#if WITH_EDITORONLY_DATA
	/** Defines how Unreal Editor will run the Tests in this map, without changing current Settings. */
	UPROPERTY(EditAnywhere, Category = "Testing")
	FMapTestingSettings TestingSettings;
#endif

	// This function is used to expose the private bool property to SpatialStatics.
	// You should call USpatialStatics::IsMultiWorkerEnabled to properly check whether multi-worker is enabled.
	bool IsMultiWorkerEnabledInWorldSettings() const { return bEnableMultiWorker && *GetMultiWorkerSettingsClass() != nullptr; }
};
