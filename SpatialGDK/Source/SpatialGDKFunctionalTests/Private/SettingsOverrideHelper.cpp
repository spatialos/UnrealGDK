// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SettingsOverrideHelper.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"

/**
 * This helper class provides common checks for the override settings
 */

SettingsOverrideHelper::SettingsOverrideHelper(){};

void SettingsOverrideHelper::VerifyNumberOfClients(int32 ExpectedNumberOfClients, ASpatialFunctionalTest* SpatialFunctionalTest)
{
	int32 ActualNumberOfClients = 0;
	// Cannot check the ULevelEditorPlaySettings directly as we overwrite a copy - so we can test for the effect instead
	for (ASpatialFunctionalTestFlowController* FlowController : SpatialFunctionalTest->GetFlowControllers())
	{
		if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
		{
			ActualNumberOfClients++;
		}
	}

	SpatialFunctionalTest->RequireTrue(ActualNumberOfClients == ExpectedNumberOfClients,
									  TEXT("Expected PlayNumberOfClients to be ActualNumberOfClients"));
}
