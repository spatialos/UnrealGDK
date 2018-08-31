// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryReader.h"
#include "UObject/CoreNet.h"
#include "SpatialPackageMapClient.h"

#include "CoreTypes/UnrealObjectRef.h"

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
	void DeserializeObjectRef(UnrealObjectRef& ObjectRef);

	USpatialPackageMapClient* PackageMap;
};

class SPATIALGDK_API FSpatialNetBitReader : public FNetBitReader
{
public:
	FSpatialNetBitReader(USpatialPackageMapClient* InPackageMap, uint8* Source, int64 CountBits)
		: FNetBitReader(InPackageMap, Source, CountBits) {}

	using FArchive::operator<<; // For visibility of the overloads we don't override

	virtual FArchive& operator<<(UObject*& Value) override;

	virtual FArchive& operator<<(struct FWeakObjectPtr& Value) override;

protected:
	void DeserializeObjectRef(struct UnrealObjectRef& ObjectRef);
};
