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

	// TODO(Alex): When using AutomationWorker, permissions in WorkerConnection tests fails
	//const FName AutomationWorkerType = TEXT("AutomationWorker");
	const FName AutomationWorkerType = TEXT("UnrealWorker");
	const FString AutomationLaunchConfig = TEXT("Improbable/AutomationLaunchConfig.json");

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
		FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialGDKServicesConstants::SpatialExe, BuildConfigArgs, SpatialGDKServicesConstants::SpatialOSDirectory, WorkerBuildConfigResult, ExitCode);

		const int32 ExitCodeSuccess = 0;
		return (ExitCode == ExitCodeSuccess);
	}

	bool GenerateWorkerJson()
	{
		const FString WorkerJsonDir = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("workers/unreal"));

		FString JsonPath = FPaths::Combine(WorkerJsonDir, TEXT("spatialos.UnrealAutomation.worker.json"));
		if (!FPaths::FileExists(JsonPath))
		{
			bool bRedeployRequired = false;
			return GenerateDefaultWorkerJson(JsonPath, AutomationWorkerType.ToString(), bRedeployRequired);
		}

		return true;
	}
}

bool FStartDeployment::Update()
{
	if (const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>())
	{
		FLocalDeploymentManager* LocalDeploymentManager = GetLocalDeploymentManager();
		const FString LaunchConfig = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()), AutomationLaunchConfig);
		const FString LaunchFlags = SpatialGDKSettings->GetSpatialOSCommandLineLaunchFlags();
		const FString SnapshotName = SpatialGDKSettings->GetSpatialOSSnapshotToLoad();

		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [LocalDeploymentManager, LaunchConfig, LaunchFlags, SnapshotName]
		{
			if (!GenerateWorkerJson())
			{
				return;
			}

			if (!GenerateWorkerAssemblies())
			{
				return;
			}

			FSpatialLaunchConfigDescription LaunchConfigDescription(AutomationWorkerType);

			if (!GenerateDefaultLaunchConfig(LaunchConfig, &LaunchConfigDescription))
			{
				return;
			}

			if (LocalDeploymentManager->IsLocalDeploymentRunning())
			{
				return;
			}

			LocalDeploymentManager->TryStartLocalDeployment(LaunchConfig, LaunchFlags, SnapshotName, TEXT(""));
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
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [LocalDeploymentManager]
		{
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
			Test->TestTrue(TEXT("Deployment is running"), LocalDeploymentManager->IsLocalDeploymentRunning() && !LocalDeploymentManager->IsDeploymentStopping());
		}
		else
		{
			Test->TestFalse(TEXT("Deployment is not running"), LocalDeploymentManager->IsLocalDeploymentRunning() || LocalDeploymentManager->IsDeploymentStopping());
		}
		return true;
	}
	
	if (LocalDeploymentManager->IsDeploymentStopping())
	{
		return false;
	}
	else
	{
		return (ExpectedDeploymentState == EDeploymentState::IsRunning) ? LocalDeploymentManager->IsLocalDeploymentRunning() : !LocalDeploymentManager->IsLocalDeploymentRunning();
	}
}

bool FCheckDeploymentState::Update()
{
	FLocalDeploymentManager* LocalDeploymentManager = GetLocalDeploymentManager();

	if (ExpectedDeploymentState == EDeploymentState::IsRunning)
	{
		Test->TestTrue(TEXT("Deployment is running"), LocalDeploymentManager->IsLocalDeploymentRunning() && !LocalDeploymentManager->IsDeploymentStopping());
	}
	else
	{
		Test->TestFalse(TEXT("Deployment is not running"), LocalDeploymentManager->IsLocalDeploymentRunning() || LocalDeploymentManager->IsDeploymentStopping());
	}

	return true;
}
