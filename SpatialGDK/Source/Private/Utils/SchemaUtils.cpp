// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SchemaUtils.h"

#include "improbable/UnrealObjectRef.h"


namespace improbable {

void GetFullPathFromUnrealObjectReference(const FUnrealObjectRef& ObjectRef, FString& OutPath) {
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

}  // namespace improbable
