// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"

#include "LocalDeploymentManager.h"
#include "SpatialGDKDefaultLaunchConfigGenerator.h"
#include "SpatialGDKDefaultWorkerJsonGenerator.h"
#include "SpatialGDKEditorSettings.h"

#include "CoreMinimal.h"

#define LOCALDEPLOYMENT_TEST(TestName) \
	TEST(Services, LocalDeployment, TestName)

namespace
{
	// TODO: UNR-1969 - Prepare LocalDeployment in CI pipeline
	static const double MAX_WAIT_TIME_FOR_LOCAL_DEPLOYMENT_OPERATION = 10.0;

	// TODO: UNR-1964 - Move EDeploymentState enum to LocalDeploymentManager
	enum class EDeploymentState { IsRunning, IsNotRunning };

	static const FName AutomationWorkerType = TEXT("AutomationWorker");
	static const FString AutomationLaunchConfig = TEXT("Improbable/AutomationLaunchConfig.json");

	FLocalDeploymentManager* GetLocalDeploymentManager()
	{
		FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
		FLocalDeploymentManager* LocalDeploymentManager = GDKServices.GetLocalDeploymentManager();
		return LocalDeploymentManager;
	}

	FSpatialLaunchConfigDescription GenerateLaunchConfigDescription()
	{
		FSpatialLaunchConfigDescription LaunchConfigDescription;
		FWorkerTypeLaunchSection UnrealWorkerDefaultSetting;
		UnrealWorkerDefaultSetting.WorkerTypeName = FName(AutomationWorkerType);
		UnrealWorkerDefaultSetting.Rows = 1;
		UnrealWorkerDefaultSetting.Columns = 1;
		UnrealWorkerDefaultSetting.bManualWorkerConnectionOnly = true;
		LaunchConfigDescription.ServerWorkers.Reset();
		LaunchConfigDescription.ServerWorkers.Add(UnrealWorkerDefaultSetting);

		return LaunchConfigDescription;
	}

	bool GenerateWorkerPB()
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

DEFINE_LATENT_COMMAND(StartDeployment)
{
	if (!GenerateWorkerJson())
	{
		return true;
	}

	if (!GenerateWorkerPB())
	{
		return true;
	}

	const FString LaunchConfig = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()), AutomationLaunchConfig);
	FSpatialLaunchConfigDescription LaunchConfigDescription = GenerateLaunchConfigDescription();
	if (!GenerateDefaultLaunchConfig(LaunchConfig, &LaunchConfigDescription))
	{
		return true;
	}

	FLocalDeploymentManager* LocalDeploymentManager = GetLocalDeploymentManager();
	if (LocalDeploymentManager->IsLocalDeploymentRunning())
	{
		return true;
	}

	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	const FString LaunchFlags = SpatialGDKSettings->GetSpatialOSCommandLineLaunchFlags();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [LocalDeploymentManager, LaunchConfig, LaunchFlags]
	{
		return LocalDeploymentManager->TryStartLocalDeployment(LaunchConfig, LaunchFlags);
	}
	);

	return true;
}

DEFINE_LATENT_COMMAND(StopDeployment)
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
			return LocalDeploymentManager->TryStopLocalDeployment();
		});
	}

	return true;
}

DEFINE_LATENT_COMMAND_TWO_PARAMETERS(WaitForDeployment, FAutomationTestBase*, Test, EDeploymentState, ExpectedDeploymentState)
{
	const double NewTime = FPlatformTime::Seconds();
	if (NewTime - StartTime >= MAX_WAIT_TIME_FOR_LOCAL_DEPLOYMENT_OPERATION)
	{
		return true;
	}

	FLocalDeploymentManager* LocalDeploymentManager = GetLocalDeploymentManager();
	if (LocalDeploymentManager->IsDeploymentStopping())
	{
		return false;
	}
	else
	{
		return (ExpectedDeploymentState == EDeploymentState::IsRunning) ? LocalDeploymentManager->IsLocalDeploymentRunning() : !LocalDeploymentManager->IsLocalDeploymentRunning();
	}
}

DEFINE_LATENT_COMMAND_TWO_PARAMETERS(CheckDeploymentState, FAutomationTestBase*, Test, EDeploymentState, ExpectedDeploymentState)
{
	FLocalDeploymentManager* LocalDeploymentManager = GetLocalDeploymentManager();

	if (ExpectedDeploymentState == EDeploymentState::IsRunning)
	{
		Test->TestTrue(TEXT("Deployment is running"), LocalDeploymentManager->IsLocalDeploymentRunning() && !LocalDeploymentManager->IsDeploymentStopping());
	}
	else
	{
		Test->TestFalse(TEXT("Deployment is running"), LocalDeploymentManager->IsLocalDeploymentRunning() || LocalDeploymentManager->IsDeploymentStopping());
	}

	return true;
}

LOCALDEPLOYMENT_TEST(GIVEN_no_deployment_running_WHEN_deployment_started_THEN_deployment_running)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(StopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(WaitForDeployment(this, EDeploymentState::IsNotRunning));

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(StartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(WaitForDeployment(this, EDeploymentState::IsRunning));

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(CheckDeploymentState(this, EDeploymentState::IsRunning));

	// Cleanup
	ADD_LATENT_AUTOMATION_COMMAND(StopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(WaitForDeployment(this, EDeploymentState::IsNotRunning));
    return true;
}

LOCALDEPLOYMENT_TEST(GIVEN_deployment_running_WHEN_deployment_stopped_THEN_deployment_not_running)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(StopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(WaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(StartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(WaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(StopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(WaitForDeployment(this, EDeploymentState::IsNotRunning));

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(CheckDeploymentState(this, EDeploymentState::IsNotRunning));
    return true;
}
