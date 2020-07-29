// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "SpatialGDKSettings.h"
#include "SpatialCommandUtils.h"
#include "SpatialGDKServicesConstants.h"

#include "Utils/LayerInfo.h"

#include "GameFramework/WorldSettings.h"
#include "Templates/SubclassOf.h"

#include "SpatialWorldSettings.generated.h"

UCLASS()
class SPATIALGDK_API ASpatialWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
	TSubclassOf<USpatialMultiWorkerSettings> CloudMultiWorkerSettingsClass;

	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
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
};
