#pragma once

#include "Templates/UniquePtr.h"

namespace SpatialGDK
{

template <typename T>
class TSchemaOption
{
public:
	TSchemaOption() = default;
	~TSchemaOption() = default;

	TSchemaOption(const T& InValue)
		: Value(MakeUnique<T>(InValue))
	{}

	TSchemaOption(T&& InValue)
		: Value(MakeUnique<T>(MoveTemp(InValue)))
	{}

	TSchemaOption(const TSchemaOption& InValue)
	{
		*this = InValue;
	}

	TSchemaOption(TSchemaOption&&) = default;

	TSchemaOption& operator=(const TSchemaOption& InValue)
	{
		if (this != &InValue)
		{
			if (InValue)
			{
				Value = MakeUnique<T>(*InValue);
			}
		}

		return *this;
	}

	TSchemaOption& operator=(TSchemaOption&&) = default;

	FORCEINLINE bool IsSet() const
	{
		return Value.IsValid();
	}

	FORCEINLINE explicit operator bool() const
	{
		return IsSet();
	}

	const T& GetValue() const
	{
		checkf(IsSet(), TEXT("It is an error to call GetValue() on an unset TSchemaOption. Please check IsSet()."));
		return *Value;
	}

	T& GetValue()
	{
		checkf(IsSet(), TEXT("It is an error to call GetValue() on an unset TSchemaOption. Please check IsSet()."));
		return *Value;
	}

	bool operator==(const TSchemaOption& InValue) const
	{
		if (IsSet() != InValue.IsSet())
		{
			return false;
		}

		if (!IsSet())
		{
			return true;
		}

		return GetValue() == InValue.GetValue();
	}

	bool operator!=(const TSchemaOption& InValue) const
	{
		return !operator==(InValue);
	}

	T& operator*() const
	{
		return *Value;
	}

	const T* operator->() const
	{
		return Value.Get();
	}

	T* operator->()
	{
		return Value.Get();
	}

private:
	TUniquePtr<T> Value;
};

} // namespace SpatialGDK

template <typename T>
inline uint32 GetTypeHash(const SpatialGDK::TSchemaOption<T>& Option)
{
	return Option.IsSet() ? 1327u * (GetTypeHash(*Option) + 977u) : 977u;
}
