// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "EntityTemplate.h"

UEntityTemplate::UEntityTemplate()
{
}

UEntityTemplate* UEntityTemplate::Init(const worker::Entity& underlying)
{
    Underlying = worker::Entity(underlying);
    return this;
}

UEntityTemplate* UEntityTemplate::AddTestData1Component(UTestType1* data)
{
    Underlying.Add<test::TestData1>(data->GetUnderlying());
	return this;
}

UEntityTemplate* UEntityTemplate::AddTestData2Component(UTestType2* data)
{
    Underlying.Add<test::TestData2>(data->GetUnderlying());
	return this;
}

UEntityTemplate* UEntityTemplate::AddBuiltInTypesComponent(UBuiltInTypesData* data)
{
    Underlying.Add<test::BuiltInTypes>(data->GetUnderlying());
	return this;
}

UEntityTemplate* UEntityTemplate::AddTestFieldTypesComponent(UTestFieldTypesData* data)
{
    Underlying.Add<test::TestFieldTypes>(data->GetUnderlying());
	return this;
}

UEntityTemplate* UEntityTemplate::AddTestEmptyComponent(UTestEmptyData* data)
{
    Underlying.Add<test::TestEmpty>(data->GetUnderlying());
	return this;
}

UEntityTemplate* UEntityTemplate::AddTestCommandResponseTypesComponent(UTestCommandResponseTypesData* data)
{
    Underlying.Add<test::TestCommandResponseTypes>(data->GetUnderlying());
	return this;
}

UEntityTemplate* UEntityTemplate::AddComponentWithSimilarlyNamedPropertyAndEventComponent(UComponentWithSimilarlyNamedPropertyAndEventData* data)
{
    Underlying.Add<test::ComponentWithSimilarlyNamedPropertyAndEvent>(data->GetUnderlying());
	return this;
}

worker::Entity UEntityTemplate::GetUnderlying()
{
	return Underlying;
}
