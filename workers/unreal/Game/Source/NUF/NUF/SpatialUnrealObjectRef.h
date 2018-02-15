// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/unreal/core_types.h>

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
		return std::hash<improbable::unreal::UnrealObjectRef>{}(ObjectRef.ObjectRef);
	}

private:
	improbable::unreal::UnrealObjectRef ObjectRef;
};
