// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/CoreNet.h"

#include "Schema/UnrealObjectRef.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialNetBitReader, All, All);

class USpatialPackageMapClient;

namespace FSpatialNetBitReader
{
	// Object to put on the stack which will collect mapped/unresolved references.
	// This will set a tls pointer to be able to access it from the package map client.
	// Native unreal has arrays on the package map client to do the same job.
	// Unlike native unreal, we have a scoped flow for this operation.
	struct SPATIALGDK_API ReadScope
	{
		ReadScope();
		~ReadScope();

		TSet<FUnrealObjectRef> DynamicRefs;
		TSet<FUnrealObjectRef> UnresolvedRefs;
	};

	static ReadScope* GetReadScope();

	SPATIALGDK_API UObject* ReadObject(bool& bUnresolved, USpatialPackageMapClient* PackageMap, FArchive& Archive);
}
