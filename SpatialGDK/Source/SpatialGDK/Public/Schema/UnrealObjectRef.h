// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Utils/SchemaOption.h"
#include <WorkerSDK/Improbable/c_worker.h>

class USpatialPackageMapClient;

struct SPATIALGDK_API FUnrealObjectRef
{
	FUnrealObjectRef() = default;
	FUnrealObjectRef(const FUnrealObjectRef&) = default;

	FUnrealObjectRef(Worker_EntityId Entity, uint32 Offset)
		: Entity(Entity)
		, Offset(Offset)
	{
	}

	FUnrealObjectRef(Worker_EntityId Entity, uint32 Offset, FString Path, FUnrealObjectRef Outer, bool bNoLoadOnClient = false)
		: Entity(Entity)
		, Offset(Offset)
		, Path(Path)
		, Outer(Outer)
		, bNoLoadOnClient(bNoLoadOnClient)
	{
	}

	FUnrealObjectRef& operator=(const FUnrealObjectRef&) = default;

	FORCEINLINE FString ToString() const { return FString::Printf(TEXT("(entity ID: %lld, offset: %u)"), Entity, Offset); }

	FORCEINLINE FUnrealObjectRef GetLevelReference() const
	{
		if (Path->Equals(TEXT("PersistentLevel")))
		{
			return *this;
		}

		if (Outer.IsSet())
		{
			return Outer->GetLevelReference();
		}
		else
		{
			return FUnrealObjectRef{};
		}
	}

	FORCEINLINE bool operator==(const FUnrealObjectRef& Other) const
	{
		return Entity == Other.Entity && Offset == Other.Offset
			   && ((!Path && !Other.Path) || (Path && Other.Path && Path->Equals(*Other.Path)))
			   && ((!Outer && !Other.Outer) || (Outer && Other.Outer && *Outer == *Other.Outer)) &&
			   // Intentionally don't compare bNoLoadOnClient since it does not affect equality.
			   bUseClassPathToLoadObject == Other.bUseClassPathToLoadObject;
	}

	FORCEINLINE bool operator!=(const FUnrealObjectRef& Other) const { return !operator==(Other); }

	FORCEINLINE bool IsValid() const { return (*this != NULL_OBJECT_REF && *this != UNRESOLVED_OBJECT_REF); }

	static UObject* ToObjectPtr(const FUnrealObjectRef& ObjectRef, USpatialPackageMapClient* PackageMap, bool& bOutUnresolved);
	static FSoftObjectPath ToSoftObjectPath(const FUnrealObjectRef& ObjectRef);
	static FUnrealObjectRef FromObjectPtr(UObject* ObjectValue, USpatialPackageMapClient* PackageMap);
	static FUnrealObjectRef FromSoftObjectPath(const FSoftObjectPath& ObjectPath);
	static FUnrealObjectRef GetRefFromObjectClassPath(UObject* Object, USpatialPackageMapClient* PackageMap);
	static bool ShouldLoadObjectFromClassPath(UObject* Object);
	static bool IsUniqueActorClass(UClass* Class);

	static const FUnrealObjectRef NULL_OBJECT_REF;
	static const FUnrealObjectRef UNRESOLVED_OBJECT_REF;

	Worker_EntityId Entity;
	uint32 Offset;
	SpatialGDK::TSchemaOption<FString> Path;
	SpatialGDK::TSchemaOption<FUnrealObjectRef> Outer;
	bool bNoLoadOnClient = false;
	// If this field is set to true, we are saying that the Actor will exist at most once on the given worker.
	// In addition, if we receive information for an Actor of this class over the network, then this data
	// should be applied to the Actor we've already spawned (where another worker created the entity). This
	// information is important for the object ref, since it means we can identify the correct Actor to apply
	// the replicated data to on each worker via the class path (since only 1 Actor should exist for this class).
	bool bUseClassPathToLoadObject = false;
};

inline uint32 GetTypeHash(const FUnrealObjectRef& ObjectRef)
{
	uint32 Result = 1327u;
	Result = (Result * 977u) + GetTypeHash(static_cast<int64>(ObjectRef.Entity));
	Result = (Result * 977u) + GetTypeHash(ObjectRef.Offset);
	Result = (Result * 977u) + GetTypeHash(ObjectRef.Path);
	Result = (Result * 977u) + GetTypeHash(ObjectRef.Outer);
	// Intentionally don't hash bNoLoadOnClient.
	Result = (Result * 977u) + GetTypeHash(ObjectRef.bUseClassPathToLoadObject ? 1 : 0);
	return Result;
}

using ObjectPtrRefPair = TPair<UObject*, FUnrealObjectRef>;
