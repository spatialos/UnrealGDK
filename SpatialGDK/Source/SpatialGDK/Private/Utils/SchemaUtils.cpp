// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SchemaUtils.h"

#include "Schema/UnrealObjectRef.h"

namespace SpatialGDK
{

void GetFullPathFromUnrealObjectReference(const FUnrealObjectRef& ObjectRef, FString& OutPath)
{
	if (!ObjectRef.Path.IsSet())
	{
		return;
	}

	if (ObjectRef.Outer.IsSet())
	{
		GetFullPathFromUnrealObjectReference(*ObjectRef.Outer, OutPath);
		OutPath.Append(TEXT("."));
	}

	OutPath.Append(*ObjectRef.Path);
}

} // namespace SpatialGDK
