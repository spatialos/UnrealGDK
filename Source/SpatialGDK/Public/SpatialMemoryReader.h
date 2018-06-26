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

	using FArchive::operator<<; // For visibility of the overloads we don't override

	virtual FArchive& operator<<(UObject*& Value) override;

	virtual FArchive& operator<<(struct FWeakObjectPtr& Value) override;

protected:
	USpatialPackageMapClient* PackageMap;
};
