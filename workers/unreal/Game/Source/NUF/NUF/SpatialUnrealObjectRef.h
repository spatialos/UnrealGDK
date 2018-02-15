// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/unreal/core_types.h>

FORCEINLINE FString ObjectRefToString(const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	return FString::Printf(TEXT("(entity ID: %llu, offset: %u)"), ObjectRef.entity(), ObjectRef.offset());
}

class FHashableUnrealObjectRef
{
public:
	FHashableUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef) : ObjectRef(ObjectRef) {}

	operator improbable::unreal::UnrealObjectRef() const
	{
		return ObjectRef;
	}

	bool operator==(const FHashableUnrealObjectRef& Other) const
	{
		return ObjectRef == Other.ObjectRef;
	}

	friend uint32 GetTypeHash(const FHashableUnrealObjectRef& ObjectRef)
	{
		// TODO(David): Currently this is not exported correctly with __declspec. This should be fixed in 12.1.1.
		//return std::hash<improbable::unreal::UnrealObjectRef>{}(ObjectRef.ObjectRef);
		return (ObjectRef.ObjectRef.entity() << 8) + ObjectRef.ObjectRef.offset();
	}

private:
	improbable::unreal::UnrealObjectRef ObjectRef;
};
