// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestType1.h"

UTestType1::UTestType1()
{
	Underlying.Reset(new test::TestType1("", 0, improbable::Coordinates(0, 0, 0)));
}

UTestType1* UTestType1::Init(const test::TestType1& underlying)
{
    Underlying.Reset(new test::TestType1(underlying));
	return this;
}

FString UTestType1::GetStringProperty()
{
    return FString(Underlying->string_property().c_str());
}
UTestType1* UTestType1::SetStringProperty(FString string_property)
{
    Underlying->set_string_property(TCHAR_TO_UTF8(*string_property));
	return this;
}

int UTestType1::GetInt32Property()
{
    return static_cast<int>(Underlying->int32_property());
}
UTestType1* UTestType1::SetInt32Property(int int32_property)
{
    Underlying->set_int32_property(static_cast<std::int32_t>(int32_property));
	return this;
}

FVector UTestType1::GetCoordinatesProperty()
{
    return USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>(Underlying->coordinates_property().x()), static_cast<float>(Underlying->coordinates_property().y()), static_cast<float>(Underlying->coordinates_property().z())));
}
UTestType1* UTestType1::SetCoordinatesProperty(FVector coordinates_property)
{
    Underlying->set_coordinates_property(USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(coordinates_property));
	return this;
}


test::TestType1 UTestType1::GetUnderlying()
{
	return *Underlying.Get();
}
