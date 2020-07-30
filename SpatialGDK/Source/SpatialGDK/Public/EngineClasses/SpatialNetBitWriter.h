// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/CoreNet.h"
#include "Schema/UnrealObjectRef.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialNetSerialize, All, All);

class USpatialPackageMapClient;

class SPATIALGDK_API FSpatialNetBitWriter : public FNetBitWriter
{
public:
	FSpatialNetBitWriter(USpatialPackageMapClient* InPackageMap);

	using FArchive::operator<<; // For visibility of the overloads we don't override

	virtual FArchive& operator<<(UObject*& Value) override;

	virtual FArchive& operator<<(struct FWeakObjectPtr& Value) override;

	static void WriteObject(FArchive& Archive, USpatialPackageMapClient* PackageMap, UObject* Object);

protected:
	static void SerializeObjectRef(FArchive& Archive, FUnrealObjectRef& ObjectRef);
};
