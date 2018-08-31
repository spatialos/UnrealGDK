#pragma once

#include <improbable/c_worker.h>
#include <improbable/collections.h>

#include <memory>
#include <string>
#include <stdint.h>

struct UnrealObjectRef
{
	UnrealObjectRef() = default;

	UnrealObjectRef(Worker_EntityId Entity, std::uint32_t Offset)
		: Entity(Entity)
		, Offset(Offset)
	{}

	UnrealObjectRef(Worker_EntityId Entity, std::uint32_t Offset, std::string Path, UnrealObjectRef Outer)
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
			((!Path && !Other.Path) || (Path && Other.Path && *Path == *Other.Path)) &&
			((!Outer && !Other.Outer) || (Outer && Other.Outer && *Outer == *Other.Outer));
	}

	FORCEINLINE bool operator!=(const UnrealObjectRef& Other) const
	{
		return !operator==(Other);
	}

	friend uint32 GetTypeHash(const UnrealObjectRef& ObjectRef);

	Worker_EntityId Entity;
	std::uint32_t Offset;
	worker::Option<std::string> Path;
	worker::Option<UnrealObjectRef> Outer;
};
