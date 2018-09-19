// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/CoreNet.h"

#include "Schema/UnrealObjectRef.h"

class USpatialPackageMapClient;

class SPATIALGDK_API FSpatialNetBitReader : public FNetBitReader
{
public:
	FSpatialNetBitReader(USpatialPackageMapClient* InPackageMap, uint8* Source, int64 CountBits, TSet<improbable::UnrealObjectRef>& InUnresolvedRefs);

	using FArchive::operator<<; // For visibility of the overloads we don't override

	virtual FArchive& operator<<(UObject*& Value) override;

	virtual FArchive& operator<<(struct FWeakObjectPtr& Value) override;

protected:
	void DeserializeObjectRef(improbable::UnrealObjectRef& ObjectRef);

	TSet<improbable::UnrealObjectRef>& UnresolvedRefs;
};
