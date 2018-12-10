#pragma once

#include "Templates/UniquePtr.h"

#include <cstdint>

namespace improbable
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

}

template <typename T>
inline uint32 GetTypeHash(const improbable::TSchemaOption<T>& Option)
{
	return Option.IsSet() ? 1327u * (GetTypeHash(*Option) + 977u) : 977u;
}


using Worker_EntityId = std::int64_t;

struct FUnrealObjectRef
{
	FUnrealObjectRef() = default;

	FUnrealObjectRef(Worker_EntityId Entity, uint32 Offset)
		: Entity(Entity)
		, Offset(Offset)
	{}

	FUnrealObjectRef(Worker_EntityId Entity, uint32 Offset, FString Path, FUnrealObjectRef Outer)
		: Entity(Entity)
		, Offset(Offset)
		, Path(Path)
		, Outer(Outer)
	{}

	FUnrealObjectRef(const FUnrealObjectRef& In)
		: Entity(In.Entity)
		, Offset(In.Offset)
		, Path(In.Path)
		, Outer(In.Outer)
	{}

	FORCEINLINE FUnrealObjectRef& operator=(const FUnrealObjectRef& In)
	{
		Entity = In.Entity;
		Offset = In.Offset;
		Path = In.Path;
		Outer = In.Outer;
		return *this;
	}

	FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("(entity ID: %lld, offset: %u)"), Entity, Offset);
	}

	FORCEINLINE bool operator==(const FUnrealObjectRef& Other) const
	{
		return Entity == Other.Entity &&
			Offset == Other.Offset &&
			((!Path && !Other.Path) || (Path && Other.Path && Path->Equals(*Other.Path))) &&
			((!Outer && !Other.Outer) || (Outer && Other.Outer && *Outer == *Other.Outer));
	}

	FORCEINLINE bool operator!=(const FUnrealObjectRef& Other) const
	{
		return !operator==(Other);
	}

	Worker_EntityId Entity;
	uint32 Offset;
	improbable::TSchemaOption<FString> Path;
	improbable::TSchemaOption<FUnrealObjectRef> Outer;
};

inline uint32 GetTypeHash(const FUnrealObjectRef& ObjectRef)
{
	uint32 Result = 1327u;
	Result = (Result * 977u) + GetTypeHash(static_cast<int64>(ObjectRef.Entity));
	Result = (Result * 977u) + GetTypeHash(ObjectRef.Offset);
	Result = (Result * 977u) + GetTypeHash(ObjectRef.Path);
	Result = (Result * 977u) + GetTypeHash(ObjectRef.Outer);
	return Result;
}