// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestData2ComponentUpdate.h"

test::TestData2::Update UTestData2ComponentUpdate::DefaultUnderlying = test::TestData2::Update();

UTestData2ComponentUpdate::UTestData2ComponentUpdate()
{
}

UTestData2ComponentUpdate* UTestData2ComponentUpdate::Init(const test::TestData2::Update& underlying)
{
	return InitInternal(underlying);
}

UTestData2ComponentUpdate* UTestData2ComponentUpdate::Reset()
{
	return ResetInternal();
}

bool UTestData2ComponentUpdate::HasDoubleProperty()
{
    return !Underlying.double_property().empty();
}

float UTestData2ComponentUpdate::GetDoubleProperty()
{
	DoubleProperty = static_cast<float>((*(Underlying.double_property().data())));
    return DoubleProperty;
}

UTestData2ComponentUpdate* UTestData2ComponentUpdate::SetDoubleProperty(float newValue)
{
    return SetDoublePropertyInternal(newValue);
}

UTestData2ComponentUpdate* UTestData2ComponentUpdate::AddTwoTextEvent(UTextEvent* event)
{
    return AddTwoTextEventInternal(event);
}

const test::TestData2::Update UTestData2ComponentUpdate::GetUnderlying()
{
    return GetUnderlyingInternal();
}

UTestData2ComponentUpdate* UTestData2ComponentUpdate::InitInternal(const test::TestData2::Update& underlying)
{
    Underlying = test::TestData2::Update(underlying);
	return this;
}

UTestData2ComponentUpdate* UTestData2ComponentUpdate::ResetInternal()
{
	return InitInternal(DefaultUnderlying);
}

UTestData2ComponentUpdate* UTestData2ComponentUpdate::SetDoublePropertyInternal(float newValue)
{
    Underlying.set_double_property(static_cast<double>(newValue));
	return this;
}

UTestData2ComponentUpdate* UTestData2ComponentUpdate::AddTwoTextEventInternal(UTextEvent* event)
{
    Underlying.add_two_text_event(event->GetUnderlying());
    return this;
}

const test::TestData2::Update UTestData2ComponentUpdate::GetUnderlyingInternal()
{
    return Underlying;
}
