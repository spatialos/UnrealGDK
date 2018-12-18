// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/CoreNet.h"
#include "Schema/UnrealObjectRef.h"

class USpatialPackageMapClient;

class SPATIALGDK_API FSpatialNetBitWriter : public FNetBitWriter
{
public:
	FSpatialNetBitWriter(USpatialPackageMapClient* InPackageMap, TSet<TWeakObjectPtr<const UObject>>& InUnresolvedObjects);

	using FArchive::operator<<; // For visibility of the overloads we don't override

	virtual FArchive& operator<<(UObject*& Value) override;

	virtual FArchive& operator<<(struct FWeakObjectPtr& Value) override;

protected:
	void SerializeObjectRef(FUnrealObjectRef& ObjectRef);

	TSet<TWeakObjectPtr<const UObject>>& UnresolvedObjects;
};
