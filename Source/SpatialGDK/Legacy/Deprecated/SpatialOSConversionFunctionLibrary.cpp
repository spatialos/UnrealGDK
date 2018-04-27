// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "SpatialOSConversionFunctionLibrary.h"

FRotator USpatialOSConversionFunctionLibrary::GetSpatialOsToUnrealCoordinateSpace()
{
	return FRotator{0.0f, -90.0f, -90.0f};
}

float USpatialOSConversionFunctionLibrary::GetSpatialOsToUnrealScale()
{
	return 100.0f;
}

FVector USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinates(const FVector& unrealCoordinates)
{
	return GetSpatialOsToUnrealCoordinateSpace().GetInverse().RotateVector(unrealCoordinates) /
		GetSpatialOsToUnrealScale();
}

improbable::Coordinates
USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(const FVector& unrealCoordinates)
{
	FVector spatialCoords = UnrealCoordinatesToSpatialOsCoordinates(unrealCoordinates);
	return improbable::Coordinates{spatialCoords.X, spatialCoords.Y, spatialCoords.Z};
}

FVector
USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(const FVector& spatialOsCoordinates)
{
	return GetSpatialOsToUnrealCoordinateSpace().RotateVector(spatialOsCoordinates) * GetSpatialOsToUnrealScale();
}

FQuat USpatialOSConversionFunctionLibrary::UnrealRotationToSpatialOsRotation(const FQuat& unrealRotation)
{
	return GetSpatialOsToUnrealCoordinateSpace().GetInverse().Quaternion() * unrealRotation;
}

FQuat USpatialOSConversionFunctionLibrary::SpatialOsRotationToUnrealRotation(const FQuat& spatialOsRotation)
{
	return GetSpatialOsToUnrealCoordinateSpace().Quaternion() * spatialOsRotation;
}
