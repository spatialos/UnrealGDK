// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTestBlueprintLibrary.h"

FSpatialFunctionalTestStepDefinition USpatialFunctionalTestBlueprintLibrary::MakeStepDefinition(const FString& StepName,
																								const FStepIsReadyDelegate& IsReadyEvent,
																								const FStepStartDelegate& StartEvent,
																								const FStepTickDelegate& TickEvent,
																								const float StepTimeLimit)
{
	FSpatialFunctionalTestStepDefinition StepDefinition;
	StepDefinition.StepName = StepName;
	StepDefinition.IsReadyEvent = IsReadyEvent;
	StepDefinition.StartEvent = StartEvent;
	StepDefinition.TickEvent = TickEvent;
	StepDefinition.TimeLimit = StepTimeLimit;
	return StepDefinition;
}
