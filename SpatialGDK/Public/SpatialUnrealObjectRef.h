// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/unreal/core_types.h>

FORCEINLINE FString ObjectRefToString(const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	return FString::Printf(TEXT("(entity ID: %lld, offset: %u)"), ObjectRef.entity(), ObjectRef.offset());
}

class FHashableUnrealObjectRef
{
  public:
	FHashableUnrealObjectRef(const improbable::unreal::UnrealObjectRef& ObjectRef)
	: ObjectRef(ObjectRef)
	{
	}

	operator improbable::unreal::UnrealObjectRef() const
	{
		return ObjectRef;
	}

	bool operator==(const FHashableUnrealObjectRef& Other) const
	{
		return ObjectRef == Other.ObjectRef;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FHashableUnrealObjectRef& HashableObjectRef)
	{
		return std::hash<improbable::unreal::UnrealObjectRef>{}(HashableObjectRef.ObjectRef);
	}

  private:
	improbable::unreal::UnrealObjectRef ObjectRef;
};
