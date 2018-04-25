#include "StdStringToImprobableCoordinatesMap.h"
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

UStdStringToImprobableCoordinatesMap::UStdStringToImprobableCoordinatesMap()
{
}

UStdStringToImprobableCoordinatesMap* UStdStringToImprobableCoordinatesMap::Init(const worker::Map<std::string, improbable::Coordinates>& underlying)
{
	Underlying = worker::Map<std::string, improbable::Coordinates>(underlying);
	return this;
}

UStdStringToImprobableCoordinatesMap* UStdStringToImprobableCoordinatesMap::Emplace(const FString& key, const FVector& value)
{
	std::string underlyingKey = TCHAR_TO_UTF8(*key);
	auto underlyingValue = USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(value);
	Underlying.emplace(underlyingKey, underlyingValue);
	return this;
}

UStdStringToImprobableCoordinatesMap* UStdStringToImprobableCoordinatesMap::Remove(const FString& key)
{
	std::string underlyingKey = TCHAR_TO_UTF8(*key);
	Underlying.erase(underlyingKey);
	return this;
}

bool UStdStringToImprobableCoordinatesMap::Contains(const FString& key)
{
	std::string underlyingKey = TCHAR_TO_UTF8(*key);
	return Underlying.find(underlyingKey) != Underlying.end();
}

FVector UStdStringToImprobableCoordinatesMap::Get(const FString& key)
{
	std::string underlyingKey = TCHAR_TO_UTF8(*key);
	auto iterator = Underlying.find(underlyingKey);
	return USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>(iterator->second.x()), static_cast<float>(iterator->second.y()), static_cast<float>(iterator->second.z())));
}

UStdStringToImprobableCoordinatesMap* UStdStringToImprobableCoordinatesMap::Clear()
{
	Underlying.clear();
	return this;
}

bool UStdStringToImprobableCoordinatesMap::IsEmpty()
{
	return Underlying.empty();
}

bool UStdStringToImprobableCoordinatesMap::operator==(const worker::Map<std::string, improbable::Coordinates>& OtherUnderlying) const
{
	return Underlying == OtherUnderlying;
}

bool UStdStringToImprobableCoordinatesMap::operator!=(const worker::Map<std::string, improbable::Coordinates>& OtherUnderlying) const
{
	return Underlying != OtherUnderlying;
}

worker::Map<std::string, improbable::Coordinates> UStdStringToImprobableCoordinatesMap::GetUnderlying()
{
	return Underlying;
}