
#include "BoolOption.h"
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

UBoolOption::UBoolOption()
{
}

UBoolOption* UBoolOption::Init(const worker::Option<bool>& underlying)
{
	Underlying = worker::Option<bool>(underlying);
	return this;
}

UBoolOption* UBoolOption::SetValue(bool value)
{
	auto underlyingValue = value;
	Underlying.emplace(underlyingValue);
	return this;
}

bool UBoolOption::HasValue()
{
	return !Underlying.empty();
}

bool UBoolOption::GetValue()
{
	return (*(Underlying.data()));
}

void UBoolOption::Clear()
{
	Underlying.clear();
}

bool UBoolOption::operator==(const worker::Option<bool>& OtherUnderlying) const
{
	return Underlying == OtherUnderlying;
}

bool UBoolOption::operator!=(const worker::Option<bool>& OtherUnderlying) const
{
	return Underlying != OtherUnderlying;
}

worker::Option<bool> UBoolOption::GetUnderlying()
{
	return Underlying;
}
