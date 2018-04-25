#include "StdStringList.h"
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

UStdStringList::UStdStringList()
{
}

UStdStringList* UStdStringList::Init(const worker::List<std::string>& underlying)
{
	Underlying = worker::List<std::string>(underlying);
	return this;
}

UStdStringList* UStdStringList::Add(const FString& value)
{
	auto underlyingValue = TCHAR_TO_UTF8(*value);
	Underlying.emplace_back(underlyingValue);
	return this;
}

FString UStdStringList::Get(int pos)
{
	return FString((Underlying)[pos].c_str());
}

UStdStringList* UStdStringList::Clear()
{
	Underlying.clear();
	return this;
}

bool UStdStringList::IsEmpty()
{
	return Underlying.empty();
}

int UStdStringList::Size()
{
	return static_cast<int>(Underlying.size());
}

bool UStdStringList::operator==(const worker::List<std::string>& OtherUnderlying) const
{
	return Underlying == OtherUnderlying;
}

bool UStdStringList::operator!=(const worker::List<std::string>& OtherUnderlying) const
{
	return Underlying != OtherUnderlying;
}

worker::List<std::string> UStdStringList::GetUnderlying()
{
	return Underlying;
}
