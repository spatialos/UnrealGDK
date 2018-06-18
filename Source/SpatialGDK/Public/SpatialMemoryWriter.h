// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryWriter.h"

class USpatialPackageMapClient;

class SPATIALGDK_API FSpatialMemoryWriter : public FMemoryWriter
{
public:
	FSpatialMemoryWriter(TArray<uint8>& InBytes, USpatialPackageMapClient* InPackageMap)
	: FMemoryWriter(InBytes)
	, PackageMap(InPackageMap)
	{}

	//using FArchive::operator<<;

	virtual FArchive& operator<<(UObject*& Value) override;

	const TSet<const UObject*>& GetUnresolvedObjects() { return UnresolvedObjects; }

protected:
	USpatialPackageMapClient* PackageMap;

	TSet<const UObject*> UnresolvedObjects;
};
