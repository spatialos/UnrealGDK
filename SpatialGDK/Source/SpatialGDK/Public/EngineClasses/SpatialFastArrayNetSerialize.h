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

#if ENGINE_MINOR_VERSION <= 22
	virtual void NetSerializeStruct(UScriptStruct* Struct, FBitArchive& Ar, UPackageMap* PackageMap, void* Data, bool& bHasUnmapped);
#else
	virtual void NetSerializeStruct(FNetDeltaSerializeInfo& Params);

	//TODO: Look at whether we need to implement these - added check(false)s here to guarantee no entry
	virtual void GatherGuidReferencesForFastArray(struct FFastArrayDeltaSerializeParams& Params) { checkf(false, TEXT("GatherGuidReferencesForFastArray called - the GDK assumes that this is not called. Tell us what happened.")); };
	virtual bool MoveGuidToUnmappedForFastArray(struct FFastArrayDeltaSerializeParams& Params) { checkf(false, TEXT("MoveGuidToUnmappedForFastArray called - the GDK assumes that this is not called. Tell us what happened.")); return false; };
	virtual void UpdateUnmappedGuidsForFastArray(struct FFastArrayDeltaSerializeParams& Params) { checkf(false, TEXT("UpdateUnmappedGuidsForFastArray called - the GDK assumes that this is not called. Tell us what happened.")); };
	virtual bool NetDeltaSerializeForFastArray(struct FFastArrayDeltaSerializeParams& Params) { checkf(false, TEXT("NetDeltaSerializeForFastArray called - the GDK assumes that this is not called. Tell us what happened.")); return false; };
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
