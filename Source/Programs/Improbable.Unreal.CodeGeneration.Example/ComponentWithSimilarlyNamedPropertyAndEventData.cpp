// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "ComponentWithSimilarlyNamedPropertyAndEventData.h"

UComponentWithSimilarlyNamedPropertyAndEventData::UComponentWithSimilarlyNamedPropertyAndEventData()
{
	Underlying.Reset(new test::ComponentWithSimilarlyNamedPropertyAndEventData(0));
}

UComponentWithSimilarlyNamedPropertyAndEventData* UComponentWithSimilarlyNamedPropertyAndEventData::Init(const test::ComponentWithSimilarlyNamedPropertyAndEventData& underlying)
{
    Underlying.Reset(new test::ComponentWithSimilarlyNamedPropertyAndEventData(underlying));
	return this;
}

int UComponentWithSimilarlyNamedPropertyAndEventData::GetMyValue()
{
    return static_cast<int>(Underlying->my_value());
}
UComponentWithSimilarlyNamedPropertyAndEventData* UComponentWithSimilarlyNamedPropertyAndEventData::SetMyValue(int my_value)
{
    Underlying->set_my_value(static_cast<std::int32_t>(my_value));
	return this;
}


test::ComponentWithSimilarlyNamedPropertyAndEventData UComponentWithSimilarlyNamedPropertyAndEventData::GetUnderlying()
{
	return *Underlying.Get();
}
