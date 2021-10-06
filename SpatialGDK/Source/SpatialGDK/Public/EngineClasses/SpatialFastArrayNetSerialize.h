// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Utils/GDKPropertyMacros.h"
#include "Utils/RepLayoutUtils.h"

class FSpatialNetBitReader;
class FSpatialNetBitWriter;
class USpatialNetDriver;

namespace SpatialGDK
{
PRAGMA_DISABLE_DEPRECATION_WARNINGS // TODO: UNR-2371 - Remove when we update our usage of FNetDeltaSerializeInfo

	class SpatialFastArrayNetSerializeCB : public INetSerializeCB
{
public:
	SpatialFastArrayNetSerializeCB(USpatialNetDriver* InNetDriver)
		: NetDriver(InNetDriver)
	{
	}
	virtual void NetSerializeStruct(UScriptStruct* Struct, FBitArchive& Ar, UPackageMap* PackageMap, void* Data,
									bool& bHasUnmapped) override;
	// TODO: UNR-2371 - Look at whether we need to implement these and implement 'NetSerializeStruct(FNetDeltaSerializeInfo& Params)'.
	virtual void NetSerializeStruct(FNetDeltaSerializeInfo& Params) override
	{
		checkf(false, TEXT("The GDK does not support the new version of NetSerializeStruct yet."));
	};

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
								   GDK_PROPERTY(Property) * ParentProperty, UScriptStruct* NetDeltaStruct);
	static bool DeltaSerializeWrite(USpatialNetDriver* NetDriver, FSpatialNetBitWriter& Writer, UObject* Object, int32 ArrayIndex,
									GDK_PROPERTY(Property) * ParentProperty, UScriptStruct* NetDeltaStruct);
};

PRAGMA_ENABLE_DEPRECATION_WARNINGS // TODO: UNR-2371 - Remove when we update our usage of FNetDeltaSerializeInfo

} // namespace SpatialGDK
