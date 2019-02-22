// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Utils/RepLayoutUtils.h"

namespace improbable
{

struct FSpatialNetDeltaSerializeInfo : FNetDeltaSerializeInfo
{
	FSpatialNetDeltaSerializeInfo()
	{
		bIsSpatialType = true;
	}
};

class SpatialFastArrayNetSerializeCB : public INetSerializeCB
{
public:
	SpatialFastArrayNetSerializeCB(USpatialNetDriver* InNetDriver)
		: NetDriver(InNetDriver)
	{ }

	virtual void NetSerializeStruct(UScriptStruct* Struct, FBitArchive& Ar, UPackageMap* PackageMap, void* Data, bool& bHasUnmapped)
	{
		if (Struct->StructFlags & STRUCT_NetSerializeNative)
		{
			UScriptStruct::ICppStructOps* CppStructOps = Struct->GetCppStructOps();
			check(CppStructOps); // else should not have STRUCT_NetSerializeNative
			bool bSuccess = true;
			if (!CppStructOps->NetSerialize(Ar, PackageMap, bSuccess, reinterpret_cast<uint8*>(Data)))
			{
				bHasUnmapped = true;
			}
			checkf(bSuccess, TEXT("NetSerialize on %s failed."), *Struct->GetStructCPPName());
		}
		else
		{
			TSharedPtr<FRepLayout> RepLayout = NetDriver->GetStructRepLayout(Struct);

			RepLayout_SerializePropertiesForStruct(*RepLayout, Ar, PackageMap, reinterpret_cast<uint8*>(Data), bHasUnmapped);
		}
	}

private:
	USpatialNetDriver* NetDriver;
};

}
