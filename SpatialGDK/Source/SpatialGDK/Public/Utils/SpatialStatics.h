// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Internationalization/Text.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/LogMacros.h"
#include "Templates/SubclassOf.h"
#include "UObject/TextProperty.h"

#include "SpatialGDKSettings.h"

#include "SpatialStatics.generated.h"

class AActor;

// This log category will always log to the spatial runtime and thus also be printed in the SpatialOutput.
DECLARE_LOG_CATEGORY_EXTERN(LogSpatial, Log, All);

UCLASS()
class SPATIALGDK_API USpatialStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

	/**
	 * Returns true if SpatialOS Networking is enabled.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS")
	static bool IsSpatialNetworkingEnabled();

    /**
    * Returns true if SpatialOS Offloading is enabled.
    */
    UFUNCTION(BlueprintPure, Category = "SpatialOS|Offloading")
    static bool IsSpatialOffloadingEnabled();

	/**
	 * Functionally the same as the native Unreal PrintString but also logs to the spatial runtime.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, Keywords = "log print spatial", AdvancedDisplay = "2", DevelopmentOnly), Category = "Utilities|String")
	static void PrintStringSpatial(UObject* WorldContextObject, const FString& InString = FString(TEXT("Hello")), bool bPrintToScreen = true, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);

	/**
	 * Functionally the same as the native Unreal PrintText but also logs to the spatial runtime.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, Keywords = "log spatial", AdvancedDisplay = "2", DevelopmentOnly), Category = "Utilities|Text")
	static void PrintTextSpatial(UObject* WorldContextObject, const FText InText = INVTEXT("Hello"), bool bPrintToScreen = true, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);

	/**
	 * Returns true if worker flag with the given name was found.
	 * Gets value of a worker flag.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool GetWorkerFlag(const UObject* WorldContextObject, const FString& InFlagName, FString& OutFlagValue);

	/**
	 * Returns the Net Cull Distance distance/frequency pairs used in client qbi-f
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static TArray<FDistanceFrequencyPair> GetNCDDistanceRatios();

	/**
	 * Returns the full frequency net cull distance ratio used in client qbi-f
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static float GetFullFrequencyNetCullDistanceRatio();

	/**
	 * Returns the inspector colour for the given worker name.
	 * Argument expected in the form: UnrealWorker1a2s3d4f...
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static FColor GetInspectorColorForWorkerName(const FString& WorkerName);
};
