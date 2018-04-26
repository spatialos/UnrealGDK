// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API USpatialOSConversionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "SpatialOS Conversions")
	static FRotator GetSpatialOsToUnrealCoordinateSpace();

	UFUNCTION(BlueprintPure, Category = "SpatialOS Conversions")
	static float GetSpatialOsToUnrealScale();

	UFUNCTION(BlueprintPure, Category = "SpatialOS Conversions")
	static FVector UnrealCoordinatesToSpatialOsCoordinates(const FVector& unrealCoordinates);

	static improbable::Coordinates
	UnrealCoordinatesToSpatialOsCoordinatesCast(const FVector& unrealCoordinates);

	UFUNCTION(BlueprintPure, Category = "SpatialOS Conversions")
	static FVector SpatialOsCoordinatesToUnrealCoordinates(const FVector& spatialOsCoordinates);

	UFUNCTION(BlueprintPure, Category = "SpatialOS Conversions")
	static FQuat UnrealRotationToSpatialOsRotation(const FQuat& unrealRotation);

	UFUNCTION(BlueprintPure, Category = "SpatialOS Conversions")
	static FQuat SpatialOsRotationToUnrealRotation(const FQuat& spatialOsRotation);
};
