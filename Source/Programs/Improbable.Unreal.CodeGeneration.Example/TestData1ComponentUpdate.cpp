// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestData1ComponentUpdate.h"

test::TestData1::Update UTestData1ComponentUpdate::DefaultUnderlying = test::TestData1::Update();

UTestData1ComponentUpdate::UTestData1ComponentUpdate()
{
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::Init(const test::TestData1::Update& underlying)
{
	return InitInternal(underlying);
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::Reset()
{
	return ResetInternal();
}

bool UTestData1ComponentUpdate::HasStringProperty()
{
    return !Underlying.string_property().empty();
}

FString UTestData1ComponentUpdate::GetStringProperty()
{
	StringProperty = FString((*(Underlying.string_property().data())).c_str());
    return StringProperty;
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::SetStringProperty(FString newValue)
{
    return SetStringPropertyInternal(newValue);
}

bool UTestData1ComponentUpdate::HasInt32Property()
{
    return !Underlying.int32_property().empty();
}

int UTestData1ComponentUpdate::GetInt32Property()
{
	Int32Property = static_cast<int>((*(Underlying.int32_property().data())));
    return Int32Property;
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::SetInt32Property(int newValue)
{
    return SetInt32PropertyInternal(newValue);
}

bool UTestData1ComponentUpdate::HasCoordinatesProperty()
{
    return !Underlying.coordinates_property().empty();
}

FVector UTestData1ComponentUpdate::GetCoordinatesProperty()
{
	CoordinatesProperty = USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>((*(Underlying.coordinates_property().data())).x()), static_cast<float>((*(Underlying.coordinates_property().data())).y()), static_cast<float>((*(Underlying.coordinates_property().data())).z())));
    return CoordinatesProperty;
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::SetCoordinatesProperty(FVector newValue)
{
    return SetCoordinatesPropertyInternal(newValue);
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::AddOneTextEvent(UTextEvent* event)
{
    return AddOneTextEventInternal(event);
}

const test::TestData1::Update UTestData1ComponentUpdate::GetUnderlying()
{
    return GetUnderlyingInternal();
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::InitInternal(const test::TestData1::Update& underlying)
{
    Underlying = test::TestData1::Update(underlying);
	return this;
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::ResetInternal()
{
	return InitInternal(DefaultUnderlying);
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::SetStringPropertyInternal(FString newValue)
{
    Underlying.set_string_property(TCHAR_TO_UTF8(*newValue));
	return this;
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::SetInt32PropertyInternal(int newValue)
{
    Underlying.set_int32_property(static_cast<std::int32_t>(newValue));
	return this;
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::SetCoordinatesPropertyInternal(FVector newValue)
{
    Underlying.set_coordinates_property(USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(newValue));
	return this;
}

UTestData1ComponentUpdate* UTestData1ComponentUpdate::AddOneTextEventInternal(UTextEvent* event)
{
    Underlying.add_one_text_event(event->GetUnderlying());
    return this;
}

const test::TestData1::Update UTestData1ComponentUpdate::GetUnderlyingInternal()
{
    return Underlying;
}
