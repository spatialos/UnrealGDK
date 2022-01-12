// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Utils/RepLayoutUtils.h"

class FSpatialNetBitReader;
class FSpatialNetBitWriter;
class USpatialNetDriver;

namespace SpatialGDK
{
class SpatialFastArrayNetSerializeCB : public INetSerializeCB
{
public:
	SpatialFastArrayNetSerializeCB(USpatialNetDriver* InNetDriver)
		: NetDriver(InNetDriver)
	{
	}

	virtual void NetSerializeStruct(FNetDeltaSerializeInfo& Params) override;

	virtual void GatherGuidReferencesForFastArray(struct FFastArrayDeltaSerializeParams& Params) override
	{
		checkf(false, TEXT("GatherGuidReferencesForFastArray called - the GDK currently does not support delta serialization of structs "
						   "within fast arrays."));
	};
	virtual bool MoveGuidToUnmappedForFastArray(struct FFastArrayDeltaSerializeParams& Params) override
	{
		checkf(false, TEXT("MoveGuidToUnmappedForFastArray called - the GDK currently does not support delta serialization of structs "
						   "within fast arrays."));
		return false;
	};
	virtual void UpdateUnmappedGuidsForFastArray(struct FFastArrayDeltaSerializeParams& Params) override
	{
		checkf(false, TEXT("UpdateUnmappedGuidsForFastArray called - the GDK currently does not support delta serialization of structs "
						   "within fast arrays."));
	};
	virtual bool NetDeltaSerializeForFastArray(struct FFastArrayDeltaSerializeParams& Params) override
	{
		checkf(false, TEXT("NetDeltaSerializeForFastArray called - the GDK currently does not support delta serialization of structs "
						   "within fast arrays."));
		return false;
	};

private:
	USpatialNetDriver* NetDriver;
};

struct FSpatialNetDeltaSerializeInfo : FNetDeltaSerializeInfo
{
	FSpatialNetDeltaSerializeInfo() { bIsSpatialType = true; }

	static bool DeltaSerializeRead(USpatialNetDriver* NetDriver, FSpatialNetBitReader& Reader, UObject* Object, int32 ArrayIndex,
								   FProperty* ParentProperty, UScriptStruct* NetDeltaStruct);
	static bool DeltaSerializeWrite(USpatialNetDriver* NetDriver, FSpatialNetBitWriter& Writer, UObject* Object, int32 ArrayIndex,
									FProperty* ParentProperty, UScriptStruct* NetDeltaStruct);
};

} // namespace SpatialGDK
