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
	{ }

	virtual void NetSerializeStruct(UScriptStruct* Struct, FBitArchive& Ar, UPackageMap* PackageMap, void* Data, bool& bHasUnmapped);
#if ENGINE_MINOR_VERSION >= 23
	//TODO (UNR-2371): Look at whether we need to implement these - added check(false)s here to guarantee no entry
	virtual void NetSerializeStruct(FNetDeltaSerializeInfo& Params) { checkf(false, TEXT("The GDK does not support the new version of NetSerializeStruct yet.")); };

	virtual void GatherGuidReferencesForFastArray(struct FFastArrayDeltaSerializeParams& Params) { checkf(false, TEXT("GatherGuidReferencesForFastArray called - the GDK currently does not support delta serialization of structs within fast arrays.")); };
	virtual bool MoveGuidToUnmappedForFastArray(struct FFastArrayDeltaSerializeParams& Params) { checkf(false, TEXT("MoveGuidToUnmappedForFastArray called - the GDK currently does not support delta serialization of structs within fast arrays.")); return false; };
	virtual void UpdateUnmappedGuidsForFastArray(struct FFastArrayDeltaSerializeParams& Params) { checkf(false, TEXT("UpdateUnmappedGuidsForFastArray called - the GDK currently does not support delta serialization of structs within fast arrays.")); };
	virtual bool NetDeltaSerializeForFastArray(struct FFastArrayDeltaSerializeParams& Params) { checkf(false, TEXT("NetDeltaSerializeForFastArray called - the GDK currently does not support delta serialization of structs within fast arrays.")); return false; };
#endif

private:
	USpatialNetDriver* NetDriver;
};

struct FSpatialNetDeltaSerializeInfo : FNetDeltaSerializeInfo
{
	FSpatialNetDeltaSerializeInfo()
	{
		bIsSpatialType = true;
	}

	static bool DeltaSerializeRead(USpatialNetDriver* NetDriver, FSpatialNetBitReader& Reader, UObject* Object, int32 ArrayIndex, UProperty* ParentProperty, UScriptStruct* NetDeltaStruct);
	static bool DeltaSerializeWrite(USpatialNetDriver* NetDriver, FSpatialNetBitWriter& Writer, UObject* Object, int32 ArrayIndex, UProperty* ParentProperty, UScriptStruct* NetDeltaStruct);
};

} // namespace SpatialGDK
