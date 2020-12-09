// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Misc/AutomationTest.h"

#include "CoreMinimal.h"

#include "LocalDeploymentManager.h"

// TODO: UNR-1964 - Move EDeploymentState enum to LocalDeploymentManager
enum class EDeploymentState
{
	IsRunning,
	IsNotRunning
};

namespace SpatialGDK
{
FLocalDeploymentManager* GetLocalDeploymentManager();
} // namespace SpatialGDK

DEFINE_LATENT_AUTOMATION_COMMAND(FStartDeployment);
DEFINE_LATENT_AUTOMATION_COMMAND(FStopDeployment);
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FWaitForDeployment, FAutomationTestBase*, Test, EDeploymentState, ExpectedDeploymentState);
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckDeploymentState, FAutomationTestBase*, Test, EDeploymentState,
											   ExpectedDeploymentState);
