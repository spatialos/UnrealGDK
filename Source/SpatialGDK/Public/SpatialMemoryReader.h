// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryReader.h"

#include "SpatialUnrealObjectRef.h"

class USpatialPackageMapClient;

class SPATIALGDK_API FSpatialMemoryReader : public FMemoryReader
{
public:
	FSpatialMemoryReader(TArray<uint8>& InBytes, USpatialPackageMapClient* InPackageMap)
	: FMemoryReader(InBytes)
	, PackageMap(InPackageMap)
	{}

	//using FArchive::operator<<;

	virtual FArchive& operator<<(UObject*& Value) override;

	const TSet<FHashableUnrealObjectRef>& GetUnresolvedObjectRefs() { return UnresolvedObjectRefs; }

protected:
	USpatialPackageMapClient* PackageMap;

	TSet<FHashableUnrealObjectRef> UnresolvedObjectRefs;
};
