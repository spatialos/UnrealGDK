// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialFastArrayNetSerialize.h"

#include "EngineClasses/SpatialNetBitReader.h"
#include "EngineClasses/SpatialNetBitWriter.h"

namespace improbable
{

FSpatialNetDeltaSerializeInfo FSpatialNetDeltaSerializeInfo::CreateWriter(FSpatialNetBitWriter& Writer, SpatialFastArrayNetSerializeCB& Callback)
{
	FSpatialNetDeltaSerializeInfo NetDeltaInfo;

	NetDeltaInfo.Writer = &Writer;
	NetDeltaInfo.Map = Writer.PackageMap;
	NetDeltaInfo.NetSerializeCB = &Callback;

	return NetDeltaInfo;
}

FSpatialNetDeltaSerializeInfo FSpatialNetDeltaSerializeInfo::CreateReader(FSpatialNetBitReader& Reader, SpatialFastArrayNetSerializeCB& Callback)
{
	FSpatialNetDeltaSerializeInfo NetDeltaInfo;

	NetDeltaInfo.Reader = &Reader;
	NetDeltaInfo.Map = Reader.PackageMap;
	NetDeltaInfo.NetSerializeCB = &Callback;

	return NetDeltaInfo;
}

void SpatialFastArrayNetSerializeCB::NetSerializeStruct(UScriptStruct* Struct, FBitArchive& Ar, UPackageMap* PackageMap, void* Data, bool& bHasUnmapped)
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

}
