// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "ComponentWithSimilarlyNamedPropertyAndEventComponentUpdate.h"

test::ComponentWithSimilarlyNamedPropertyAndEvent::Update UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::DefaultUnderlying = test::ComponentWithSimilarlyNamedPropertyAndEvent::Update();

UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate()
{
}

UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::Init(const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update& underlying)
{
	return InitInternal(underlying);
}

UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::Reset()
{
	return ResetInternal();
}

bool UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::HasMyValue()
{
    return !Underlying.my_value().empty();
}

int UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::GetMyValue()
{
	MyValue = static_cast<int>((*(Underlying.my_value().data())));
    return MyValue;
}

UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::SetMyValue(int newValue)
{
    return SetMyValueInternal(newValue);
}

UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::AddMyValueUpdate(UTestType1* event)
{
    return AddMyValueUpdateInternal(event);
}

const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::GetUnderlying()
{
    return GetUnderlyingInternal();
}

UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::InitInternal(const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update& underlying)
{
    Underlying = test::ComponentWithSimilarlyNamedPropertyAndEvent::Update(underlying);
	return this;
}

UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::ResetInternal()
{
	return InitInternal(DefaultUnderlying);
}

UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::SetMyValueInternal(int newValue)
{
    Underlying.set_my_value(static_cast<std::int32_t>(newValue));
	return this;
}

UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate* UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::AddMyValueUpdateInternal(UTestType1* event)
{
    Underlying.add_my_value_update(event->GetUnderlying());
    return this;
}

const test::ComponentWithSimilarlyNamedPropertyAndEvent::Update UComponentWithSimilarlyNamedPropertyAndEventComponentUpdate::GetUnderlyingInternal()
{
    return Underlying;
}
