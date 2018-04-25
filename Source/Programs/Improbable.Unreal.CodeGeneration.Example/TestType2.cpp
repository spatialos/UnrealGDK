// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestType2.h"

UTestType2::UTestType2()
{
	Underlying.Reset(new test::TestType2(0));
}

UTestType2* UTestType2::Init(const test::TestType2& underlying)
{
    Underlying.Reset(new test::TestType2(underlying));
	return this;
}

float UTestType2::GetDoubleProperty()
{
    return static_cast<float>(Underlying->double_property());
}
UTestType2* UTestType2::SetDoubleProperty(float double_property)
{
    Underlying->set_double_property(static_cast<double>(double_property));
	return this;
}


test::TestType2 UTestType2::GetUnderlying()
{
	return *Underlying.Get();
}
