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

		virtual void NetSerializeStruct(FNetDeltaSerializeInfo& Params);

		//TODO: Look at whether we need to implement these
		virtual void GatherGuidReferencesForFastArray(struct FFastArrayDeltaSerializeParams& Params) {};
		virtual bool MoveGuidToUnmappedForFastArray(struct FFastArrayDeltaSerializeParams& Params) { return false; };
		virtual void UpdateUnmappedGuidsForFastArray(struct FFastArrayDeltaSerializeParams& Params) {};
		virtual bool NetDeltaSerializeForFastArray(struct FFastArrayDeltaSerializeParams& Params) { return false; };

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
