// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Utils/RepLayoutUtils.h"

class FSpatialNetBitWriter;
class FSpatialNetBitReader;

namespace improbable
{

class SpatialFastArrayNetSerializeCB : public INetSerializeCB
{
public:
	SpatialFastArrayNetSerializeCB(USpatialNetDriver* InNetDriver)
		: NetDriver(InNetDriver)
	{ }

	virtual void NetSerializeStruct(UScriptStruct* Struct, FBitArchive& Ar, UPackageMap* PackageMap, void* Data, bool& bHasUnmapped);

private:
	USpatialNetDriver* NetDriver;
};

struct FSpatialNetDeltaSerializeInfo : FNetDeltaSerializeInfo
{
	FSpatialNetDeltaSerializeInfo()
	{
		bIsSpatialType = true;
	}

	static FSpatialNetDeltaSerializeInfo CreateWriter(FSpatialNetBitWriter& Writer, SpatialFastArrayNetSerializeCB& Callback);
	static FSpatialNetDeltaSerializeInfo CreateReader(FSpatialNetBitReader& Reader, SpatialFastArrayNetSerializeCB& Callback);
};

}
