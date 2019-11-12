// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"

#include "LocalDeploymentManager.h"
#include "SpatialGDKDefaultLaunchConfigGenerator.h"
#include "SpatialGDKDefaultWorkerJsonGenerator.h"
#include "SpatialGDKEditorSettings.h"

#include "CoreMinimal.h"

#define LOCALDEPLOYMENT_TEST(TestName) \
	GDK_TEST(Services, LocalDeployment, TestName)

namespace
{
	// TODO: UNR-1969 - Prepare LocalDeployment in CI pipeline
	const double MAX_WAIT_TIME_FOR_LOCAL_DEPLOYMENT_OPERATION = 30.0;

	// TODO: UNR-1964 - Move EDeploymentState enum to LocalDeploymentManager
	enum class EDeploymentState { IsRunning, IsNotRunning };

	const FName AutomationWorkerType = TEXT("AutomationWorker");
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
		const FString SpatialExe(TEXT("spatial.exe"));
		FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialExe, BuildConfigArgs, FSpatialGDKServicesModule::GetSpatialOSDirectory(), WorkerBuildConfigResult, ExitCode);

		const int32 ExitCodeSuccess = 0;
		return (ExitCode == ExitCodeSuccess);
	}

	bool GenerateWorkerJson()
	{
		const FString WorkerJsonDir = FSpatialGDKServicesModule::GetSpatialOSDirectory(TEXT("workers/unreal"));

		FString JsonPath = FPaths::Combine(WorkerJsonDir, TEXT("spatialos.UnrealAutomation.worker.json"));
		if (!FPaths::FileExists(JsonPath))
		{
			bool bRedeployRequired = false;
			return GenerateDefaultWorkerJson(JsonPath, AutomationWorkerType.ToString(), bRedeployRequired);
		}

		return true;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND(FStartDeployment);
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

DEFINE_LATENT_AUTOMATION_COMMAND(FStopDeployment);
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

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FWaitForDeployment, FAutomationTestBase*, Test, EDeploymentState, ExpectedDeploymentState);
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

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckDeploymentState, FAutomationTestBase*, Test, EDeploymentState, ExpectedDeploymentState);
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

LOCALDEPLOYMENT_TEST(GIVEN_no_deployment_running_WHEN_deployment_started_THEN_deployment_running)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FCheckDeploymentState(this, EDeploymentState::IsRunning));

	// Cleanup
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
    return true;
}

LOCALDEPLOYMENT_TEST(GIVEN_deployment_running_WHEN_deployment_stopped_THEN_deployment_not_running)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FCheckDeploymentState(this, EDeploymentState::IsNotRunning));
    return true;
}
