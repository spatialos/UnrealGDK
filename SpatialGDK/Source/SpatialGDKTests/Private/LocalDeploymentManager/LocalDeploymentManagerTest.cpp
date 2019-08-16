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
	static const double MAX_WAIT_TIME_FOR_LOCAL_DEPLOYMENT_OPERATION = 10.0;
	enum class EDeploymentState { IsRunning, IsNotRunning };

	FLocalDeploymentManager* GetLocalDeploymentManager()
	{
		FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
		FLocalDeploymentManager* LocalDeploymentManager = GDKServices.GetLocalDeploymentManager();
		return LocalDeploymentManager;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND(StartDeployment);
bool StartDeployment::Update()
{
	bool bRedeployRequired = false;
	if (!GenerateDefaultWorkerJson(bRedeployRequired))
	{
		return true;
	}

	const FString LaunchConfig = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()), TEXT("Improbable/AutomationLaunchConfig.json"));
	if (!GenerateDefaultLaunchConfig(LaunchConfig))
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

DEFINE_LATENT_AUTOMATION_COMMAND(StopDeployment);
bool StopDeployment::Update()
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

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(WaitForDeployment, FAutomationTestBase*, Test, EDeploymentState, ExpectedDeploymentState);
bool WaitForDeployment::Update()
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

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(CheckDeploymentState, FAutomationTestBase*, Test, EDeploymentState, ExpectedDeploymentState);
bool CheckDeploymentState::Update()
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
