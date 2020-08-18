// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalDeploymentManagerUtilities.h"

#include "LocalDeploymentManager.h"
#include "SpatialGDKDefaultLaunchConfigGenerator.h"
#include "SpatialGDKDefaultWorkerJsonGenerator.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesConstants.h"

#include "CoreMinimal.h"

namespace
{
// TODO: UNR-1969 - Prepare LocalDeployment in CI pipeline
const double MAX_WAIT_TIME_FOR_LOCAL_DEPLOYMENT_OPERATION = 30.0;

const FName AutomationWorkerType = TEXT("AutomationWorker");
const FString AutomationLaunchConfig = FString(TEXT("Improbable/")) + *AutomationWorkerType.ToString() + FString(TEXT(".json"));

FLocalDeploymentManager* GetLocalDeploymentManager()
{
	FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
	FLocalDeploymentManager* LocalDeploymentManager = GDKServices.GetLocalDeploymentManager();
	return LocalDeploymentManager;
}

bool GenerateWorkerAssemblies()
{
	FString BuildConfigArgs = TEXT("worker build build-config");
	FString WorkerBuildConfigResult;
	int32 ExitCode;
	FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialGDKServicesConstants::SpatialExe, BuildConfigArgs,
													SpatialGDKServicesConstants::SpatialOSDirectory, WorkerBuildConfigResult, ExitCode);

	const int32 ExitCodeSuccess = 0;
	return (ExitCode == ExitCodeSuccess);
}

bool GenerateWorkerJson()
{
	const FString WorkerJsonDir = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("workers/unreal"));

	FString Filename = FString(TEXT("spatialos.")) + *AutomationWorkerType.ToString() + FString(TEXT(".worker.json"));
	FString JsonPath = FPaths::Combine(WorkerJsonDir, Filename);
	if (!FPaths::FileExists(JsonPath))
	{
		bool bRedeployRequired = false;
		return GenerateDefaultWorkerJson(JsonPath, AutomationWorkerType.ToString(), bRedeployRequired);
	}

	return true;
}
} // namespace

bool FStartDeployment::Update()
{
	if (const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>())
	{
		FLocalDeploymentManager* LocalDeploymentManager = GetLocalDeploymentManager();
		const FString LaunchConfig =
			FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()), AutomationLaunchConfig);
		const FString LaunchFlags = SpatialGDKSettings->GetSpatialOSCommandLineLaunchFlags();
		const FString SnapshotName = SpatialGDKSettings->GetSpatialOSSnapshotToLoad();
		const FString RuntimeVersion = SpatialGDKSettings->GetSelectedRuntimeVariantVersion().GetVersionForLocal();

		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [LocalDeploymentManager, LaunchConfig, LaunchFlags, SnapshotName,
																 RuntimeVersion] {
			if (!GenerateWorkerJson())
			{
				return;
			}

			if (!GenerateWorkerAssemblies())
			{
				return;
			}

			FSpatialLaunchConfigDescription LaunchConfigDescription;

			FWorkerTypeLaunchSection Conf;

			if (!GenerateLaunchConfig(LaunchConfig, &LaunchConfigDescription, Conf))
			{
				return;
			}

			if (LocalDeploymentManager->IsLocalDeploymentRunning())
			{
				return;
			}

			LocalDeploymentManager->TryStartLocalDeployment(LaunchConfig, RuntimeVersion, LaunchFlags, SnapshotName, TEXT(""), nullptr);
		});
	}

	return true;
}

bool FStopDeployment::Update()
{
	FLocalDeploymentManager* LocalDeploymentManager = GetLocalDeploymentManager();

	if (!LocalDeploymentManager->IsLocalDeploymentRunning() && !LocalDeploymentManager->IsDeploymentStopping())
	{
		return true;
	}

	if (!LocalDeploymentManager->IsDeploymentStopping())
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [LocalDeploymentManager] {
			LocalDeploymentManager->TryStopLocalDeployment();
		});
	}

	return true;
}

bool FWaitForDeployment::Update()
{
	FLocalDeploymentManager* const LocalDeploymentManager = GetLocalDeploymentManager();

	const double NewTime = FPlatformTime::Seconds();

	if (NewTime - StartTime >= MAX_WAIT_TIME_FOR_LOCAL_DEPLOYMENT_OPERATION)
	{
		// The given time for the deployment to start/stop has expired - test its current state.
		if (ExpectedDeploymentState == EDeploymentState::IsRunning)
		{
			Test->TestTrue(TEXT("Deployment is running"),
						   LocalDeploymentManager->IsLocalDeploymentRunning() && !LocalDeploymentManager->IsDeploymentStopping());
		}
		else
		{
			Test->TestFalse(TEXT("Deployment is not running"),
							LocalDeploymentManager->IsLocalDeploymentRunning() || LocalDeploymentManager->IsDeploymentStopping());
		}
		return true;
	}

	if (LocalDeploymentManager->IsDeploymentStopping())
	{
		return false;
	}
	else
	{
		return (ExpectedDeploymentState == EDeploymentState::IsRunning) ? LocalDeploymentManager->IsLocalDeploymentRunning()
																		: !LocalDeploymentManager->IsLocalDeploymentRunning();
	}
}

bool FCheckDeploymentState::Update()
{
	FLocalDeploymentManager* LocalDeploymentManager = GetLocalDeploymentManager();

	if (ExpectedDeploymentState == EDeploymentState::IsRunning)
	{
		Test->TestTrue(TEXT("Deployment is running"),
					   LocalDeploymentManager->IsLocalDeploymentRunning() && !LocalDeploymentManager->IsDeploymentStopping());
	}
	else
	{
		Test->TestFalse(TEXT("Deployment is not running"),
						LocalDeploymentManager->IsLocalDeploymentRunning() || LocalDeploymentManager->IsDeploymentStopping());
	}

	return true;
}
