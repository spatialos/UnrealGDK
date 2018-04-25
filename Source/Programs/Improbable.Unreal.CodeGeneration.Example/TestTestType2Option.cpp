
#include "TestTestType2Option.h"
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

UTestTestType2Option::UTestTestType2Option()
{
}

UTestTestType2Option* UTestTestType2Option::Init(const worker::Option<test::TestType2>& underlying)
{
	Underlying = worker::Option<test::TestType2>(underlying);
	return this;
}

UTestTestType2Option* UTestTestType2Option::SetValue(UTestType2* value)
{
	auto underlyingValue = value->GetUnderlying();
	Underlying.emplace(underlyingValue);
	return this;
}

bool UTestTestType2Option::HasValue()
{
	return !Underlying.empty();
}

UTestType2* UTestTestType2Option::GetValue()
{
	return NewObject<UTestType2>()->Init((*(Underlying.data())));
}

void UTestTestType2Option::Clear()
{
	Underlying.clear();
}

bool UTestTestType2Option::operator==(const worker::Option<test::TestType2>& OtherUnderlying) const
{
	return Underlying == OtherUnderlying;
}

bool UTestTestType2Option::operator!=(const worker::Option<test::TestType2>& OtherUnderlying) const
{
	return Underlying != OtherUnderlying;
}

worker::Option<test::TestType2> UTestTestType2Option::GetUnderlying()
{
	return Underlying;
}
