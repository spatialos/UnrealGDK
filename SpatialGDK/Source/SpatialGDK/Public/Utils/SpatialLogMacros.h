// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/LogMacros.h"

#include "SpatialLogMacros.generated.h"

// This log category will always log to the spatial runtime and thus also be printed in the SpatialOutput.
DECLARE_LOG_CATEGORY_EXTERN(LogSpatial, Log, All);

UCLASS()
class SPATIALGDK_API USpatialLogMacros : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// Functionally the same as the native Unreal PrintString but also logs to the spatial runtime.
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, Keywords = "log print spatial", AdvancedDisplay = "2", DevelopmentOnly), Category = "Utilities|String")
	static void PrintStringSpatial(UObject* WorldContextObject, const FString& InString = FString(TEXT("Hello")), bool bPrintToScreen = true, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);
};
