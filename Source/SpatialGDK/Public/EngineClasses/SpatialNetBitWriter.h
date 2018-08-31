// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/CoreNet.h"
#include "SpatialPackageMapClient.h"

class USpatialPackageMapClient;

class SPATIALGDK_API FSpatialNetBitWriter : public FNetBitWriter
{
public:
	FSpatialNetBitWriter(USpatialPackageMapClient* InPackageMap, TSet<const UObject*>& InUnresolvedObjects)
		: FNetBitWriter(InPackageMap, 0)
		, UnresolvedObjects(InUnresolvedObjects)
	{}

	using FArchive::operator<<; // For visibility of the overloads we don't override

	virtual FArchive& operator<<(UObject*& Value) override;

	virtual FArchive& operator<<(struct FWeakObjectPtr& Value) override;

protected:
	void SerializeObjectRef(struct UnrealObjectRef& ObjectRef);

	TSet<const UObject*>& UnresolvedObjects;
};
