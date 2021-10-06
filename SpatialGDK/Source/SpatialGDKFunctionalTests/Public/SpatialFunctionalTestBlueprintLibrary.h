// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SpatialFunctionalTestStep.h"
#include "SpatialFunctionalTestBlueprintLibrary.generated.h"

UCLASS(meta = (ScriptName = "SpatialFunctionalTestLibrary"))
class SPATIALGDKFUNCTIONALTESTS_API USpatialFunctionalTestBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test",
			  meta = (AutoCreateRefTerm = "IsReadyEvent,StartEvent,TickEvent", NativeMakeFunc))
	static FSpatialFunctionalTestStepDefinition MakeStepDefinition(const FString& StepName, const FStepIsReadyDelegate& IsReadyEvent,
																   const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent,
																   const float StepTimeLimit);
};
