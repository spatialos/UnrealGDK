// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/CoreNet.h"
#include "SpatialPackageMapClient.h"

#include "CoreTypes/UnrealObjectRef.h"

class USpatialPackageMapClient;

class SPATIALGDK_API FSpatialNetBitReader : public FNetBitReader
{
public:
	FSpatialNetBitReader(USpatialPackageMapClient* InPackageMap, uint8* Source, int64 CountBits, TSet<UnrealObjectRef>& InUnresolvedRefs)
		: FNetBitReader(InPackageMap, Source, CountBits)
		, UnresolvedRefs(InUnresolvedRefs) {}

	using FArchive::operator<<; // For visibility of the overloads we don't override

	virtual FArchive& operator<<(UObject*& Value) override;

	virtual FArchive& operator<<(struct FWeakObjectPtr& Value) override;

protected:
	void DeserializeObjectRef(struct UnrealObjectRef& ObjectRef);

	TSet<UnrealObjectRef>& UnresolvedRefs;
};
