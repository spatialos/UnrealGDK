// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryWriter.h"

class USpatialPackageMapClient;

class SPATIALGDK_API FSpatialMemoryWriter : public FMemoryWriter
{
public:
	FSpatialMemoryWriter(TArray<uint8>& InBytes, USpatialPackageMapClient* InPackageMap, TSet<const UObject*>& InUnresolvedObjects)
	: FMemoryWriter(InBytes)
	, PackageMap(InPackageMap)
	, UnresolvedObjects(InUnresolvedObjects)
	{}

	using FArchive::operator<<; // For visibility of the overloads we don't override

	virtual FArchive& operator<<(UObject*& Value) override;

	virtual FArchive& operator<<(struct FWeakObjectPtr& Value) override;

protected:
	USpatialPackageMapClient* PackageMap;

	TSet<const UObject*>& UnresolvedObjects;
};
