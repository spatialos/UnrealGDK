// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SchemaOption.h"

namespace improbable
{

struct UnrealObjectRef
{
	UnrealObjectRef() = default;

	UnrealObjectRef(Worker_EntityId Entity, uint32 Offset)
		: Entity(Entity)
		, Offset(Offset)
	{}

	UnrealObjectRef(Worker_EntityId Entity, uint32 Offset, FString Path, UnrealObjectRef Outer)
		: Entity(Entity)
		, Offset(Offset)
		, Path(Path)
		, Outer(Outer)
	{}

	UnrealObjectRef(const UnrealObjectRef& In)
		: Entity(In.Entity)
		, Offset(In.Offset)
		, Path(In.Path)
		, Outer(In.Outer)
	{}

	FORCEINLINE UnrealObjectRef& operator=(const UnrealObjectRef& In)
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

	FORCEINLINE bool operator==(const UnrealObjectRef& Other) const
	{
		return Entity == Other.Entity &&
			Offset == Other.Offset &&
			((!Path && !Other.Path) || (Path && Other.Path && Path->Equals(*Other.Path))) &&
			((!Outer && !Other.Outer) || (Outer && Other.Outer && *Outer == *Other.Outer));
	}

	FORCEINLINE bool operator!=(const UnrealObjectRef& Other) const
	{
		return !operator==(Other);
	}

	Worker_EntityId Entity;
	uint32 Offset;
	improbable::TSchemaOption<FString> Path;
	improbable::TSchemaOption<UnrealObjectRef> Outer;
};

}

// This template specialization informs THasGetTypeHash
// that there exists a valid GetTypeHash function for improbable::UnrealObjectRef.
// This is required as THasGetTypeHash fails to resolve GetTypeHash functions referencing 
// classes inside namespaces.
template <>
struct THasGetTypeHash<improbable::UnrealObjectRef>
{
	enum
	{
		Value = true
	};
};

inline uint32 GetTypeHash(const improbable::UnrealObjectRef& ObjectRef)
{
	uint32 Result = 1327u;
	Result = (Result * 977u) + GetTypeHash(static_cast<int64>(ObjectRef.Entity));
	Result = (Result * 977u) + GetTypeHash(ObjectRef.Offset);
	Result = (Result * 977u) + GetTypeHash(ObjectRef.Path);
	Result = (Result * 977u) + GetTypeHash(ObjectRef.Outer);
	return Result;
}

