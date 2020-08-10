// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Tests/TestDefinitions.h"

#include "CoreMinimal.h"

// TODO: UNR-1964 - Move EDeploymentState enum to LocalDeploymentManager
enum class EDeploymentState
{
	IsRunning,
	IsNotRunning
};

DEFINE_LATENT_AUTOMATION_COMMAND(FStartDeployment);
DEFINE_LATENT_AUTOMATION_COMMAND(FStopDeployment);
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FWaitForDeployment, FAutomationTestBase*, Test, EDeploymentState, ExpectedDeploymentState);
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckDeploymentState, FAutomationTestBase*, Test, EDeploymentState,
											   ExpectedDeploymentState);
