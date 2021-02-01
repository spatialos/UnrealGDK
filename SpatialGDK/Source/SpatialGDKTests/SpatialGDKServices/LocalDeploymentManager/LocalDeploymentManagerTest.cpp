// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "LocalDeploymentManagerUtilities.h"

#include "CoreMinimal.h"

#define LOCALDEPLOYMENT_TEST(TestName) GDK_TEST(Services, LocalDeployment, TestName)

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
